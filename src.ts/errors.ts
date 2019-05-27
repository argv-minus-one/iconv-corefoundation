import { inspect } from "util";
import cliTruncate = require("cli-truncate");

// These are imported as types only, in order to avoid run-time import cycles.
type StringEncoding = import("./index").StringEncoding;

export class NotRepresentableError extends Error {
	constructor(string: unknown, encoding: StringEncoding) {
		super(`Not fully representable in ${encoding}: ${inspect(typeof string === "string" ? cliTruncate(string, 15) : string)}`);
	}
}

export class UnrecognizedEncodingError extends Error {
	constructor(encodingSpecifier: unknown, specifierKind: UnrecognizedEncodingError.SpecifierKind | string) {
		if (typeof specifierKind !== "string")
			specifierKind = UnrecognizedEncodingError.SpecifierKind.toString(specifierKind);

		super(`Unrecognized ${specifierKind}: ${inspect(encodingSpecifier)}`);
	}
}

export namespace UnrecognizedEncodingError {
	export enum SpecifierKind {
		CFStringEncoding = 1,
		IANACharSetName,
		WindowsCodepage,
		NSStringEncoding
	}

	export namespace SpecifierKind {
		const strings: string[] = [
			"(unknown encoding specifier)",
			"CFStringEncoding",
			"IANA charset name",
			"Windows codepage",
			"NSStringEncoding"
		];

		export function toString(specifierKind: SpecifierKind): string {
			return strings[specifierKind >= strings.length ? 0 : specifierKind];
		}
	}
}
