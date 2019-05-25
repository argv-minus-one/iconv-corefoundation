#include "StringEncoding.hh"
#include "string-utils.hh"
#include "transcode.hh"
#include <sstream>
#include <optional>
#include <stdexcept>

const void *StringEncodingClass::MAGIC = &StringEncodingClass::MAGIC;
const void *StringEncoding::MAGIC = &StringEncoding::MAGIC;
static constexpr const char *CLASS_DATA_KEY = "__native_class_data";

StringEncodingClass::StringEncodingClass(Napi::Env env, Iccf *iccf)
: magic(MAGIC)
, iccf(iccf)
, _constructor([this, &env] () {
	auto ctor = StringEncoding::DefineClass(env, "StringEncoding", {
		StringEncoding::InstanceAccessor("ianaCharSetName", &StringEncoding::ianaCharSetName, nullptr, napi_enumerable, this),
		StringEncoding::InstanceAccessor("windowsCodepage", &StringEncoding::windowsCodepage, nullptr, napi_enumerable, this),
		StringEncoding::InstanceAccessor("nsStringEncoding", &StringEncoding::nsStringEncoding, nullptr, napi_enumerable, this),
		StringEncoding::InstanceAccessor("name", &StringEncoding::name, nullptr, napi_enumerable, this),
		StringEncoding::InstanceMethod("decode", &StringEncoding::decode, napi_default, this),
		StringEncoding::InstanceMethod("encode", &StringEncoding::encode, napi_default, this),
		StringEncoding::InstanceMethod("equals", &StringEncoding::equals, napi_default, this),
		StringEncoding::InstanceMethod(Napi::Symbol::WellKnown(env, "toPrimitive"), &StringEncoding::toPrimitive, napi_default, this),
		StringEncoding::StaticMethod("byCFStringEncoding", &StringEncoding::byCFStringEncoding, napi_default, this),
		StringEncoding::StaticMethod("byIANACharSetName", &StringEncoding::byIANACharSetName, napi_default, this),
		StringEncoding::StaticMethod("byWindowsCodepage", &StringEncoding::byWindowsCodepage, napi_default, this),
		StringEncoding::StaticMethod("byNSStringEncoding", &StringEncoding::byNSStringEncoding, napi_default, this),
		StringEncoding::StaticAccessor("system", &StringEncoding::system, nullptr, napi_enumerable, this)
	}, this);

	{
		auto ext = Napi::External<StringEncodingClass>::New(env, this);
		ctor.DefineProperty(Napi::PropertyDescriptor::Value(CLASS_DATA_KEY, ext));
	}

	auto ref = Napi::Persistent(ctor);
	ref.SuppressDestruct();
	return ref;
}())
{}

StringEncodingClass *StringEncodingClass::ForMethodCall(const Napi::CallbackInfo &info) {
	auto ptr = reinterpret_cast<StringEncodingClass *>(info.Data());
	if (ptr->magic != MAGIC)
		throw Napi::TypeError::New(info.Env(), "StringEncoding method was called on invalid receiver object.");
	return ptr;
}

StringEncodingClass *StringEncodingClass::ForJsCtor(Napi::Function ctor) {
	class Invalid {};

	try {
		Napi::Value prop = ctor[CLASS_DATA_KEY];
		if (!prop.IsExternal())
			throw Invalid();

		auto ptr = prop.As<Napi::External<StringEncodingClass>>().Data();
		if (ptr->magic != MAGIC)
			throw Invalid();

		return ptr;
	}
	catch (Invalid) {
		throw Napi::TypeError::New(ctor.Env(), "Supplied object is not the StringEncoding constructor.");
	}
}

StringEncoding *StringEncodingClass::New(Napi::Env env, CFStringEncoding encoding) const {
	StringEncoding::ConstructorCookie cookie(encoding);
	auto extCookie = Napi::External<StringEncoding::ConstructorCookie>::New(env, &cookie);
	auto wrapper = constructor().New({ extCookie });
	return *StringEncoding::Unwrap(wrapper);
}

StringEncoding::ConstructorCookie::ConstructorCookie(CFStringEncoding encoding)
: magic(MAGIC)
, encoding(encoding)
{}

StringEncoding::ConstructorCookie *StringEncoding::ConstructorCookie::ForCtorCall(const Napi::CallbackInfo &info) {
	class Invalid {};

	try {
		auto cookie = info[0];
		if (!cookie.IsExternal())
			throw Invalid();
		auto ptr = cookie.As<Napi::External<ConstructorCookie>>().Data();

		// This assumes that ptr is a valid pointer, and that reading the first sizeof(void *) bytes from the pointed-to address won't segfault. It'd be nice if there were some portable way of *checking* whether the pointer (and pointer + sizeof(...)) is a valid address, because we have a perfectly good recovery action here.
		if (ptr->magic != MAGIC)
			throw Invalid();
		return ptr;
	}
	catch (Invalid) {
		throw Napi::TypeError::New(info.Env(), info.IsConstructCall() ? "StringEncoding is not a constructor." : "StringEncoding is not a function.");
	}
}

StringEncoding::StringEncoding(const Napi::CallbackInfo &info)
: ObjectWrap(info)
, magic(MAGIC)
, _cfStringEncoding(ConstructorCookie::ForCtorCall(info)->encoding)
{
	info.This().As<Napi::Object>().DefineProperty(
		Napi::PropertyDescriptor::Value("cfStringEncoding", Napi::Number::New(info.Env(), _cfStringEncoding))
	);
}

std::optional<StringEncoding *> StringEncoding::Unwrap(Napi::Value wrapper) {
	if (!wrapper.IsObject())
		return std::nullopt;

	auto se = Napi::ObjectWrap<StringEncoding>::Unwrap(wrapper.As<Napi::Object>());

	if (se != nullptr && se->magic != MAGIC)
		return std::nullopt;

	return se;
}

std::optional<Napi::String> StringEncoding::ianaCharSetName(const Napi::Env &env) {
	auto cfString = CFStringConvertEncodingToIANACharSetName(_cfStringEncoding);

	if (cfString == nullptr)
		return std::nullopt;
	else
		return CFStringToNapiString(cfString, env);
}

Napi::Value StringEncoding::ianaCharSetName(const Napi::CallbackInfo &info) {
	auto v = ianaCharSetName(info.Env());
	if (v)
		return *v;
	else
		return info.Env().Null();
}

Napi::Value StringEncoding::windowsCodepage(const Napi::CallbackInfo &info) {
	auto codepage = CFStringConvertEncodingToWindowsCodepage(_cfStringEncoding);
	if (codepage == UINT32_MAX)
		return info.Env().Null();
	else
		return Napi::Number::New(info.Env(), codepage);
}

Napi::Value StringEncoding::nsStringEncoding(const Napi::CallbackInfo &info) {
	return Napi::Number::New(info.Env(), CFStringConvertEncodingToNSStringEncoding(_cfStringEncoding));
}

Napi::Value StringEncoding::decode(const Napi::CallbackInfo &info) {
	try {
		auto cfString = BufferToCFString(info[0], _cfStringEncoding);
		return CFStringToNapiString(cfString, info.Env());
	}
	catch (NotRepresentableInEncoding) {
		throw _class(info)->iccf->newNotRepresentableError(info.Env(), info[0], info.This().As<Napi::Object>());
	}
	catch (NotABuffer) {
		throw _class(info)->iccf->newIccfTypeError(info.Env(), "First parameter to StringEncoding.prototype.decode", "a Buffer, ArrayBuffer, DataView, or Uint8Array", info[0]);
	}
}

Napi::Value StringEncoding::encode(const Napi::CallbackInfo &info) {
	auto string = info[0].ToString();
	auto cfString = NapiStringToCFString(string);
	EncodeOptions options(info[1]);

	try {
		return CFStringToBuffer(cfString, _cfStringEncoding, info.Env(), options.lossByte);
	}
	catch (NotRepresentableInEncoding) {
		throw _class(info)->iccf->newNotRepresentableError(info.Env(), info[0], info.This().As<Napi::Object>());
	}
}

Napi::Value StringEncoding::equals(const Napi::CallbackInfo &info) {
	auto other = info[0];
	auto otherAsSE = Unwrap(other);
	return Napi::Boolean::From(info.Env(), otherAsSE && (*otherAsSE)->_cfStringEncoding == _cfStringEncoding);
}

Napi::String StringEncoding::name(const Napi::Env &env) {
	return CFStringToNapiString(CFStringGetNameOfEncoding(_cfStringEncoding), env);
}

Napi::Value StringEncoding::name(const Napi::CallbackInfo &info) {
	return name(info.Env());
}

Napi::Value StringEncoding::toPrimitive(const Napi::CallbackInfo &info) {
	const auto env = info.Env();
	if (static_cast<std::string>(info[0].ToString()) == "number")
		return Napi::Number::New(env, _cfStringEncoding);
	else {
		auto csName = ianaCharSetName(env);
		if (csName)
			return *csName;
		else
			return name(env);
	}
}

Napi::Value StringEncoding::byCFStringEncoding(const Napi::CallbackInfo &info) {
	CFStringEncoding encoding = info[0].ToNumber();

	if (CFStringIsEncodingAvailable(encoding))
		return _class(info)->New(info.Env(), encoding)->Value();
	else
		throw _class(info)->iccf->newUnrecognizedEncodingError(info.Env(), info[0], Iccf::EncodingSpecifierKind::CFStringEncoding);
}

Napi::Value StringEncoding::byIANACharSetName(const Napi::CallbackInfo &info) {
	auto jsEncodingName = info[0].ToString();
	auto encodingName = NapiStringToCFString(jsEncodingName);
	auto encoding = CFStringConvertIANACharSetNameToEncoding(encodingName);

	if (encoding == kCFStringEncodingInvalidId)
		throw _class(info)->iccf->newUnrecognizedEncodingError(info.Env(), info[0], Iccf::EncodingSpecifierKind::IANACharSetName);
	else
		return _class(info)->New(info.Env(), encoding)->Value();
}

Napi::Value StringEncoding::byWindowsCodepage(const Napi::CallbackInfo &info) {
	UInt32 codepage = info[0].ToNumber();
	auto encoding = CFStringConvertWindowsCodepageToEncoding(codepage);

	if (encoding == kCFStringEncodingInvalidId)
		throw _class(info)->iccf->newUnrecognizedEncodingError(info.Env(), info[0], Iccf::EncodingSpecifierKind::WindowsCodepage);
	else
		return _class(info)->New(info.Env(), encoding)->Value();
}

Napi::Value StringEncoding::byNSStringEncoding(const Napi::CallbackInfo &info) {
	uint32_t nsEncoding = info[0].ToNumber();
	auto encoding = CFStringConvertNSStringEncodingToEncoding(nsEncoding);

	if (encoding == kCFStringEncodingInvalidId)
		throw _class(info)->iccf->newUnrecognizedEncodingError(info.Env(), info[0], Iccf::EncodingSpecifierKind::NSStringEncoding);
	else
		return _class(info)->New(info.Env(), encoding)->Value();
}

Napi::Value StringEncoding::system(const Napi::CallbackInfo &info) {
	return _class(info)->New(info.Env(), CFStringGetSystemEncoding())->Value();
}
