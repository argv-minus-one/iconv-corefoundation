import { inspect } from "util";
import cliTruncate = require("cli-truncate");

// These are imported as types only, in order to avoid run-time import cycles.
type StringEncoding = import("./index").StringEncoding;

/** Signals that the given text cannot be fully encoded in the chosen {@link StringEncoding}. */
export class NotRepresentableError extends Error {
	private constructor(text: unknown, encoding: StringEncoding) {
		super(`Not fully representable in ${encoding}:\n${inspect(typeof text === "string" ? cliTruncate(text, 65) : text)}`);
	}
}
NotRepresentableError.prototype.name = NotRepresentableError.name;

/**
 * Signals that the given encoded text is not valid in the chosen {@link StringEncoding}.
 *
 * @remarks
 * Not all {@link StringEncoding}s can throw this error. Most single-byte encodings and some multi-byte encodings have a valid mapping for every possible sequence of bytes. However, some encodings (such as ASCII and UTF-8) don't consider all byte sequences valid; such encodings will throw this error if the input contains any invalid byte sequences.
 */
export class InvalidEncodedTextError extends Error {
	private constructor(text: unknown, encoding: StringEncoding) {
		super(`Input is not valid ${encoding}:\n${inspect(text)}`);
	}
}
InvalidEncodedTextError.prototype.name = InvalidEncodedTextError.name;

const specifierKinds = [
	"CFStringEncoding",
	"IANA charset name",
	"Windows codepage",
	"NSStringEncoding"
];

/** Signals that the given {@link StringEncoding} specifier (IANA character set name, `CFStringEncoding` constant, or the like) is not recognized or not supported. */
export class UnrecognizedEncodingError extends Error {
	private constructor(encodingSpecifier: unknown, specifierKind: number | string) {
		if (typeof specifierKind !== "string")
			specifierKind = specifierKinds[specifierKind];

		super(`Unrecognized ${specifierKind}: ${inspect(encodingSpecifier)}`);
	}
}
UnrecognizedEncodingError.prototype.name = UnrecognizedEncodingError.name;
