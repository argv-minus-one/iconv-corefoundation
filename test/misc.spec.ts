import * as Chai from "chai";
import { encode, encodingExists } from "..";
import ChaiBytes = require("chai-bytes");

Chai.use(ChaiBytes);
const { assert } = Chai;

// Check for segfaults in native finalizers.
afterEach(global.gc);

describe("encodingExists", () => {
	it("should return true for encodings that exist", () => {
		assert.isTrue(encodingExists("us-ascii"));
	});

	it("should return false for encodings that don't exist", () => {
		assert.isFalse(encodingExists("FOOBIE BLETCH"));
	});
});
