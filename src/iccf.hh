#pragma once

#include "napi.hh"
#include "StringEncoding.hh"

struct Iccf {
	const Napi::FunctionReference NotRepresentableError, UnrecognizedEncodingError, IccfTypeError;
	const StringEncodingClass StringEncoding;

	Iccf(Napi::Object imports, Napi::Object exports);

	inline Napi::Error newNotRepresentableError(const Napi::Env env, Napi::Value string, Napi::Object encoding) const {
		return NotRepresentableError.New({ string, encoding }).As<Napi::Error>();
	}

	enum class EncodingSpecifierKind : uint32_t {
		CFStringEncoding = 1,
		IANACharSetName,
		WindowsCodepage,
		NSStringEncoding
	};

	inline Napi::Error newUnrecognizedEncodingErrorWithCustomSpecifierKind(const Napi::Env env, Napi::Value encodingSpecifier, const char *specifierKind) const {
		return UnrecognizedEncodingError.New({ encodingSpecifier, Napi::String::New(env, specifierKind) }).As<Napi::Error>();
	}

	inline Napi::Error newUnrecognizedEncodingError(const Napi::Env env, Napi::Value encodingSpecifier, EncodingSpecifierKind specifierKind) const {
		return UnrecognizedEncodingError.New({ encodingSpecifier, Napi::Number::New(env, static_cast<uint32_t>(specifierKind)) }).As<Napi::Error>();
	}

	inline Napi::TypeError newIccfTypeError(const Napi::Env env, const char *where, const char *expected, Napi::Value actual) const {
		return IccfTypeError.New({ Napi::String::New(env, where), Napi::String::New(env, expected), actual }).As<Napi::TypeError>();
	}
};
