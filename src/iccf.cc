#include "iccf.hh"
#include "StringEncoding.hh"
#include "transcode.hh"
#include "napi.hh"
#include <sstream>

static Napi::Object init(Napi::Env env, Napi::Object exports) {
	return Napi::Function::New(env, [] (const Napi::CallbackInfo &info) {
		const auto env = info.Env();
		const auto exports = Napi::Object::New(env);
		const auto iccf = new Iccf(info[0].ToObject(), exports);

		napi_add_env_cleanup_hook(env, [] (void *arg) {
			delete reinterpret_cast<Iccf *>(arg);
		}, iccf);

		return exports;
	}, "");
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, init)

static Napi::FunctionReference funcRef(Napi::Object imports, const char *name) {
	Napi::Value value = imports[name];

	if (value.IsUndefined()) {
		std::stringstream ss;
		ss << "Property \"" << name << "\" is missing from imports object.";
		throw Napi::TypeError::New(imports.Env(), ss.str());
	}
	else if (value.IsFunction()) {
		auto ref = Napi::Persistent(value.As<Napi::Function>());
		ref.SuppressDestruct();
		return ref;
	}
	else {
		std::stringstream ss;
		ss << "Property \"" << name << "\" of imports object is not a function. Instead, it is: " << value.ToString().Utf8Value();
		throw Napi::TypeError::New(imports.Env(), ss.str());
	}
}

Iccf::Iccf(Napi::Object imports, Napi::Object exports)
: NotRepresentableError(funcRef(imports, "NotRepresentableError"))
, UnrecognizedEncodingError(funcRef(imports, "UnrecognizedEncodingError"))
, _newFormattedTypeError(funcRef(imports, "newFormattedTypeError"))
, StringEncoding(imports.Env(), this)
{
	exports.DefineProperties({
		Napi::PropertyDescriptor::Value("StringEncoding", StringEncoding.constructor(), napi_enumerable)
	});

	TranscodeInit(imports.Env(), exports, this);
}
