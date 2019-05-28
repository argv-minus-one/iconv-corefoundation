import { BufferLike, DecodeOptions, EncodeOptions, StringEncoding } from "./native";

export * from "./errors";
export * from "./native";

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
