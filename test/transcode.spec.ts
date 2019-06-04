import * as Chai from "chai";
import { encodeSmallest, NotRepresentableError, SelectAndEncodeOptions, StringEncoding, TextAndEncoding, transcode, transcodeSmallest } from "..";
import ChaiBytes = require("chai-bytes");

Chai.use(ChaiBytes);
const { assert } = Chai;

describe("transcode", () => {
	it("should correctly transcode", () => {
		const result = transcode(
			Buffer.from("2 ÷ 2 = 1¶", "latin1"),
			StringEncoding.byIANACharSetName("iso-8859-1"),
			StringEncoding.byIANACharSetName("macintosh")
		);

		assert.equalBytes(result, Buffer.from("MiDWIDIgPSAxpg==", "base64"));
	});

	it("should handle loss bytes", () => {
		const result = transcode(
			Buffer.from("2 ÷ 2 = 1¶", "latin1"),
			StringEncoding.byIANACharSetName("iso-8859-1"),
			StringEncoding.byIANACharSetName("us-ascii"),
			{ lossByte: 63 }
		);

		assert.equalBytes(result, Buffer.from("2 ? 2 = 1?", "ascii"));
	});

	it("should throw when a loss byte is required but missing", () => {
		assert.throws(() => transcode(
			Buffer.from("2 ÷ 2 = 1¶", "latin1"),
			StringEncoding.byIANACharSetName("iso-8859-1"),
			StringEncoding.byIANACharSetName("us-ascii")
		), NotRepresentableError);
	});

	it("should throw TypeError when appropriate", () => {
		assert.throws(() => transcode(
			42 as any,
			StringEncoding.byIANACharSetName("iso-8859-1"),
			StringEncoding.byIANACharSetName("us-ascii")
		), TypeError);

		assert.throws(() => transcode(
			Buffer.alloc(0),
			"iso-8859-1" as any,
			StringEncoding.byIANACharSetName("us-ascii")
		), TypeError);

		assert.throws(() => transcode(
			Buffer.alloc(0),
			StringEncoding.byIANACharSetName("iso-8859-1"),
			"us-ascii" as any
		), TypeError);
	});
});

describe("encodeSmallest", () => {
	const input = "2 ÷ 2 = 1¶";

	function checkRoundTrip(encoded: TextAndEncoding): void {
		assert.strictEqual(encoded.encoding.decode(encoded.text), input);
	}

	it("should round-trip, and choose a sensible smallest encoding", () => {
		const encoded = encodeSmallest(input);
		// For any given string (that isn't all ASCII), any representation of Unicode will be byte-wise larger than a single-byte character set covering all of the characters. Therefore, the smallest encoding should *not* be a UTF.
		assert.notMatch(encoded.encoding.ianaCharSetName, /^UTF/i, "Chose a nonsensical smallest encoding");
		checkRoundTrip(encoded);
	});

	it("should respect isEncodingOk", () => {
		for (const shouldAccept of [true, false]) {
			const options: SelectAndEncodeOptions = {
				isEncodingOk() {
					return shouldAccept;
				}
			};

			const encoded = encodeSmallest(input, options);

			(shouldAccept ? assert.isNotNull : assert.isNull)(encoded);
			if (shouldAccept)
				checkRoundTrip(encoded!);
		}
	});
});

describe("transcodeSmallest", () => {
	const inputString = "4 ÷ 2 = 2¶";
	const input = Buffer.from(inputString, "latin1");
	const inputEncoding = StringEncoding.byIANACharSetName("iso-8859-1");
	const inputCopy = Buffer.from(input);

	function checkRoundTrip(encoded: TextAndEncoding): void {
		assert.equalBytes(input, inputCopy, "Input buffer has been overwritten!");
		assert.strictEqual(encoded.encoding.decode(encoded.text), inputString);
	}

	it("should round-trip and respect isEncodingOk", () => {
		for (const shouldAccept of [true, false]) {
			const options: SelectAndEncodeOptions = {
				isEncodingOk() {
					return shouldAccept;
				}
			};

			const encoded = transcodeSmallest(input, inputEncoding, options);

			(shouldAccept ? assert.isNotNull : assert.isNull)(encoded);
			if (shouldAccept)
				checkRoundTrip(encoded!);
		}
	});
});
