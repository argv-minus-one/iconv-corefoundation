#pragma once

#include "napi.hh"
#include <CoreFoundation/CFString.h>
#include <optional>

class StringEncoding;
struct Iccf;

struct EncodeOptions {
	UInt8 lossByte = 0;
	Napi::FunctionReference _isEncodingOk;

	inline EncodeOptions() {}
	EncodeOptions(Napi::Value options);

	bool isEncodingOk(StringEncoding *encoding) const;
	bool isEncodingOk(Napi::Env env, const Iccf *iccf, CFStringEncoding encoding, StringEncoding **encodingObj = nullptr) const;
};

struct DecodeOptions {
	inline DecodeOptions() {}
	inline DecodeOptions(Napi::Value options) : DecodeOptions() {}
};

void TranscodeInit(Napi::Env env, Napi::Object exports, Iccf *globals);

#include "iccf.hh"
#include "StringEncoding.hh"
