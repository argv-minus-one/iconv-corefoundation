#include "transcode.hh"
#include "iccf.hh"
#include "CFHandle.hh"
#include "string-utils.hh"
#include "StringEncoding.hh"
#include <optional>
#include <functional>
#include <CoreFoundation/CFString.h>

bool EncodeOptions::isEncodingOk(StringEncoding *encoding) const {
	if (_isEncodingOk.IsEmpty())
		return true;
	else
		return _isEncodingOk({encoding->Value()}).ToBoolean();
}

bool EncodeOptions::isEncodingOk(Napi::Env env, const Iccf *iccf, CFStringEncoding encoding, StringEncoding **encodingObj) const {
	auto _encodingObj = iccf->StringEncoding.New(env, encoding);

	if (encodingObj != nullptr)
		*encodingObj = _encodingObj;

	return isEncodingOk(_encodingObj);
}

static Iccf *getIccf(const Napi::CallbackInfo &info) {
	return reinterpret_cast<Iccf *>(info.Data());
}

static Napi::Value selectAndEncode(
	const Napi::Env env,
	const Iccf *iccf,
	const CFStringRef text,
	const EncodeOptions &options,
	const std::function<StringEncoding *(CFStringRef)> &selectEncoding,
	StringEncoding **selectedEncoding = nullptr,
	const std::function<Napi::Value(CFStringRef, Napi::Env)> &origString = CFStringToNapiString
) {
	const auto encoding = selectEncoding(text);

	if (selectedEncoding != nullptr)
		*selectedEncoding = encoding;

	if (options.isEncodingOk(encoding)) {
		const auto encodedText = encoding->cfEncode(env, text, options.lossByte, origString);

		auto result = Napi::Object::New(env);
		result["encoding"] = encoding->Value();
		result["text"] = encodedText;
		return result;
	}
	else
		return env.Null();
}

static Napi::Value selectAndEncode(
	const Napi::Env env,
	const Iccf *iccf,
	const CFStringRef text,
	const EncodeOptions &options,
	const std::function<StringEncoding *(CFStringRef)> &selectEncoding,
	StringEncoding **selectedEncoding,
	const Napi::Value &origString
) {
	return selectAndEncode(
		env,
		iccf,
		text,
		options,
		selectEncoding,
		selectedEncoding,
		[&] (auto, auto) {
			return origString;
		}
	);
}

#if __has_cpp_attribute(maybe_unused)
[[maybe_unused]]
#endif
static Napi::Value selectAndEncode(
	const Napi::Env env,
	const Iccf *iccf,
	const CFStringRef text,
	const EncodeOptions &options,
	const std::function<StringEncoding *(CFStringRef)> &selectEncoding,
	const Napi::Value &origString
) {
	return selectAndEncode(
		env,
		iccf,
		text,
		options,
		selectEncoding,
		nullptr,
		origString
	);
}

static Napi::Value selectAndEncode(
	const Napi::CallbackInfo &info,
	CFStringEncoding (*selectEncoding)(CFStringRef text)
) {
	const auto env = info.Env();
	const auto iccf = getIccf(info);
	const Napi::Value text = info[0];

	return selectAndEncode(
		env,
		iccf,
		NapiStringToCFString(text.ToString()),
		EncodeOptions(info[1]),
		[&] (auto cfString) {
			return iccf->StringEncoding.New(env, selectEncoding(cfString));
		}
	);
}

static Napi::Value encodeSmallest(const Napi::CallbackInfo &info) {
	return selectAndEncode(info, CFStringGetSmallestEncoding);
}

static Napi::Value transcode(const Napi::CallbackInfo &info) {
	const auto env = info.Env();
	const auto iccf = getIccf(info);
	const EncodeOptions encodeOptions(info[3]);
	const auto fromEncoding = iccf->StringEncoding.UnwrapOrThrow(info[1]), toEncoding = iccf->StringEncoding.UnwrapOrThrow(info[2]);
	const Napi::Value text = info[0];

	return toEncoding->cfEncode(
		env,
		fromEncoding->cfDecode(text),
		encodeOptions.lossByte,
		text
	);
}

static Napi::Value selectAndTranscode(
	const Napi::Env env,
	const Iccf *iccf,
	const Napi::Value &text,
	const DecodeOptions &decodeOptions,
	const EncodeOptions &encodeOptions,
	const StringEncoding *fromEncoding,
	const std::function<StringEncoding *(CFStringRef)> &selectToEncoding,
	StringEncoding **selectedToEncoding = nullptr
) {
	return selectAndEncode(
		env,
		iccf,
		fromEncoding->cfDecode(text),
		encodeOptions,
		selectToEncoding,
		selectedToEncoding,
		text
	);
}

static Napi::Value selectAndTranscode(
	const Napi::CallbackInfo &info,
	CFStringEncoding (*selectToEncoding)(CFStringRef text)
) {
	const auto env = info.Env();
	const auto iccf = getIccf(info);

	return selectAndTranscode(
		env,
		iccf,
		info[0],
		DecodeOptions(info[2]),
		EncodeOptions(info[2]),
		iccf->StringEncoding.UnwrapOrThrow(info[1]),
		[&] (auto cfString) {
			return iccf->StringEncoding.New(env, selectToEncoding(cfString));
		}
	);
}

static Napi::Value transcodeSmallest(const Napi::CallbackInfo &info) {
	return selectAndTranscode(info, CFStringGetSmallestEncoding);
}

EncodeOptions::EncodeOptions(Napi::Value options) {
	if (options.IsObject()) {
		const Napi::Object _options = options.ToObject();

		{
			const Napi::Value _lossByte = _options["lossByte"];
			if (_lossByte.IsNumber())
				lossByte = static_cast<UInt8>(_lossByte.As<Napi::Number>().DoubleValue());
		}

		{
			const Napi::Value _isEncodingOkV = _options["isEncodingOk"];
			if (_isEncodingOkV.IsFunction())
				_isEncodingOk = Napi::Persistent(_isEncodingOkV.As<Napi::Function>());
		}
	}
}

void TranscodeInit(Napi::Env env, Napi::Object exports, Iccf *iccf) {
	Napi::HandleScope scope(env);

	exports.DefineProperties({
		Napi::PropertyDescriptor::Value("encodeSmallest", Napi::Function::New(env, encodeSmallest, "encodeSmallest", iccf), napi_enumerable),
		Napi::PropertyDescriptor::Value("transcode", Napi::Function::New(env, transcode, "transcode", iccf), napi_enumerable),
		Napi::PropertyDescriptor::Value("transcodeSmallest", Napi::Function::New(env, transcodeSmallest, "transcodeSmallest", iccf), napi_enumerable)
	});
}
