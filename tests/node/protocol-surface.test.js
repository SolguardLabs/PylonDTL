import assert from "node:assert/strict";
import test from "node:test";

import { expectDigest, runScenario } from "../helpers/pylon-cli.js";

test("snapshot inicializa catalogo operativo y cuotas visibles", () => {
  const report = runScenario("snapshot");

  assert.equal(report.scenario, "snapshot");
  assert.deepEqual(report.surface.assets, 4);
  assert.deepEqual(report.surface.operators, 4);
  assert.deepEqual(report.surface.channels, 4);
  assert.deepEqual(report.surface.policies, 7);
  assert.equal(report.epoch.id, 1);
  assert.equal(report.ledger.conservation_ok, true);
  assert.equal(report.ledger.totals.USDC, 10_550_000_000);
  assert.equal(report.sessions.opened, 0);
  assert.equal(report.receipts.total, 0);
  assert.equal(report.quotes.length, 3);
  assert.equal(report.quotes[0].key, "atlas:USDC:rtgs");
  assert.equal(report.quotes[0].accepted, true);
  assert.equal(report.quotes[0].net_amount, 98_460_000);
  expectDigest(report.state_digest);
  expectDigest(report.surface.quota_log[0].digest);
});

test("epoch recalcula superficie de cuotas sin alterar conservacion contable", () => {
  const report = runScenario("epoch");

  assert.equal(report.scenario, "epoch");
  assert.equal(report.epoch.id, 2);
  assert.equal(report.epoch.label, "compressed-liquidity");
  assert.equal(report.stats.orders_accepted, 2);
  assert.equal(report.receipts.settled, 2);
  assert.equal(report.exposure.global, 406_000_000);
  assert.equal(report.exposure.by_operator.atlas, 406_000_000);
  assert.equal(report.ledger.conservation_ok, true);

  const postSession = report.sessions.log.find((session) => session.id === "ses-atlas-post");
  assert.equal(postSession.epoch_opened, 2);
  assert.equal(postSession.committed_notional, 100_000_000);

  const quota = report.surface.quota_log.find((item) => item.key === "atlas:USDC:rtgs");
  assert.equal(quota.epoch_id, 2);
  assert.equal(quota.remaining, 764_000_000);
});
