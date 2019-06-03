import { BufferLike, DecodeOptions, EncodeOptions, StringEncoding } from "./native";
import { UnrecognizedEncodingError } from "./errors";

export * from "./errors";
export * from "./native";

/**
 * Tests whether an encoding exists and is supported.
 *
 * @param encoding - The IANA character set name of the encoding.
 */
export function encodingExists(encoding: string): boolean {
	try {
		StringEncoding.byIANACharSetName(encoding);
	}
	catch (e) {
		if (e instanceof UnrecognizedEncodingError)
			return false;
		else
			throw e;
	}
	return true;
}

/**
 * Convenience alias for {@link StringEncoding.decode}.
 *
 * @param buffer - The encoded text.
 * @param encoding - The encoding of the text in `buffer`. May be an IANA character set name or a {@link StringEncoding}.
 * @param options - Options for decoding.
 * @returns The decoded text, as a string.
 */
export function decode(buffer: BufferLike, encoding: string | StringEncoding, options?: DecodeOptions): string {
	if (typeof encoding === "string")
		encoding = StringEncoding.byIANACharSetName(encoding);
	return encoding.decode(buffer, options);
}

/**
 * Convenience alias for {@link StringEncoding.encode}.
 *
 * @remarks
 * Throws {@link NotRepresentableError} if the `string` cannot be fully represented in this encoding, and `options` does not contain a `lossByte`.
 *
 * @param string - The text to encode.
 * @param encoding - The encoding to use. May be an IANA character set name or a {@link StringEncoding}.
 * @param options - Options for encoding.
 * @returns The encoded text, in a `Buffer`.
 */
export function encode(content: string, encoding: string | StringEncoding, options?: EncodeOptions): Buffer {
	if (typeof encoding === "string")
		encoding = StringEncoding.byIANACharSetName(encoding);
	return encoding.encode(content, options);
}
