#include "pylon.hpp"

namespace pylon {
namespace {

OrderIntent make_order(
    const std::string& id,
    const std::string& session_id,
    const std::string& op,
    const std::string& asset,
    const std::string& channel,
    const std::string& source,
    const std::string& beneficiary,
    Amount notional,
    Amount units,
    int max_fee_bps,
    int priority
) {
    OrderIntent intent;
    intent.id = id;
    intent.session_id = session_id;
    intent.operator_id = op;
    intent.asset_id = asset;
    intent.channel_id = channel;
    intent.source_account = source;
    intent.beneficiary_account = beneficiary;
    intent.side = OrderSide::Debit;
    intent.notional = notional;
    intent.units = units;
    intent.requested_fee_bps = max_fee_bps;
    intent.priority = priority;
    intent.settlement_ref = "scenario:" + id;
    return intent;
}

void add_core_quotes(ScenarioReport& report) {
    report.quotes.push_back(report.engine.quote(PolicyKey("atlas", "USDC", "rtgs"), 100000000, 10));
    report.quotes.push_back(report.engine.quote(PolicyKey("atlas", "USDC", "netex"), 250000000, 10));
    report.quotes.push_back(report.engine.quote(PolicyKey("boron", "USDC", "netex"), 125000000, 12));
}

}  // namespace

ScenarioReport ScenarioRunner::run(const std::string& name) const {
    const std::string key = lowercase(name);
    if (key == "snapshot") {
        return snapshot();
    }
    if (key == "single") {
        return single();
    }
    if (key == "epoch") {
        return epoch();
    }
    if (key == "portfolio") {
        return portfolio();
    }
    if (key == "channels") {
        return channels();
    }
    if (starts_with(key, "script:")) {
        return script(name.substr(7));
    }
    throw RuntimeError("unknown scenario: " + name);
}

ScenarioReport ScenarioRunner::snapshot() const {
    ScenarioReport report;
    report.name = "snapshot";
    report.engine.bootstrap_default();
    add_core_quotes(report);
    report.notes.push_back("baseline policy catalogue loaded");
    report.notes.push_back("no execution sessions are active");
    return report;
}

ScenarioReport ScenarioRunner::single() const {
    ScenarioReport report;
    report.name = "single";
    report.engine.bootstrap_default();
    report.engine.open_session("ses-atlas-1", "atlas", "USDC", "rtgs", "atlas-funding", SessionMode::Balanced);
    report.engine.execute(make_order(
        "ord-atlas-1",
        "ses-atlas-1",
        "atlas",
        "USDC",
        "rtgs",
        "atlas-funding",
        "beneficiary-a",
        400000000,
        400000000,
        10,
        5
    ));
    report.quotes.push_back(report.engine.quote(PolicyKey("atlas", "USDC", "rtgs"), 200000000, 10));
    report.notes.push_back("single operator settlement path completed");
    return report;
}

ScenarioReport ScenarioRunner::epoch() const {
    ScenarioReport report;
    report.name = "epoch";
    report.engine.bootstrap_default();
    report.engine.open_session("ses-atlas-pre", "atlas", "USDC", "rtgs", "atlas-funding", SessionMode::Balanced);
    report.engine.execute(make_order(
        "ord-atlas-pre",
        "ses-atlas-pre",
        "atlas",
        "USDC",
        "rtgs",
        "atlas-funding",
        "beneficiary-a",
        300000000,
        300000000,
        10,
        4
    ));
    report.engine.rotate_epoch("compressed-liquidity", 6200, 7600, 8200);
    report.engine.open_session("ses-atlas-post", "atlas", "USDC", "rtgs", "atlas-funding", SessionMode::Immediate);
    report.engine.execute(make_order(
        "ord-atlas-post",
        "ses-atlas-post",
        "atlas",
        "USDC",
        "rtgs",
        "atlas-funding",
        "beneficiary-b",
        100000000,
        100000000,
        10,
        8
    ));
    report.quotes.push_back(report.engine.quote(PolicyKey("atlas", "USDC", "rtgs"), 150000000, 10));
    report.quotes.push_back(report.engine.quote(PolicyKey("atlas", "USDC", "netex"), 150000000, 10));
    report.notes.push_back("epoch transition recomputed visible quota surface");
    return report;
}

ScenarioReport ScenarioRunner::portfolio() const {
    ScenarioReport report;
    report.name = "portfolio";
    report.engine.bootstrap_default();
    report.engine.open_session("ses-atlas-net", "atlas", "USDC", "netex", "atlas-funding", SessionMode::NetOnly);
    report.engine.open_session("ses-boron-usdc", "boron", "USDC", "netex", "boron-funding", SessionMode::Balanced);
    report.engine.open_session("ses-delta-tbill", "delta", "TBILL", "netex", "delta-funding", SessionMode::Agency);
    report.engine.open_session("ses-cygnus-custody", "cygnus", "USDC", "custody", "cygnus-funding", SessionMode::Agency);
    report.engine.execute(make_order(
        "ord-atlas-net",
        "ses-atlas-net",
        "atlas",
        "USDC",
        "netex",
        "atlas-funding",
        "prime-broker",
        700000000,
        700000000,
        10,
        6
    ));
    report.engine.execute(make_order(
        "ord-boron-usdc",
        "ses-boron-usdc",
        "boron",
        "USDC",
        "netex",
        "boron-funding",
        "settlement-maker",
        250000000,
        250000000,
        12,
        5
    ));
    report.engine.execute(make_order(
        "ord-delta-tbill",
        "ses-delta-tbill",
        "delta",
        "TBILL",
        "netex",
        "delta-funding",
        "clearing-house",
        900000000,
        900000000,
        8,
        9
    ));
    report.engine.execute(make_order(
        "ord-cygnus-custody",
        "ses-cygnus-custody",
        "cygnus",
        "USDC",
        "custody",
        "cygnus-funding",
        "agency-broker",
        100000000,
        100000000,
        12,
        3
    ));
    report.quotes.push_back(report.engine.quote(PolicyKey("boron", "SOL", "l2fast"), 150000000, 12));
    report.quotes.push_back(report.engine.quote(PolicyKey("delta", "TBILL", "netex"), 200000000, 8));
    report.notes.push_back("portfolio scenario spans principal, agency and clearing desks");
    return report;
}

ScenarioReport ScenarioRunner::channels() const {
    ScenarioReport report;
    report.name = "channels";
    report.engine.bootstrap_default();
    report.engine.open_session("ses-boron-l2", "boron", "SOL", "l2fast", "boron-funding", SessionMode::Immediate);
    report.engine.open_session("ses-atlas-eurc", "atlas", "EURC", "rtgs", "atlas-funding", SessionMode::Balanced);
    report.engine.execute(make_order(
        "ord-boron-l2",
        "ses-boron-l2",
        "boron",
        "SOL",
        "l2fast",
        "boron-funding",
        "beneficiary-b",
        150000000,
        150000000,
        12,
        7
    ));
    report.engine.execute(make_order(
        "ord-atlas-eurc",
        "ses-atlas-eurc",
        "atlas",
        "EURC",
        "rtgs",
        "atlas-funding",
        "beneficiary-a",
        200000000,
        200000000,
        12,
        5
    ));
    report.quotes.push_back(report.engine.quote(PolicyKey("cygnus", "USDC", "custody"), 100000000, 12));
    report.notes.push_back("channel mix covers rtgs and l2 settlement");
    return report;
}

ScenarioReport ScenarioRunner::script(const std::string& path) const {
    ScriptRunner runner;
    return runner.run_file(path);
}

}  // namespace pylon
