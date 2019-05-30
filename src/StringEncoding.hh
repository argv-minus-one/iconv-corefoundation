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
	static std::optional<StringEncoding *> Unwrap(Napi::Value wrapper);
	static StringEncoding *UnwrapOrThrow(Iccf *iccf, Napi::Value wrapper);

	Napi::Buffer<uint8_t> cfEncode(
		Napi::Env env,
		CFStringRef string,
		UInt8 lossByte = 0,
		std::function<Napi::Value(CFStringRef, Napi::Env)> origString = CFStringToNapiString
	) const;

	inline Napi::Buffer<uint8_t> cfEncode(
		Napi::Env env,
		CFStringRef string,
		UInt8 lossByte,
		Napi::Value origString
	) const {
		return cfEncode(
			env,
			string,
			lossByte,
			[origString] (auto, auto) {
				return origString;
			}
		);
	}

	inline Napi::Buffer<uint8_t> cfEncode(
		Napi::Env env,
		CFStringRef string,
		Napi::Value origString
	) const {
		return cfEncode(
			env,
			string,
			0,
			origString
		);
	}

	CFStringHandle cfDecode(Napi::Value buffer) const;
};

#include "iccf.hh"
