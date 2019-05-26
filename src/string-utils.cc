#include "string-utils.hh"
#include <sstream>
#include <cstdlib>
#include <arpa/inet.h>

static constexpr const bool isBigEndian = htonl(1) == 1;

static constexpr const CFAllocatorRef &cfAlloc = kCFAllocatorDefault;

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

static void finalizeCFObject(Napi::Env env, CFTypeRef object) noexcept {
	CFRelease(reinterpret_cast<CFTypeRef>(object));
}

Napi::Buffer<uint8_t> CFStringToBuffer(
	CFStringRef string,
	CFStringEncoding encoding,
	Napi::Env env,
	UInt8 lossByte
) {
	auto data = CFStringCreateExternalRepresentation(kCFAllocatorMalloc, string, encoding, lossByte);

	if (data == nullptr)
		throw NotRepresentableInEncoding();

	const auto length = CFDataGetLength(data);

	// We're going to be doing a naughty here. Although CFData is supposed to be immutable, we're going to be using its memory as the backing store of a JavaScript ArrayBuffer, which *is* mutable. Behavior is undefined when we do this. It is allocated using plain malloc (to keep it off any special CF/GC/ObjC/Cocoa/whatever heap), so this hopefully won't break anything. Hopefully. Probably. It'd be nice if CFString could write its external representation to a CFMutableData...

	return Napi::Buffer<uint8_t>::New(env, const_cast<UInt8 *>(CFDataGetBytePtr(data)), length, finalizeCFObject);
}

CFStringHandle BufferToCFString(
	Napi::Value buffer,
	CFStringEncoding encoding
) {
	const auto env = buffer.Env();
	void *data;
	size_t length;

	if (buffer.IsArrayBuffer())
		throwIfFailed(env, napi_get_arraybuffer_info(env, buffer, &data, &length));
	else if (buffer.IsDataView())
		throwIfFailed(env, napi_get_dataview_info(env, buffer, &length, &data, nullptr, nullptr));
	else if (buffer.IsTypedArray() || buffer.IsBuffer()) {
		napi_typedarray_type type;
		throwIfFailed(env, napi_get_typedarray_info(env, buffer, &type, &length, &data, nullptr, nullptr));

		if (type != napi_uint8_array)
			throw NotABuffer();
	}
	else
		throw NotABuffer();

	// There's no getting around it: we have to copy the buffer here. There is a CFStringCreateWithBytesNoCopy function, but this may result in the buffer's contents being overwritten, or the whole thing being garbage-collected before the CFString is freed (which would leave the CFString with a dangling pointer). Nor does N-API offer any way to detach a buffer and take ownership of the underlying memory (assuming the JavaScript program is even okay with that). Nor does CF offer any way (as far as I can tell) to transcode a string without making a supposedly-immutable CFString in the process.

	auto cfString = CFStringCreateWithBytes(
		cfAlloc,
		const_cast<const UInt8 *>(reinterpret_cast<UInt8 *>(data)),
		length,
		encoding,
		true
	);

	if (cfString == nullptr)
		throw NotRepresentableInEncoding();

	return CFStringHandle(cfString);
}
