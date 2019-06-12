#pragma once

#include "napi.hh"
#include "CFHandle.hh"
#include "string-utils.hh"
#include <optional>
#include <CoreFoundation/CFString.h>

struct Iccf;
class StringEncoding;

class StringEncodingClass {
	static const void *MAGIC;
	const void *magic;

	public:
	const Iccf *iccf;

	private:
	const Napi::FunctionReference _constructor;

	public:
	StringEncodingClass(Napi::Env env, Iccf *iccf);
	StringEncoding *New(Napi::Env env, CFStringEncoding encoding) const;
	StringEncoding *byIANACharSetName(const Napi::String name) const;
	std::optional<StringEncoding *> Unwrap(Napi::Value wrapper, bool acceptStrings = true) const;
	StringEncoding *UnwrapOrThrow(Napi::Value wrapper, bool acceptStrings = true) const;

	inline Napi::Function constructor() const {
		return _constructor.Value();
	}

	static StringEncodingClass *ForMethodCall(const Napi::CallbackInfo &info);
};

class StringEncoding : public Napi::ObjectWrap<StringEncoding> {
	friend class StringEncodingClass;
	friend class Napi::ObjectWrap<StringEncoding>;

	static const void *MAGIC;
	const void *magic;

	Napi::Value ianaCharSetName(const Napi::CallbackInfo &info);
	Napi::Value windowsCodepage(const Napi::CallbackInfo &info);
	Napi::Value nsStringEncoding(const Napi::CallbackInfo &info);
	Napi::Value decode(const Napi::CallbackInfo &info);
	Napi::Value encode(const Napi::CallbackInfo &info);
	Napi::Value toPrimitive(const Napi::CallbackInfo &info);
	Napi::Value name(const Napi::CallbackInfo &info);

	static Napi::Value byCFStringEncoding(const Napi::CallbackInfo &info);
	static Napi::Value byIANACharSetName(const Napi::CallbackInfo &info);
	static Napi::Value byWindowsCodepage(const Napi::CallbackInfo &info);
	static Napi::Value byNSStringEncoding(const Napi::CallbackInfo &info);
	static Napi::Value system(const Napi::CallbackInfo &info);

	StringEncoding(const Napi::CallbackInfo &info);
	~StringEncoding();

	class ConstructorCookie {
		const void *magic;

		public:
		const CFStringEncoding encoding;
		ConstructorCookie(CFStringEncoding encoding);
		static ConstructorCookie *ForCtorCall(const Napi::CallbackInfo &info);
	};

	public:
	const StringEncodingClass *_class;
	const CFStringEncoding _cfStringEncoding;

	inline operator CFStringEncoding() const {
		return _cfStringEncoding;
	}

	std::optional<Napi::String> ianaCharSetName(const Napi::Env &env);
	Napi::String name(const Napi::Env &env);

	Napi::Buffer<uint8_t> cfEncode(
		Napi::Env env,
		CFStringRef text,
		UInt8 lossByte = 0,
		std::function<Napi::Value(CFStringRef, Napi::Env)> origString = CFStringToNapiString
	) const;

	inline Napi::Buffer<uint8_t> cfEncode(
		Napi::Env env,
		CFStringRef text,
		UInt8 lossByte,
		Napi::Value origString
	) const {
		return cfEncode(
			env,
			text,
			lossByte,
			[origString] (auto, auto) {
				return origString;
			}
		);
	}

	inline Napi::Buffer<uint8_t> cfEncode(
		Napi::Env env,
		CFStringRef text,
		Napi::Value origString
	) const {
		return cfEncode(
			env,
			text,
			0,
			origString
		);
	}

	CFStringHandle cfDecode(Napi::Value text) const;
};

#include "iccf.hh"
