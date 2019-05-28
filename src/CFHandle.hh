#pragma once

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

template <typename T>
class CFHandle {
	public:
	const T obj;

	inline CFHandle(T obj, bool retain = false) noexcept : obj(obj) {
		if (retain)
			CFRetain(obj);
	}

	inline CFHandle(CFHandle<T> &other) noexcept : CFHandle(other.obj, true) {}

	inline ~CFHandle() noexcept {
		CFRelease(obj);
	}

	constexpr operator T() const noexcept {
		return obj;
	}
};

typedef CFHandle<CFStringRef> CFStringHandle;
