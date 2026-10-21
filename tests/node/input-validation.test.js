import assert from "node:assert/strict";
import test from "node:test";

import { runScript } from "../helpers/pylon-cli.js";

test("rejects amounts outside the signed integer domain", () => {
  assert.throws(
    () => runScript(["SCENARIO invalid-amount", "QUOTE atlas USDC rtgs 9223372036854775808 10"]),
    /amount out of range/,
  );
});

test("rejects epoch biases outside the integer domain", () => {
  assert.throws(
    () => runScript(["SCENARIO invalid-epoch", "EPOCH changed 9223372036854775808 10000 10000"]),
    /integer out of range/,
  );
});
