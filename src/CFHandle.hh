#pragma once

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFString.h>

template <typename T>
class CFHandle {
	public:
	const T obj;

	inline CFHandle(T obj, bool retain = false) : obj(obj) {
		if (retain)
			CFRetain(obj);
	}

	inline CFHandle(CFHandle<T> &other) : CFHandle(other.obj, true) {}

	inline ~CFHandle() {
		CFRelease(obj);
	}

	constexpr operator T() const noexcept {
		return obj;
	}
};

typedef CFHandle<CFStringRef> CFStringHandle;
