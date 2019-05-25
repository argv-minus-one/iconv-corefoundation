#pragma once

#include "napi.hh"
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

	static inline StringEncodingClass *_class(const Napi::CallbackInfo &info) {
		return StringEncodingClass::ForMethodCall(info);
	}

	Napi::Value ianaCharSetName(const Napi::CallbackInfo &info);
	Napi::Value windowsCodepage(const Napi::CallbackInfo &info);
	Napi::Value nsStringEncoding(const Napi::CallbackInfo &info);
	Napi::Value decode(const Napi::CallbackInfo &info);
	Napi::Value encode(const Napi::CallbackInfo &info);
	Napi::Value toPrimitive(const Napi::CallbackInfo &info);
	Napi::Value equals(const Napi::CallbackInfo &info);
	Napi::Value name(const Napi::CallbackInfo &info);

	static Napi::Value byCFStringEncoding(const Napi::CallbackInfo &info);
	static Napi::Value byIANACharSetName(const Napi::CallbackInfo &info);
	static Napi::Value byWindowsCodepage(const Napi::CallbackInfo &info);
	static Napi::Value byNSStringEncoding(const Napi::CallbackInfo &info);
	static Napi::Value system(const Napi::CallbackInfo &info);

	StringEncoding(const Napi::CallbackInfo &info);

	class ConstructorCookie {
		const void *magic;

		public:
		const CFStringEncoding encoding;
		ConstructorCookie(CFStringEncoding encoding);
		static ConstructorCookie *ForCtorCall(const Napi::CallbackInfo &info);
	};

	public:
	const CFStringEncoding _cfStringEncoding;

	inline operator CFStringEncoding() const {
		return _cfStringEncoding;
	}

	std::optional<Napi::String> ianaCharSetName(const Napi::Env &env);
	Napi::String name(const Napi::Env &env);
	static std::optional<StringEncoding *> Unwrap(Napi::Value wrapper);
};

#include "iccf.hh"
