import * as Chai from "chai";
import { encodeFastest, EncodeOptionsWithIsEncodingOk, encodeSmallest, NotRepresentableError, StringEncoding, TextAndEncoding, transcode, transcodeSmallest, transcodeFastest } from "..";
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

describe("encodeSmallest & encodeFastest", () => {
	const input = "2 ÷ 2 = 1¶";

	function checkRoundTrip(encoded: TextAndEncoding): void {
		assert.strictEqual(encoded.encoding.decode(encoded.text), input);
	}

	it("should round-trip, and choose a sensible smallest encoding", () => {
		let encoded = encodeSmallest(input);
		// For any given string (that isn't all ASCII), any representation of Unicode will be byte-wise larger than a single-byte character set covering all of the characters. Therefore, the smallest encoding should *not* be a UTF.
		assert.notMatch(encoded.encoding.ianaCharSetName, /^UTF/i, "Chose a nonsensical smallest encoding");
		checkRoundTrip(encoded);

		encoded = encodeFastest(input);
		// The fastest encoding is probably (but is not guaranteed to be) UTF-16, so there's no way to check the sanity of the chosen encoding.
		checkRoundTrip(encoded);
	});

	it("should respect isEncodingOk", () => {
		for (const smallest of [true, false])
		for (const shouldAccept of [true, false]) {
			const options: EncodeOptionsWithIsEncodingOk = {
				isEncodingOk() {
					return shouldAccept;
				}
			};

			const encoded = (smallest ? encodeSmallest : encodeFastest)(input, options);

			(shouldAccept ? assert.isNotNull : assert.isNull)(encoded);
			if (shouldAccept)
				checkRoundTrip(encoded!);
		}
	});
});

describe("transcodeSmallest & transcodeFastest", () => {
	const inputString = "4 ÷ 2 = 2¶";
	const input = Buffer.from(inputString, "latin1");
	const inputEncoding = StringEncoding.byIANACharSetName("iso-8859-1");
	const inputCopy = Buffer.from(input);

	function checkRoundTrip(encoded: TextAndEncoding): void {
		assert.equalBytes(input, inputCopy, "Input buffer has been overwritten!");
		assert.strictEqual(encoded.encoding.decode(encoded.text), inputString);
	}

	it("should round-trip and respect isEncodingOk", () => {
		for (const smallest of [true, false])
		for (const shouldAccept of [true, false]) {
			const options: EncodeOptionsWithIsEncodingOk = {
				isEncodingOk() {
					return shouldAccept;
				}
			};

			const encoded = (smallest ? transcodeSmallest : transcodeFastest)(input, inputEncoding, options);

			(shouldAccept ? assert.isNotNull : assert.isNull)(encoded);
			if (shouldAccept)
				checkRoundTrip(encoded!);
		}
	});
});