import * as Chai from "chai";
import { encodingExists } from "..";
import ChaiBytes = require("chai-bytes");

Chai.use(ChaiBytes);
const { assert } = Chai;

describe("encodingExists", () => {
	it("should return true for encodings that exist", () => {
		assert.isTrue(encodingExists("us-ascii"));
	});

	it("should return false for encodings that don't exist", () => {
		assert.isFalse(encodingExists("FOOBIE BLETCH"));
	});
});
