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
		throw Napi::Error::New(info.Env(), "Invalid callback data passed to StringEncodingClass::ForMethodCall! This is a bug in iconv-corefoundation.");
	return ptr;
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
, _class(StringEncodingClass::ForMethodCall(info))
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

Napi::Buffer<uint8_t> StringEncoding::cfEncode(
	Napi::Env env,
	CFStringRef string,
	UInt8 lossByte,
	std::optional<Napi::Value> origString
) const {
	auto data = CFStringCreateExternalRepresentation(kCFAllocatorMalloc, string, _cfStringEncoding, lossByte);

	if (data == nullptr)
		throw _class->iccf->newNotRepresentableError(env, origString.value_or(CFStringToNapiString(string, env)), Value());

	const auto length = CFDataGetLength(data);

	// We're going to be doing a naughty here. Although CFData is supposed to be immutable, we're going to be using its memory as the backing store of a JavaScript ArrayBuffer, which *is* mutable. Behavior is undefined when we do this. It is allocated using plain malloc (to keep it off any special CF/GC/ObjC/Cocoa/whatever heap), so this hopefully won't break anything. Hopefully. Probably. It'd be nice if CFString could write its external representation to a CFMutableData...

	return Napi::Buffer<uint8_t>::New(
		env,
		const_cast<UInt8 *>(CFDataGetBytePtr(data)),
		length,
		[] (auto env, auto object) noexcept {
			CFRelease(object);
		}
	);
}

CFStringHandle StringEncoding::cfDecode(Napi::Value buffer) const {
	const auto env = buffer.Env();
	void *data;
	size_t length;

	{
		class NotABuffer {};
		try {
			if (buffer.IsArrayBuffer())
				throwIfFailed(env, napi_get_arraybuffer_info(env, buffer, &data, &length));
			else if (buffer.IsDataView())
				throwIfFailed(env, napi_get_dataview_info(env, buffer, &length, &data, nullptr, nullptr));
			else if (buffer.IsTypedArray() || buffer.IsBuffer()) {
				napi_typedarray_type type;
				throwIfFailed(env, napi_get_typedarray_info(env, buffer, &type, &length, &data, nullptr, nullptr));

				if (type != napi_uint8_array)
					throw NotABuffer();
			}
			else
				throw NotABuffer();
		}
		catch (NotABuffer) {
			throw _class->iccf->newFormattedTypeError(env, "a Buffer, ArrayBuffer, DataView, or Uint8Array", buffer);
		}
	}

	// There's no getting around it: we have to copy the buffer here. There is a CFStringCreateWithBytesNoCopy function, but this may result in the buffer's contents being overwritten, or the whole thing being garbage-collected before the CFString is freed (which would leave the CFString with a dangling pointer). Nor does N-API offer any way to detach a buffer and take ownership of the underlying memory (assuming the JavaScript program is even okay with that). Nor does CF offer any way (as far as I can tell) to transcode a string without making a supposedly-immutable CFString in the process.

	auto cfString = CFStringCreateWithBytes(
		kCFAllocatorDefault,
		const_cast<const UInt8 *>(reinterpret_cast<UInt8 *>(data)),
		length,
		_cfStringEncoding,
		true
	);

	if (cfString == nullptr)
		throw _class->iccf->newNotRepresentableError(env, buffer, Value());

	return CFStringHandle(cfString);
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
	return CFStringToNapiString(cfDecode(info[0]), info.Env());
}

Napi::Value StringEncoding::encode(const Napi::CallbackInfo &info) {
	auto string = info[0].ToString();
	EncodeOptions options(info[1]);

	return cfEncode(info.Env(), NapiStringToCFString(string), options.lossByte, string);
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
	const auto _class = StringEncodingClass::ForMethodCall(info);
	CFStringEncoding encoding = info[0].ToNumber();

	if (CFStringIsEncodingAvailable(encoding))
		return _class->New(info.Env(), encoding)->Value();
	else
		throw _class->iccf->newUnrecognizedEncodingError(info.Env(), info[0], Iccf::EncodingSpecifierKind::CFStringEncoding);
}

Napi::Value StringEncoding::byIANACharSetName(const Napi::CallbackInfo &info) {
	const auto _class = StringEncodingClass::ForMethodCall(info);
	auto jsEncodingName = info[0].ToString();
	auto encodingName = NapiStringToCFString(jsEncodingName);
	auto encoding = CFStringConvertIANACharSetNameToEncoding(encodingName);

	if (encoding == kCFStringEncodingInvalidId)
		throw _class->iccf->newUnrecognizedEncodingError(info.Env(), info[0], Iccf::EncodingSpecifierKind::IANACharSetName);
	else
		return _class->New(info.Env(), encoding)->Value();
}

Napi::Value StringEncoding::byWindowsCodepage(const Napi::CallbackInfo &info) {
	const auto _class = StringEncodingClass::ForMethodCall(info);
	UInt32 codepage = info[0].ToNumber();
	auto encoding = CFStringConvertWindowsCodepageToEncoding(codepage);

	if (encoding == kCFStringEncodingInvalidId)
		throw _class->iccf->newUnrecognizedEncodingError(info.Env(), info[0], Iccf::EncodingSpecifierKind::WindowsCodepage);
	else
		return _class->New(info.Env(), encoding)->Value();
}

Napi::Value StringEncoding::byNSStringEncoding(const Napi::CallbackInfo &info) {
	const auto _class = StringEncodingClass::ForMethodCall(info);
	uint32_t nsEncoding = info[0].ToNumber();
	auto encoding = CFStringConvertNSStringEncodingToEncoding(nsEncoding);

	if (encoding == kCFStringEncodingInvalidId)
		throw _class->iccf->newUnrecognizedEncodingError(info.Env(), info[0], Iccf::EncodingSpecifierKind::NSStringEncoding);
	else
		return _class->New(info.Env(), encoding)->Value();
}

Napi::Value StringEncoding::system(const Napi::CallbackInfo &info) {
	const auto _class = StringEncodingClass::ForMethodCall(info);
	return _class->New(info.Env(), CFStringGetSystemEncoding())->Value();
}
