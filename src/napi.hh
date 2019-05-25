#pragma once

#define NAPI_VERSION 3
#define NODE_ADDON_API_DISABLE_DEPRECATED
#include <napi.h>

inline void throwIfFailed(Napi::Env env, napi_status status) {
	if (status != napi_ok)
		throw Napi::Error::New(env);
}
