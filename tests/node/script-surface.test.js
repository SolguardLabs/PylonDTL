import assert from "node:assert/strict";
import test from "node:test";

import { expectDigest, runScript } from "../helpers/pylon-cli.js";

test("script ejecuta flujo externo reproducible", () => {
  const report = runScript([
    "SCENARIO scripted-flow",
    "OPEN ses-script atlas USDC rtgs atlas-funding balanced",
    "ORDER ord-script ses-script atlas USDC rtgs atlas-funding beneficiary-a 150000000 150000000 10 5",
    "QUOTE atlas USDC rtgs 75000000 10",
    "NOTE scripted rtgs settlement completed",
  ]);

  assert.equal(report.scenario, "scripted-flow");
  assert.equal(report.sessions.opened, 1);
  assert.equal(report.receipts.settled, 1);
  assert.equal(report.stats.settled_notional, 150_000_000);
  assert.equal(report.stats.collected_fees, 60_000);
  assert.equal(report.stats.reserved_notional, 2_250_000);
  assert.equal(report.exposure.global, 152_250_000);
  assert.equal(report.ledger.accounts["beneficiary-a"].USDC, 147_690_000);
  assert.equal(report.quotes.length, 1);
  assert.equal(report.quotes[0].accepted, true);
  assert.equal(report.notes[0], "scripted rtgs settlement completed");
  expectDigest(report.state_digest);
});
