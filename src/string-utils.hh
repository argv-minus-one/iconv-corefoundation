#pragma once

#include <CoreFoundation/CFString.h>
#include "CFHandle.hh"
#include "napi.hh"

struct Iccf;

/**
 * Makes a `CFStringRef` from the characters in the given `Napi::String`, making 1 to 2 copies.
 */
CFStringHandle NapiStringToCFString(const Napi::String string);

/**
 * Makes a `Napi::String` from the characters in the given `CFString`, making 1 to 2 copies.
 *
 * The passed-in string is always copied, so it is safe to `CFRelease` it after this function completes. (That will happen automatically if it's wrapped in a CFHandle.)
 */
Napi::String CFStringToNapiString(CFStringRef string, Napi::Env env);

#include "iccf.hh"
