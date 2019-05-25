#pragma once

#include "napi.hh"
#include <CoreFoundation/CFBase.h>

struct EncodeOptions {
	UInt8 lossByte;

	inline EncodeOptions() {}
	EncodeOptions(Napi::Value options);
};

struct DecodeOptions {
	inline DecodeOptions() {}
	inline DecodeOptions(Napi::Value options) : DecodeOptions() {}
};

// struct Iccf;
// void TranscodeInit(Napi::Env env, Napi::Object exports, Iccf *globals);
// #include "iccf.hh"
