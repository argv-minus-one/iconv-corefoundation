#include "transcode.hh"
#include <stdexcept>
#include <optional>

// struct EncodeOptionsWithIsEncodingOk : public EncodeOptions {
// 	std::optional<Napi::FunctionReference> _isEncodingOk = std::nullopt;

// 	bool isEncodingOk(CFStringEncoding encoding) {
// 		if (_isEncodingOk) {

// 			return (*_isEncodingOk).Call({})
// 		}
// 	}

// 	inline EncodeOptionsWithIsEncodingOk() {}

// 	EncodeOptionsWithIsEncodingOk(Napi::Object options) : EncodeOptions(options) {
// 		Napi::Value __isEncodingOk = options["isEncodingOk"];
// 		if (__isEncodingOk.IsFunction)
// 			_isEncodingOk = Napi::Persistent(__isEncodingOk.As<Napi::Function>());
// 	}
// }

// static Iccf *globals(const Napi::CallbackInfo &info) {
// 	return reinterpret_cast<Iccf *>(info.Data());
// }

// static Napi::Value selectAndEncode(
// 	const Napi::String &string,
// 	const CFStringEncoding (*selectEncoding)(CFStringRef string),
// 	const EncodeOptions &options
// ) {

// }

// static Napi::Value encodeFastest(const Napi::CallbackInfo &info) {
// 	const auto str = info[0].ToString();
// 	const EncodeOptions options(info[1]);
// }

// static Napi::Value encodeSmallest(const Napi::CallbackInfo &info) {}

// static Napi::Value transcode(const Napi::CallbackInfo &info) {}

EncodeOptions::EncodeOptions(Napi::Value options) {
	if (options.IsUndefined() || options.IsNull()) {
		lossByte = 0;
	}
	else {
		Napi::Object _options = options.ToObject();
		Napi::Value _lossByte = _options["lossByte"];
		if (_lossByte.IsNumber())
			lossByte = static_cast<UInt8>(_lossByte.As<Napi::Number>().DoubleValue());
	}
}

// void TranscodeInit(Napi::Env env, Napi::Object exports, Iccf *globals) {
// 	Napi::HandleScope scope(env);

// 	exports.Set("encodeFastest", Napi::Function::New(env, encodeFastest, "encodeFastest", globals));
// 	exports.Set("encodeSmallest", Napi::Function::New(env, encodeSmallest, "encodeSmallest", globals));
// 	exports.Set("transcode", Napi::Function::New(env, transcode, "transcode", globals));
// }
