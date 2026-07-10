import assert from "node:assert/strict";
import test from "node:test";

import { expectDigest, runScenario } from "../helpers/pylon-cli.js";

test("single liquida un flujo rtgs y separa fee, reserve y net amount", () => {
  const report = runScenario("single");

  assert.equal(report.scenario, "single");
  assert.equal(report.stats.orders_submitted, 1);
  assert.equal(report.stats.orders_accepted, 1);
  assert.equal(report.stats.orders_rejected, 0);
  assert.equal(report.stats.settled_notional, 400_000_000);
  assert.equal(report.stats.collected_fees, 160_000);
  assert.equal(report.stats.reserved_notional, 6_000_000);
  assert.equal(report.exposure.by_policy["atlas:USDC:rtgs"], 406_000_000);
  assert.equal(report.ledger.accounts["beneficiary-a"].USDC, 393_840_000);
  assert.equal(report.ledger.accounts["fee-rtgs"].USDC, 160_000);
  assert.equal(report.ledger.accounts["reserve-rtgs"].USDC, 6_000_000);
  assert.equal(report.ledger.conservation_ok, true);
  assert.equal(report.sessions.log[0].remaining, 1_439_039_139);
  expectDigest(report.receipts.log[0].digest);
});

test("portfolio coordina multiples operadores, assets y canales", () => {
  const report = runScenario("portfolio");

  assert.equal(report.scenario, "portfolio");
  assert.equal(report.sessions.opened, 4);
  assert.equal(report.receipts.settled, 4);
  assert.equal(report.stats.settled_notional, 1_950_000_000);
  assert.equal(report.stats.collected_fees, 595_000);
  assert.equal(report.exposure.by_operator.atlas, 712_600_000);
  assert.equal(report.exposure.by_operator.delta, 916_200_000);
  assert.equal(report.exposure.by_asset.USDC, 1_069_700_000);
  assert.equal(report.exposure.by_asset.TBILL, 916_200_000);
  assert.equal(report.ledger.accounts["prime-broker"].USDC, 687_190_000);
  assert.equal(report.ledger.accounts["clearing-house"].TBILL, 883_620_000);
  assert.equal(report.quotes[0].accepted, false);
  assert.equal(report.quotes[1].accepted, true);
  assert.equal(report.ledger.conservation_ok, true);
});

test("channels cubre rtgs y l2 con accounting por asset", () => {
  const report = runScenario("channels");

  assert.equal(report.scenario, "channels");
  assert.equal(report.receipts.settled, 2);
  assert.equal(report.exposure.by_channel.l2fast, 154_500_000);
  assert.equal(report.exposure.by_channel.rtgs, 203_600_000);
  assert.equal(report.ledger.accounts["beneficiary-b"].SOL, 145_380_000);
  assert.equal(report.ledger.accounts["beneficiary-a"].EURC, 196_300_000);
  assert.equal(report.ledger.accounts["fee-l2fast"].SOL, 120_000);
  assert.equal(report.ledger.accounts["reserve-rtgs"].EURC, 3_600_000);
  assert.equal(report.ledger.conservation_ok, true);
});
