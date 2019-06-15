#pragma once

#define NAPI_VERSION 3
#include <node_api.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) napi_status napi_type_tag_object(napi_env env, napi_value js_object, const void* type_tag);
__attribute__((weak)) napi_status napi_check_object_type_tag(napi_env env, napi_value js_object, const void* type_tag, bool* result);

#ifdef __cplusplus
}
#endif
