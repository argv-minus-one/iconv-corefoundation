import { inspect } from "util";
import * as errors from "./errors";

module.exports = require("./native.node")({
	newFormattedTypeError(expected: unknown, actual: unknown) {
		return new TypeError(`Expected ${expected}; got ${inspect(actual)}`);
	},
	...errors
});

/** Supported representations of encoded text. */
export type BufferLike = Buffer | Uint8Array | DataView | ArrayBufferLike;

/**
 * A character encoding, known to the Core Foundation framework.
 *
 * @remarks
 * Each instance of this class represents a {@link https://developer.apple.com/documentation/corefoundation/cfstringencoding?language=objc | CFStringEncoding} value.
 */
export declare class StringEncoding {
	/** Instances of this class cannot be constructed directly. */
	private constructor();

	/**
	 * The numeric identifier of this {@link StringEncoding}.
	 *
	 * @remarks
	 * This corresponds to the Core Foundation type {@link https://developer.apple.com/documentation/corefoundation/cfstringencoding?language=objc | CFStringEncoding}.
	 *
	 * Note that this is not a *unique* identifier. Core Foundation interprets many different values as Mac OS Roman. The `name` is more likely (though still not guaranteed) to be truly unique.
	 */
	readonly cfStringEncoding: number;

	/**
	 * The IANA character set name that is the closest mapping to this {@link StringEncoding}.
	 *
	 * @remarks
	 * This is the return value of the Core Foundation function {@link https://developer.apple.com/documentation/corefoundation/1542710-cfstringconvertencodingtoianacha?language=objc | CFStringConvertEncodingToIANACharSetName}.
	 */
	readonly ianaCharSetName: string;

	/**
	 * The Windows codepage that is the closest mapping to this {@link StringEncoding}.
	 *
	 * @remarks
	 * This is the return value of the Core Foundation function {@link https://developer.apple.com/documentation/corefoundation/1543125-cfstringconvertencodingtowindows?language=objc | CFStringConvertEncodingToWindowsCodepage}.
	 */
	readonly windowsCodepage: number | null;

	/**
	 * The Cocoa encoding constant that is the closest mapping to this {@link StringEncoding}.
	 *
	 * @remarks
	 * This is the return value of the Core Foundation function {@link https://developer.apple.com/documentation/corefoundation/1543046-cfstringconvertencodingtonsstrin?language=objc | CFStringConvertEncodingToNSStringEncoding}.
	 */
	readonly nsStringEncoding: number | null;

	/**
	 * The canonical name of this {@link StringEncoding}. This is likely (but not guaranteed) to be a unique identifier for each distinct encoding.
	 *
	 * @remarks
	 * This is the return value of the Core Foundation function {@link https://developer.apple.com/documentation/corefoundation/1543585-cfstringgetnameofencoding?language=objc | CFStringGetNameOfEncoding}.
	 */
	readonly name: string;

	/**
	 * Decodes the text in the given `buffer`.
	 *
	 * @param buffer - The encoded text.
	 * @param options - Options for decoding.
	 * @returns The decoded text, as a string.
	 */
	decode(buffer: BufferLike, options?: DecodeOptions): string;

	/**
	 * Encodes the given string.
	 *
	 * @remarks
	 * Throws {@link NotRepresentableError} if the `string` cannot be fully represented in this encoding, and `options` does not contain a `lossByte`.
	 *
	 * @param string - The text to encode.
	 * @param options - Options for encoding.
	 * @returns The encoded text, in a `Buffer`.
	 */
	encode(string: string, options?: EncodeOptions): Buffer;

	/**
	 * Looks up a {@link StringEncoding} by its {@link https://developer.apple.com/documentation/corefoundation/cfstringencoding?language=objc | numeric identifier}.
	 *
	 * @remarks
	 * Throws {@link UnrecognizedEncodingError} if not recognized and supported.
	 *
	 * @returns The found {@link StringEncoding}.
	 */
	static byCFStringEncoding(id: number): StringEncoding;

	/**
	 * Looks up a {@link StringEncoding} by corresponding IANA character set identifier.
	 *
	 * @remarks
	 * Throws {@link UnrecognizedEncodingError} if not recognized and supported.
	 *
	 * This uses the Core Foundation function {@link https://developer.apple.com/documentation/corefoundation/1542975-cfstringconvertianacharsetnameto?language=objc | CFStringConvertIANACharSetNameToEncoding}.
	 *
	 * @returns The found {@link StringEncoding}.
	 */
	static byIANACharSetName(charset: string): StringEncoding;

	/**
	 * Looks up a {@link StringEncoding} by corresponding Windows codepage.
	 *
	 * @remarks
	 * Throws {@link UnrecognizedEncodingError} if not recognized and supported.
	 *
	 * This uses the Core Foundation function {@link https://developer.apple.com/documentation/corefoundation/1542113-cfstringconvertwindowscodepageto?language=objc | CFStringConvertWindowsCodepageToEncoding}.
	 *
	 * @returns The found {@link StringEncoding}.
	 */
	static byWindowsCodepage(codepage: number): StringEncoding;

	/**
	 * Looks up a {@link StringEncoding} by corresponding Cocoa encoding constant.
	 *
	 * @remarks
	 * Throws {@link UnrecognizedEncodingError} if not recognized and supported.
	 *
	 * This uses the Core Foundation function {@link https://developer.apple.com/documentation/corefoundation/1542787-cfstringconvertnsstringencodingt?language=objc | CFStringConvertNSStringEncodingToEncoding}.
	 *
	 * @returns The found {@link StringEncoding}.
	 */
	static byNSStringEncoding(nsStringEncoding: number): StringEncoding;

	/**
	 * The default {@link StringEncoding} used by the operating system when it creates strings.
	 *
	 * @remarks
	 * This uses the Core Foundation function {@link https://developer.apple.com/documentation/corefoundation/1541720-cfstringgetsystemencoding?language=objc | CFStringGetSystemEncoding}.
	 */
	static readonly system: StringEncoding;
}

/** An object containing some encoded text in a `Buffer`, along with the encoding used. */
export interface TextAndEncoding {
	/** The encoding of the `text`. */
	encoding: StringEncoding;
	/** The encoded text. */
	text: Buffer;
}

/**
 * Encodes the given string, using the smallest representation supported by Core Foundation.
 *
 * @param content - The text to encode.
 * @param options - Options for encoding.
 * @returns The encoded text and chosen encoding.
 */
export declare function encodeSmallest(content: string, options?: SelectAndEncodeOptions & { isEncodingOk?: never }): TextAndEncoding;

/**
 * Encodes the given string, using the smallest representation supported by Core Foundation.
 *
 * @param content - The text to encode.
 * @param options - Options for encoding, possibly including an {@link SelectAndEncodeOptions.isEncodingOk | options.isEncodingOk} method.
 * @returns If {@link SelectAndEncodeOptions.isEncodingOk | options.isEncodingOk} exists and returns `false`, this function returns `null`. Otherwise, this function returns the encoded text and chosen encoding.
 */
export declare function encodeSmallest(content: string, options: SelectAndEncodeOptions): TextAndEncoding | null;

/**
 * Converts encoded text from one encoding to another. This is faster than decoding to a JavaScript string and then encoding the string.
 *
 * @remarks
 * Throws `NotRepresentableError` if the text in `from` cannot be fully represented in `toEncoding`, and `options` does not contain a `lossByte`.
 *
 * @param from - The encoded text to transcode.
 * @param fromEncoding - The encoding of the text in `from`.
 * @param toEncoding - The desired encoding.
 * @param options - Options for both decoding and encoding.
 * @returns The text in `from`, encoded in `toEncoding` instead of `fromEncoding`.
 */
export declare function transcode(from: BufferLike, fromEncoding: StringEncoding, toEncoding: StringEncoding, options?: DecodeOptions & EncodeOptions): Buffer;

/**
 * Converts encoded text from its current encoding to the smallest representation supported by Core Foundation.
 *
 * @param content - The text to encode.
 * @param fromEncoding - The encoding of the text.
 * @param options - Options for both decoding and encoding.
 * @returns The encoded text and chosen encoding.
 */
export declare function transcodeSmallest(content: BufferLike, fromEncoding: StringEncoding, options?: DecodeOptions & SelectAndEncodeOptions & { isEncodingOk?: never }): TextAndEncoding;

/**
 * Converts encoded text from its current encoding to the smallest representation supported by Core Foundation.
 *
 * @param content - The text to encode.
 * @param fromEncoding - The encoding of the text.
 * @param options - Options for both decoding and encoding, possibly including an {@link SelectAndEncodeOptions.isEncodingOk | options.isEncodingOk} method.
 * @returns If {@link SelectAndEncodeOptions.isEncodingOk | options.isEncodingOk} exists and returns `false`, this function returns `null`. Otherwise, this function returns the encoded text and chosen encoding.
 */
export declare function transcodeSmallest(content: BufferLike, fromEncoding: StringEncoding, options: DecodeOptions & SelectAndEncodeOptions): TextAndEncoding | null;

/**
 * Tests whether an encoding exists and is supported.
 *
 * @param encoding - The IANA character set name of the encoding.
 */
export declare function encodingExists(encoding: string): boolean;

/**
 * Options for decoding.
 *
 * @remarks
 * This interface is an empty placeholder, as there are currently no pertinent decoding options supported by Core Foundation.
 */
export interface DecodeOptions {
}

/** Options for encoding. */
export interface EncodeOptions {
	/**
	 * The character that will serve as a substitute for characters that cannot be represented in the requested encoding. It must be an integer between 1 and 255, inclusive.
	 */
	lossByte?: number;
}

/** Additional options for encoding with {@link encodeSmallest} and {@link transcodeSmallest} */
export interface SelectAndEncodeOptions extends EncodeOptions {
	/**
	 * A callback for deciding whether to encode with the given {@link StringEncoding}. This method is called by {@link encodeSmallest} and {@link transcodeSmallest} to let the application decide whether to proceed with Core Foundation's chosen smallest encoding, before actually performing the work of encoding the string.
	 *
	 * @param encoding - The selected {@link StringEncoding}.
	 * @returns `true` if the string should be encoded; `false` to abort encoding. If this method returns `false`, then the calling function ({@link encodeSmallest} or {@link transcodeSmallest}) will return `null` instead of the encoded text.
	 */
	isEncodingOk?(encoding: StringEncoding): boolean;
}
