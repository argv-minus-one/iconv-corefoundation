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
