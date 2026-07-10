#include "pylon.hpp"

#include <ostream>

namespace pylon {
namespace {

void comma(std::ostream& out, bool& first) {
    if (!first) {
        out << ",";
    }
    first = false;
}

void json_key(std::ostream& out, const std::string& key) {
    out << "\"" << json_escape(key) << "\":";
}

void json_string(std::ostream& out, const std::string& value) {
    out << "\"" << json_escape(value) << "\"";
}

void json_bool(std::ostream& out, bool value) {
    out << (value ? "true" : "false");
}

void write_amount_map(std::ostream& out, const std::map<std::string, Amount>& values) {
    out << "{";
    bool first = true;
    for (const auto& pair : values) {
        comma(out, first);
        json_key(out, pair.first);
        out << pair.second;
    }
    out << "}";
}

void write_string_array(std::ostream& out, const std::vector<std::string>& values) {
    out << "[";
    bool first = true;
    for (const std::string& value : values) {
        comma(out, first);
        json_string(out, value);
    }
    out << "]";
}

void write_asset(std::ostream& out, const AssetProfile& asset) {
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "id");
    json_string(out, asset.id);
    comma(out, first);
    json_key(out, "symbol");
    json_string(out, asset.symbol);
    comma(out, first);
    json_key(out, "kind");
    json_string(out, to_string(asset.kind));
    comma(out, first);
    json_key(out, "decimals");
    out << asset.decimals;
    comma(out, first);
    json_key(out, "volatility_bps");
    out << asset.volatility_bps;
    comma(out, first);
    json_key(out, "collateral_weight_bps");
    out << asset.collateral_weight_bps;
    comma(out, first);
    json_key(out, "enabled");
    json_bool(out, asset.enabled);
    out << "}";
}

void write_operator(std::ostream& out, const OperatorProfile& op) {
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "id");
    json_string(out, op.id);
    comma(out, first);
    json_key(out, "desk");
    json_string(out, op.desk);
    comma(out, first);
    json_key(out, "tier");
    json_string(out, to_string(op.tier));
    comma(out, first);
    json_key(out, "trust_bps");
    out << op.trust_bps;
    comma(out, first);
    json_key(out, "max_sessions");
    out << op.max_sessions;
    comma(out, first);
    json_key(out, "enabled");
    json_bool(out, op.enabled);
    out << "}";
}

void write_channel(std::ostream& out, const ChannelProfile& channel) {
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "id");
    json_string(out, channel.id);
    comma(out, first);
    json_key(out, "venue");
    json_string(out, channel.venue);
    comma(out, first);
    json_key(out, "class");
    json_string(out, to_string(channel.channel_class));
    comma(out, first);
    json_key(out, "latency_seconds");
    out << channel.latency_seconds;
    comma(out, first);
    json_key(out, "settlement_fee_bps");
    out << channel.settlement_fee_bps;
    comma(out, first);
    json_key(out, "reserve_floor_bps");
    out << channel.reserve_floor_bps;
    comma(out, first);
    json_key(out, "netting_supported");
    json_bool(out, channel.netting_supported);
    out << "}";
}

void write_policy(std::ostream& out, const LimitPolicy& policy) {
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "key");
    json_string(out, policy.key.str());
    comma(out, first);
    json_key(out, "operator_limit");
    out << policy.operator_limit;
    comma(out, first);
    json_key(out, "asset_limit");
    out << policy.asset_limit;
    comma(out, first);
    json_key(out, "channel_limit");
    out << policy.channel_limit;
    comma(out, first);
    json_key(out, "global_limit");
    out << policy.global_limit;
    comma(out, first);
    json_key(out, "min_order_notional");
    out << policy.min_order_notional;
    comma(out, first);
    json_key(out, "max_order_notional");
    out << policy.max_order_notional;
    comma(out, first);
    json_key(out, "fee_bps");
    out << policy.fee_bps;
    comma(out, first);
    json_key(out, "reserve_bps");
    out << policy.reserve_bps;
    comma(out, first);
    json_key(out, "enabled");
    json_bool(out, policy.enabled);
    out << "}";
}

void write_quota(std::ostream& out, const QuotaState& quota) {
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "key");
    json_string(out, quota.key.str());
    comma(out, first);
    json_key(out, "epoch_id");
    out << quota.epoch_id;
    comma(out, first);
    json_key(out, "remaining");
    out << quota.strictest_remaining();
    comma(out, first);
    json_key(out, "operator_limit");
    out << quota.operator_limit;
    comma(out, first);
    json_key(out, "asset_limit");
    out << quota.asset_limit;
    comma(out, first);
    json_key(out, "channel_limit");
    out << quota.channel_limit;
    comma(out, first);
    json_key(out, "global_limit");
    out << quota.global_limit;
    comma(out, first);
    json_key(out, "global_observed");
    out << quota.global_observed;
    comma(out, first);
    json_key(out, "enabled");
    json_bool(out, quota.enabled);
    comma(out, first);
    json_key(out, "digest");
    json_string(out, quota.digest());
    out << "}";
}

void write_session_object(std::ostream& out, const Session& session) {
    out << "{";
    bool first = true;
    const Amount local = checked_add(session.committed_notional, session.committed_reserve);
    comma(out, first);
    json_key(out, "id");
    json_string(out, session.id);
    comma(out, first);
    json_key(out, "operator_id");
    json_string(out, session.operator_id);
    comma(out, first);
    json_key(out, "asset_id");
    json_string(out, session.asset_id);
    comma(out, first);
    json_key(out, "channel_id");
    json_string(out, session.channel_id);
    comma(out, first);
    json_key(out, "funding_account");
    json_string(out, session.funding_account);
    comma(out, first);
    json_key(out, "mode");
    json_string(out, to_string(session.mode));
    comma(out, first);
    json_key(out, "epoch_opened");
    out << session.snapshot.epoch_id;
    comma(out, first);
    json_key(out, "opened_sequence");
    out << session.opened_sequence;
    comma(out, first);
    json_key(out, "committed_notional");
    out << session.committed_notional;
    comma(out, first);
    json_key(out, "committed_reserve");
    out << session.committed_reserve;
    comma(out, first);
    json_key(out, "orders");
    out << session.orders;
    comma(out, first);
    json_key(out, "remaining");
    out << session.snapshot.strictest_remaining(local);
    comma(out, first);
    json_key(out, "open");
    json_bool(out, session.open);
    comma(out, first);
    json_key(out, "snapshot_digest");
    json_string(out, session.snapshot.digest());
    comma(out, first);
    json_key(out, "digest");
    json_string(out, session.digest());
    out << "}";
}

void write_receipt_object(std::ostream& out, const ExecutionReceipt& receipt) {
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "id");
    json_string(out, receipt.id);
    comma(out, first);
    json_key(out, "order_id");
    json_string(out, receipt.order_id);
    comma(out, first);
    json_key(out, "session_id");
    json_string(out, receipt.session_id);
    comma(out, first);
    json_key(out, "operator_id");
    json_string(out, receipt.operator_id);
    comma(out, first);
    json_key(out, "asset_id");
    json_string(out, receipt.asset_id);
    comma(out, first);
    json_key(out, "channel_id");
    json_string(out, receipt.channel_id);
    comma(out, first);
    json_key(out, "epoch_id");
    out << receipt.epoch_id;
    comma(out, first);
    json_key(out, "sequence");
    out << receipt.sequence;
    comma(out, first);
    json_key(out, "notional");
    out << receipt.notional;
    comma(out, first);
    json_key(out, "fee");
    out << receipt.fee;
    comma(out, first);
    json_key(out, "reserve");
    out << receipt.reserve;
    comma(out, first);
    json_key(out, "net_amount");
    out << receipt.net_amount;
    comma(out, first);
    json_key(out, "code");
    json_string(out, to_string(receipt.code));
    comma(out, first);
    json_key(out, "status");
    json_string(out, receipt.status);
    comma(out, first);
    json_key(out, "digest");
    json_string(out, receipt.digest());
    out << "}";
}

void write_quote_object(std::ostream& out, const QuoteResult& quote) {
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "key");
    json_string(out, quote.key.str());
    comma(out, first);
    json_key(out, "accepted");
    json_bool(out, quote.accepted);
    comma(out, first);
    json_key(out, "requested_notional");
    out << quote.requested_notional;
    comma(out, first);
    json_key(out, "fee");
    out << quote.fee;
    comma(out, first);
    json_key(out, "reserve");
    out << quote.reserve;
    comma(out, first);
    json_key(out, "net_amount");
    out << quote.net_amount;
    comma(out, first);
    json_key(out, "remaining");
    out << quote.remaining;
    comma(out, first);
    json_key(out, "reason");
    json_string(out, quote.reason);
    comma(out, first);
    json_key(out, "digest");
    json_string(out, quote.digest());
    out << "}";
}

}  // namespace

std::string ReportBuilder::build(const ScenarioReport& report, ReportVerbosity verbosity) const {
    std::ostringstream out;
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "scenario");
    json_string(out, report.name);
    comma(out, first);
    json_key(out, "state_digest");
    json_string(out, report.engine.state_digest());
    comma(out, first);
    json_key(out, "surface");
    write_surface(out, report.engine);
    comma(out, first);
    json_key(out, "epoch");
    write_epoch(out, report.engine);
    comma(out, first);
    json_key(out, "stats");
    write_stats(out, report.engine);
    comma(out, first);
    json_key(out, "exposure");
    write_exposure(out, report.engine);
    comma(out, first);
    json_key(out, "ledger");
    write_ledger(out, report.engine, verbosity);
    comma(out, first);
    json_key(out, "sessions");
    write_sessions(out, report.engine, verbosity);
    comma(out, first);
    json_key(out, "receipts");
    write_receipts(out, report.engine, verbosity);
    comma(out, first);
    json_key(out, "quotes");
    write_quotes(out, report.quotes);
    comma(out, first);
    json_key(out, "notes");
    write_notes(out, report.notes);
    out << "}\n";
    return out.str();
}

void ReportBuilder::write_surface(std::ostream& out, const RiskEngine& engine) const {
    const std::vector<AssetProfile> assets = engine.policies().assets();
    const std::vector<OperatorProfile> operators = engine.policies().operators();
    const std::vector<ChannelProfile> channels = engine.policies().channels();
    const std::vector<LimitPolicy> policies = engine.policies().policies();
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "assets");
    out << assets.size();
    comma(out, first);
    json_key(out, "operators");
    out << operators.size();
    comma(out, first);
    json_key(out, "channels");
    out << channels.size();
    comma(out, first);
    json_key(out, "policies");
    out << policies.size();
    comma(out, first);
    json_key(out, "asset_log");
    out << "[";
    bool item_first = true;
    for (const AssetProfile& asset : assets) {
        comma(out, item_first);
        write_asset(out, asset);
    }
    out << "]";
    comma(out, first);
    json_key(out, "operator_log");
    out << "[";
    item_first = true;
    for (const OperatorProfile& op : operators) {
        comma(out, item_first);
        write_operator(out, op);
    }
    out << "]";
    comma(out, first);
    json_key(out, "channel_log");
    out << "[";
    item_first = true;
    for (const ChannelProfile& channel : channels) {
        comma(out, item_first);
        write_channel(out, channel);
    }
    out << "]";
    comma(out, first);
    json_key(out, "policy_log");
    out << "[";
    item_first = true;
    for (const LimitPolicy& policy : policies) {
        comma(out, item_first);
        write_policy(out, policy);
    }
    out << "]";
    comma(out, first);
    json_key(out, "quota_log");
    out << "[";
    item_first = true;
    for (const QuotaState& quota : engine.policies().all_quotas(engine.exposure().totals())) {
        comma(out, item_first);
        write_quota(out, quota);
    }
    out << "]";
    out << "}";
}

void ReportBuilder::write_epoch(std::ostream& out, const RiskEngine& engine) const {
    const EpochState epoch = engine.policies().epoch();
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "id");
    out << epoch.id;
    comma(out, first);
    json_key(out, "label");
    json_string(out, epoch.label);
    comma(out, first);
    json_key(out, "risk_bias_bps");
    out << epoch.risk_bias_bps;
    comma(out, first);
    json_key(out, "liquidity_bias_bps");
    out << epoch.liquidity_bias_bps;
    comma(out, first);
    json_key(out, "settlement_bias_bps");
    out << epoch.settlement_bias_bps;
    comma(out, first);
    json_key(out, "sequence");
    out << epoch.sequence;
    comma(out, first);
    json_key(out, "digest");
    json_string(out, epoch.digest());
    out << "}";
}

void ReportBuilder::write_stats(std::ostream& out, const RiskEngine& engine) const {
    const EngineStats stats = engine.stats();
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "sessions_opened");
    out << stats.sessions_opened;
    comma(out, first);
    json_key(out, "sessions_closed");
    out << stats.sessions_closed;
    comma(out, first);
    json_key(out, "orders_submitted");
    out << stats.orders_submitted;
    comma(out, first);
    json_key(out, "orders_accepted");
    out << stats.orders_accepted;
    comma(out, first);
    json_key(out, "orders_rejected");
    out << stats.orders_rejected;
    comma(out, first);
    json_key(out, "settled_notional");
    out << stats.settled_notional;
    comma(out, first);
    json_key(out, "collected_fees");
    out << stats.collected_fees;
    comma(out, first);
    json_key(out, "reserved_notional");
    out << stats.reserved_notional;
    out << "}";
}

void ReportBuilder::write_exposure(std::ostream& out, const RiskEngine& engine) const {
    const ExposureTotals totals = engine.exposure().totals();
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "global");
    out << totals.global;
    comma(out, first);
    json_key(out, "by_operator");
    write_amount_map(out, totals.by_operator);
    comma(out, first);
    json_key(out, "by_asset");
    write_amount_map(out, totals.by_asset);
    comma(out, first);
    json_key(out, "by_channel");
    write_amount_map(out, totals.by_channel);
    comma(out, first);
    json_key(out, "by_policy");
    write_amount_map(out, totals.by_policy);
    comma(out, first);
    json_key(out, "debt_by_operator");
    write_amount_map(out, totals.debt_by_operator);
    comma(out, first);
    json_key(out, "digest");
    json_string(out, engine.exposure().digest());
    out << "}";
}

void ReportBuilder::write_ledger(std::ostream& out, const RiskEngine& engine, ReportVerbosity verbosity) const {
    const std::vector<AccountSnapshot> accounts = engine.ledger().accounts();
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "conservation_ok");
    json_bool(out, engine.ledger().conservation_ok());
    comma(out, first);
    json_key(out, "totals");
    write_amount_map(out, engine.ledger().totals());
    comma(out, first);
    json_key(out, "movement_count");
    out << engine.ledger().movements().size();
    comma(out, first);
    json_key(out, "accounts");
    out << "{";
    bool account_first = true;
    for (const AccountSnapshot& account : accounts) {
        comma(out, account_first);
        json_key(out, account.id);
        write_amount_map(out, account.balances);
    }
    out << "}";
    if (verbosity == ReportVerbosity::Full) {
        comma(out, first);
        json_key(out, "movements");
        out << "[";
        bool move_first = true;
        for (const LedgerMovement& movement : engine.ledger().movements()) {
            comma(out, move_first);
            out << "{";
            bool item_first = true;
            comma(out, item_first);
            json_key(out, "sequence");
            out << movement.sequence;
            comma(out, item_first);
            json_key(out, "kind");
            json_string(out, to_string(movement.kind));
            comma(out, item_first);
            json_key(out, "asset_id");
            json_string(out, movement.asset_id);
            comma(out, item_first);
            json_key(out, "from");
            json_string(out, movement.from_account);
            comma(out, item_first);
            json_key(out, "to");
            json_string(out, movement.to_account);
            comma(out, item_first);
            json_key(out, "amount");
            out << movement.amount;
            comma(out, item_first);
            json_key(out, "reference");
            json_string(out, movement.reference);
            comma(out, item_first);
            json_key(out, "digest");
            json_string(out, movement.digest());
            out << "}";
        }
        out << "]";
    }
    comma(out, first);
    json_key(out, "digest");
    json_string(out, engine.ledger().digest());
    out << "}";
}

void ReportBuilder::write_sessions(std::ostream& out, const RiskEngine& engine, ReportVerbosity verbosity) const {
    const std::vector<Session> sessions = engine.sessions().sessions();
    int active = 0;
    for (const Session& session : sessions) {
        if (session.open) {
            ++active;
        }
    }
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "opened");
    out << engine.sessions().opened();
    comma(out, first);
    json_key(out, "closed");
    out << engine.sessions().closed();
    comma(out, first);
    json_key(out, "active");
    out << active;
    comma(out, first);
    json_key(out, "digest");
    json_string(out, engine.sessions().digest());
    if (verbosity != ReportVerbosity::Compact) {
        comma(out, first);
        json_key(out, "log");
        out << "[";
        bool item_first = true;
        for (const Session& session : sessions) {
            comma(out, item_first);
            write_session_object(out, session);
        }
        out << "]";
    }
    out << "}";
}

void ReportBuilder::write_receipts(std::ostream& out, const RiskEngine& engine, ReportVerbosity verbosity) const {
    int settled = 0;
    int rejected = 0;
    for (const ExecutionReceipt& receipt : engine.receipts()) {
        if (receipt.settled()) {
            ++settled;
        } else {
            ++rejected;
        }
    }
    out << "{";
    bool first = true;
    comma(out, first);
    json_key(out, "total");
    out << engine.receipts().size();
    comma(out, first);
    json_key(out, "settled");
    out << settled;
    comma(out, first);
    json_key(out, "rejected");
    out << rejected;
    if (verbosity != ReportVerbosity::Compact) {
        comma(out, first);
        json_key(out, "log");
        out << "[";
        bool item_first = true;
        for (const ExecutionReceipt& receipt : engine.receipts()) {
            comma(out, item_first);
            write_receipt_object(out, receipt);
        }
        out << "]";
    }
    out << "}";
}

void ReportBuilder::write_quotes(std::ostream& out, const std::vector<QuoteResult>& quotes) const {
    out << "[";
    bool first = true;
    for (const QuoteResult& quote : quotes) {
        comma(out, first);
        write_quote_object(out, quote);
    }
    out << "]";
}

void ReportBuilder::write_notes(std::ostream& out, const std::vector<std::string>& notes) const {
    write_string_array(out, notes);
}

}  // namespace pylon
