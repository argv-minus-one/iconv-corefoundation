import * as Chai from "chai";
import { inspect } from "util";
import { DecodeOptions, EncodeOptions, InvalidEncodedTextError, NotRepresentableError, StringEncoding, UnrecognizedEncodingError, encode, decode } from "..";
import ChaiBytes = require("chai-bytes");

Chai.use(ChaiBytes);
const { assert } = Chai;

describe("StringEncoding", () => {
	{
		const testEncodings: Array<{
			testingName: string;
			se: StringEncoding;
			ref: {
				cfStringEncoding: number;
				ianaCharSetName: string;
				windowsCodepage?: number | null | Array<number | null>;
				nsStringEncoding?: number | null | Array<number | null>;
				name: RegExp;
				text?: Array<{
					comment?: string;
					string: string;
					bytes: Buffer;
					encodeOptions?: EncodeOptions | null; // If null, do not attempt encoding.
					decodeOptions?: DecodeOptions | null; // If null, do not attempt decoding.
				}>;
				unrepresentable?: string[];
			};
		}> = [
			{
				testingName: "ASCII",
				se: StringEncoding.byIANACharSetName("ascii"),
				ref: {
					cfStringEncoding: 0x0600,
					ianaCharSetName: "US-ASCII",
					windowsCodepage: 20127,
					nsStringEncoding: 1,
					name: /ASCII/i,
					text: [{
						comment: "with loss byte '?'",
						string: "Hello, world¡",
						bytes: Buffer.from("Hello, world?", "ascii"),
						encodeOptions: {
							lossByte: 63
						},
						decodeOptions: null
					}],
					unrepresentable: ["Hello, world¡"]
				}
			},
			{
				testingName: "MacRoman",
				se: StringEncoding.byNSStringEncoding(30),
				ref: {
					cfStringEncoding: 0,
					ianaCharSetName: "macintosh",
					windowsCodepage: 10000,
					nsStringEncoding: 30,
					name: /Mac.*Roman|Roman.*Mac/i,
					text: [{
						string: "Hello, world¡",
						bytes: Buffer.from([72, 101, 108, 108, 111, 44, 32, 119, 111, 114, 108, 100, 193])
					}]
				}
			},
			{
				testingName: "UTF-16LE",
				se: StringEncoding.byCFStringEncoding(0x14000100),
				ref: {
					cfStringEncoding: 0x14000100,
					ianaCharSetName: "UTF-16LE",
					windowsCodepage: [1200, null],
					nsStringEncoding: 0x94000100,
					name: /UTF-?16/i,
					text: [{
						string: "👍",
						bytes: Buffer.from([61, 216, 77, 220])
					}]
				}
			},
			{
				testingName: "UTF-8",
				se: StringEncoding.byWindowsCodepage(65001),
				ref: {
					cfStringEncoding: 0x08000100,
					ianaCharSetName: "UTF-8",
					windowsCodepage: 65001,
					nsStringEncoding: 4,
					name: /UTF-?8/i
				}
			},
			{
				testingName: "MacJapanese",
				se: StringEncoding.byCFStringEncoding(1),
				ref: {
					cfStringEncoding: 1,
					ianaCharSetName: "x-mac-japanese",
					windowsCodepage: 10001,
					name: /Mac.*Japanese|Japanese.*Mac/i,
					text: [{
						string: "同意します~",
						bytes: Buffer.from("k6+I04K1gtyCt34=", "base64")
					}]
				}
			},
			{
				testingName: "Shift JIS",
				se: StringEncoding.byCFStringEncoding(0x0A01),
				ref: {
					cfStringEncoding: 0x0A01,
					ianaCharSetName: "Shift_JIS",
					name: /S(hift)?.*JIS/i,
					text: [{
						string: "同意します‾",
						bytes: Buffer.from("k6+I04K1gtyCt34=", "base64")
					}]
				}
			},
			{
				testingName: "MacInuit",
				se: StringEncoding.byCFStringEncoding(0xEC),
				ref: {
					cfStringEncoding: 0xEC,
					ianaCharSetName: "x-mac-inuit",
					windowsCodepage: null,
					name: /Mac.*Inuit|Inuit.*Mac/i
				}
			}
		];

		for (const {testingName, se, ref} of testEncodings)
		describe(`(${testingName})`, () => {
			it(`should have cfStringEncoding = ${ref.cfStringEncoding}`, () => {
				assert.strictEqual(se.cfStringEncoding, ref.cfStringEncoding);
			});

			it(`should have ianaCharSetName = ${ref.ianaCharSetName}`, () => {
				assert.strictEqual(se.ianaCharSetName.toLowerCase(), ref.ianaCharSetName.toLowerCase());
			});

			if (ref.windowsCodepage !== undefined)
			it(`should have Windows codepage ${ref.windowsCodepage}`, () => {
				if (Array.isArray(ref.windowsCodepage))
					assert.include(ref.windowsCodepage, se.windowsCodepage);
				else
					assert.strictEqual(se.windowsCodepage, ref.windowsCodepage);
			});

			if (ref.nsStringEncoding !== undefined)
			it(`should have NSStringEncoding = ${ref.nsStringEncoding}`, () => {
				if (Array.isArray(ref.nsStringEncoding))
					assert.include(ref.nsStringEncoding, se.nsStringEncoding);
				else
					assert.strictEqual(se.nsStringEncoding, ref.nsStringEncoding);
			});

			it(`should have a name matching ${ref.name}`, () => {
				assert.notStrictEqual(se.name.search(ref.name), -1, `"${se.name}" does not match ${ref.name}`);
			});

			if (ref.text)
			for (const {comment, string, bytes, encodeOptions, decodeOptions} of ref.text) {
				if (encodeOptions !== null)
				it(`should encode ${inspect(string)} to ${inspect(bytes)}${comment ? ` (${comment})` : ""}`, () => {
					let result = Buffer.from(se.encode(string, encodeOptions));
					assert.equalBytes(result, bytes);

					result = Buffer.from(encode(string, se, encodeOptions));
					assert.equalBytes(result, bytes);
				});

				if (decodeOptions !== null)
				it(`should decode ${inspect(bytes)} to ${inspect(string)}${comment ? ` (${comment})` : ""}`, () => {
					let result = se.decode(bytes, decodeOptions);
					assert.strictEqual(result, string);

					result = decode(bytes, se, decodeOptions);
					assert.strictEqual(result, string);
				});
			}

			if (ref.unrepresentable)
			for (const unrepresentable of ref.unrepresentable) {
				it(`should fail to encode ${inspect(unrepresentable)} because it is not representable`, () => {
					assert.throws(() => se.encode(unrepresentable), NotRepresentableError);
				});
			}
		});
	}

	describe(".system", () => {
		const se = StringEncoding.system;

		it("should round-trip a basic string", () => {
			assert.strictEqual(se.decode(se.encode("Hello")), "Hello");
		});
	});

	it("shouldn't choke on null bytes", () => {
		const ascii = StringEncoding.byIANACharSetName("us-ascii");
		const encoded = ascii.encode("\0Hello\0world");
		assert.equalBytes(encoded, [0, 72, 101, 108, 108, 111, 0, 119, 111, 114, 108, 100]);
		const decoded = ascii.decode(encoded);
		assert.strictEqual(decoded, "\0Hello\0world");
	});

	it("should pass through control characters", () => {
		const string = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20";
		const a = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32];
		const ascii = StringEncoding.byIANACharSetName("us-ascii");

		const encoded = ascii.encode(string);
		assert.equalBytes(encoded, a);
		const decoded = ascii.decode(encoded);
		assert.strictEqual(decoded, string);
	});

	it("should reject invalid encoding specifiers", () => {
		assert.throws(() => StringEncoding.byCFStringEncoding(0xffffffff /* kCFStringEncodingInvalidId */), UnrecognizedEncodingError);
		assert.throws(() => StringEncoding.byCFStringEncoding("hi" as any));
		assert.throws(() => StringEncoding.byIANACharSetName("FOOBIE BLETCH"), UnrecognizedEncodingError);
		assert.throws(() => StringEncoding.byIANACharSetName(null as any));
		assert.throws(() => StringEncoding.byNSStringEncoding(0xffffffff), UnrecognizedEncodingError);
		assert.throws(() => StringEncoding.byNSStringEncoding(0x80000000), UnrecognizedEncodingError);
		assert.throws(() => StringEncoding.byNSStringEncoding("xyzzy" as any));
		assert.throws(() => StringEncoding.byWindowsCodepage(0), UnrecognizedEncodingError);
		assert.throws(() => StringEncoding.byWindowsCodepage("lolwut" as any));
	});

	it("should throw on invalid decode input", () => {
		const text = Buffer.from([0x80, 0xa0, 0xc0, 0xf0]);
		assert.throws(() => StringEncoding.byIANACharSetName("UTF-8").decode(text), InvalidEncodedTextError);
	});
});
