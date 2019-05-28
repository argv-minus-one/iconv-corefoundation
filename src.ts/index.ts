import { inspect } from "util";
import * as iccfErrors from "./errors";

export const stuff: unknown = require("bindings")("iconv_corefoundation_native.node")({
	IccfTypeError: class extends TypeError {
		constructor(where: any, expected: any, actual: any) {
			super(`${where} must be ${expected}. Instead it is: ${inspect(actual)}`);
		}
	},
	...iccfErrors
});

Object.assign(exports,
	stuff,
	iccfErrors
);

export type BufferLike = Buffer | Uint8Array | DataView | ArrayBufferLike;

/**
 * A character encoding, known to the Core Foundation framework.
 *
 * @see [`CFStringEncoding`](https://developer.apple.com/documentation/corefoundation/cfstringencoding?language=objc)
 */
export declare class StringEncoding {
	/**
	 * Instances of this class cannot be constructed directly.
	 */
	private constructor();

	/**
	 * The numeric identifier of this `StringEncoding`.
	 *
	 * @see [`CFStringEncoding`](https://developer.apple.com/documentation/corefoundation/cfstringencoding?language=objc)
	 */
	readonly cfStringEncoding: number;

	/**
	 * The IANA character set name that is the closest mapping to this `StringEncoding`.
	 *
	 * @see [`CFStringConvertEncodingToIANACharSetName`](https://developer.apple.com/documentation/corefoundation/1542710-cfstringconvertencodingtoianacha?language=objc)
	 */
	readonly ianaCharSetName: string;

	/**
	 * The Windows codepage that is the closest mapping to this `StringEncoding`.
	 *
	 * @see [`CFStringConvertEncodingToWindowsCodepage`](https://developer.apple.com/documentation/corefoundation/1543125-cfstringconvertencodingtowindows?language=objc)
	 */
	readonly windowsCodepage: number | null;

	/**
	 * The Cocoa encoding constant that is the closest mapping to this `StringEncoding`.
	 *
	 * @see [`CFStringConvertEncodingToNSStringEncoding`](https://developer.apple.com/documentation/corefoundation/1543046-cfstringconvertencodingtonsstrin?language=objc)
	 */
	readonly nsStringEncoding: number | null;

	/**
	 * The name of this `StringEncoding`.
	 *
	 * @see [`CFStringGetNameOfEncoding`](https://developer.apple.com/documentation/corefoundation/1543585-cfstringgetnameofencoding?language=objc)
	 */
	readonly name: string;

	decode(buffer: BufferLike, options?: DecodeOptions): string;

	encode(string: string, options?: EncodeOptions): Buffer;

	equals(other: StringEncoding): boolean;

	/**
	 * Looks up a `StringEncoding` by its numeric identifier.
	 *
	 * @remarks
	 * Throws `UnrecognizedEncodingError` if not recognized.
	 *
	 * @returns The found `StringEncoding`.
	 *
	 * @see [`CFStringEncoding`](https://developer.apple.com/documentation/corefoundation/cfstringencoding?language=objc)
	 */
	static byCFStringEncoding(id: number): StringEncoding;

	/**
	 * Looks up a `StringEncoding` by corresponding IANA character set identifier.
	 *
	 * @remarks
	 * Throws `UnrecognizedEncodingError` if not recognized.
	 *
	 * @returns The found `StringEncoding`.
	 *
	 * @see [`CFStringConvertIANACharSetNameToEncoding`](https://developer.apple.com/documentation/corefoundation/1542975-cfstringconvertianacharsetnameto?language=objc)
	 */
	static byIANACharSetName(charset: string): StringEncoding;

	/**
	 * Looks up a `StringEncoding` by corresponding Windows codepage.
	 *
	 * @remarks
	 * Throws `UnrecognizedEncodingError` if not recognized.
	 *
	 * @returns The found `StringEncoding`.
	 *
	 * @see [`CFStringConvertWindowsCodepageToEncoding`](https://developer.apple.com/documentation/corefoundation/1542113-cfstringconvertwindowscodepageto?language=objc)
	 */
	static byWindowsCodepage(codepage: number): StringEncoding;

	/**
	 * Looks up a `StringEncoding` by corresponding Cocoa encoding constant.
	 *
	 * @remarks
	 * Throws `UnrecognizedEncodingError` if not recognized.
	 *
	 * @returns The found `StringEncoding`.
	 *
	 * @see [`CFStringConvertNSStringEncodingToEncoding`](https://developer.apple.com/documentation/corefoundation/1542787-cfstringconvertnsstringencodingt?language=objc)
	 */
	static byNSStringEncoding(nsStringEncoding: number): StringEncoding;

	/**
	 * The default `StringEncoding` used by the operating system when it creates strings.
	 *
	 * @see [`CFStringGetSystemEncoding`](https://developer.apple.com/documentation/corefoundation/1541720-cfstringgetsystemencoding?language=objc)
	 */
	static readonly system: StringEncoding;
}

export interface TextAndEncoding {
	encoding: StringEncoding;
	text: Buffer;
}

export declare function encodeSmallest(content: string, options: EncodeOptionsWithIsEncodingOk): TextAndEncoding | null;
export declare function encodeSmallest(content: string, options?: EncodeOptions): TextAndEncoding;

export declare function encodeFastest(content: string, options: EncodeOptionsWithIsEncodingOk): TextAndEncoding | null;
export declare function encodeFastest(content: string, options?: EncodeOptions): TextAndEncoding;

export declare function transcode(from: BufferLike, fromEncoding: StringEncoding, toEncoding: StringEncoding, options?: DecodeOptions & EncodeOptions): Buffer;

export declare function transcodeSmallest(content: BufferLike, fromEncoding: StringEncoding, options: DecodeOptions & EncodeOptionsWithIsEncodingOk): TextAndEncoding | null;
export declare function transcodeSmallest(content: BufferLike, fromEncoding: StringEncoding, options?: DecodeOptions & EncodeOptions): TextAndEncoding;

export declare function transcodeFastest(content: BufferLike, fromEncoding: StringEncoding, options: DecodeOptions & EncodeOptionsWithIsEncodingOk): TextAndEncoding | null;
export declare function transcodeFastest(content: BufferLike, fromEncoding: StringEncoding, options?: DecodeOptions & EncodeOptions): TextAndEncoding;

export interface DecodeOptions {
}

export interface EncodeOptions {
	lossByte?: number;
}

export interface EncodeOptionsWithIsEncodingOk extends EncodeOptions {
	isEncodingOk(encoding: StringEncoding): boolean;
}

export function encodingExists(encoding: string): boolean {
	return StringEncoding.byIANACharSetName(encoding) !== null;
}

export function decode(buffer: BufferLike, encoding: string | StringEncoding, options?: DecodeOptions): string {
	if (typeof encoding === "string")
		encoding = StringEncoding.byIANACharSetName(encoding);
	return encoding.decode(buffer, options);
}

export function encode(content: string, encoding: string | StringEncoding, options?: EncodeOptions): Buffer {
	if (typeof encoding === "string")
		encoding = StringEncoding.byIANACharSetName(encoding);
	return encoding.encode(content, options);
}

export * from "./errors";
