#include "string-utils.hh"
#include <sstream>
#include <cstdlib>
#include <arpa/inet.h>

static constexpr const bool isBigEndian = htonl(1) == 1;

CFStringHandle NapiStringToCFString(const Napi::String string) {
	// Napi::String::Utf16Value would be painfully inefficient for what we're doing: using it would involve *three* copies of the string (JS VM to std::u16string to CFString) per call to this function! Using raw N-API, we can reduce it to one copy (JS VM to buffer, then transfer ownership of buffer to CFString). I'd rather have zero copies, but N-API makes that impossible, unfortunately.
	const napi_env env = string.Env();
	size_t length;

	// Get length of string.
	// Unfortunately, there is no N-API equivalent to CFStringGetFastestEncoding. We'll assume that UTF-16 is it, since several JavaScript string operations (like character indices) act on UTF-16 code units.
	throwIfFailed(env, napi_get_value_string_utf16(
		env,
		string,
		nullptr,
		0,
		&length
	));

	// For some insane reason, napi_get_value_string_utf16 adds a null code unit to the end of the UTF-16 string (which is useful in UTF-8 but completely useless in UTF-16), so we need to allocate an extra byte pair for it.
	const size_t
		lengthWithNull = length + 1,
		byteLength = length * 2,
		byteLengthWithNull = lengthWithNull * 2;

	// Allocate memory for string.
	auto buf = static_cast<char16_t *>(CFAllocatorAllocate(cfAlloc, byteLengthWithNull, 0));

	try {
		// Copy string contents.
		throwIfFailed(env, napi_get_value_string_utf16(
			env,
			string,
			buf,
			lengthWithNull,
			nullptr
		));

		// Make CFString.
		auto cfstr = CFStringCreateWithBytesNoCopy(
			cfAlloc,
			reinterpret_cast<UInt8 *>(buf),
			byteLength,
			kCFStringEncodingUTF16LE,
			true,
			cfAlloc // This tells it to deallocate `buf` when ready. In other words, this transfers ownership of the buffer to the new CFString.
		);

		if (cfstr == nullptr) {
			auto error = Napi::Error::New(string.Env(), "CFStringCreateWithBytesNoCopy() failed to convert the supplied string.");
			error.Set("string", string);
			throw error;
		}

		return CFStringHandle(cfstr);
	}
	catch (...) {
		// If any of that failed, we need to free the allocated buffer memory ourselves.
		CFAllocatorDeallocate(cfAlloc, buf);
		throw;
	}
}

Napi::String CFStringToNapiString(CFStringRef string, Napi::Env env) {
	auto length = CFStringGetLength(string);

	if (!isBigEndian) {
		// Try to avoid copying. Only possible if the system is little-endian, because N-API only accepts little-endian UTF-16.
		auto bytes = CFStringGetCharactersPtr(string);
		if (bytes != nullptr)
			// This will still copy the string from CF memory to the JS heap, but at least it's copied only once.
			return Napi::String::New(env, reinterpret_cast<const char16_t *>(bytes), length);
	}

	// Extract UTF16LE characters from the CFString.
	// This code path, unfortunately, will copy the string twice: once from CFString to the buffer here, then again to copy the string to the JS heap.
	const size_t byteLength = length * 2;
	uint8_t bytes[byteLength];
	auto charsConverted = CFStringGetBytes(string, { .length = length }, kCFStringEncodingUTF16LE, 0, false, bytes, byteLength, nullptr);

	// Make sure it's the correct length.
	if (charsConverted != length) {
		std::stringstream ss;
		ss << "CFStringGetBytes should have yielded "
			<< byteLength
			<< " bytes, but it instead yielded "
			<< (charsConverted * 2)
			<< " bytes.";
		throw Napi::Error::New(env, ss.str());
	}

	// Construct the JS string. This will copy the characters from the buffer (which is allocated on the stack) to the JS heap.
	return Napi::String::New(env, reinterpret_cast<char16_t *>(bytes), length);
}
