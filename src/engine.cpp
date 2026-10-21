#include "pylon.hpp"

namespace pylon {
namespace {

AssetProfile asset_profile(
    const std::string& id,
    const std::string& symbol,
    AssetKind kind,
    int decimals,
    int volatility_bps,
    int collateral_weight_bps,
    Amount lot
) {
    AssetProfile asset;
    asset.id = id;
    asset.symbol = symbol;
    asset.kind = kind;
    asset.decimals = decimals;
    asset.volatility_bps = volatility_bps;
    asset.collateral_weight_bps = collateral_weight_bps;
    asset.settlement_lot = lot;
    return asset;
}

OperatorProfile operator_profile(
    const std::string& id,
    const std::string& desk,
    OperatorTier tier,
    int trust_bps,
    int buffer_bps,
    int max_sessions
) {
    OperatorProfile op;
    op.id = id;
    op.desk = desk;
    op.tier = tier;
    op.trust_bps = trust_bps;
    op.internal_buffer_bps = buffer_bps;
    op.max_sessions = max_sessions;
    return op;
}

ChannelProfile channel_profile(
    const std::string& id,
    const std::string& venue,
    ChannelClass channel_class,
    int latency_seconds,
    int fee_bps,
    int risk_bps,
    int reserve_floor_bps
) {
    ChannelProfile channel;
    channel.id = id;
    channel.venue = venue;
    channel.channel_class = channel_class;
    channel.latency_seconds = latency_seconds;
    channel.settlement_fee_bps = fee_bps;
    channel.channel_risk_bps = risk_bps;
    channel.reserve_floor_bps = reserve_floor_bps;
    channel.netting_supported = channel_class == ChannelClass::Netting || channel_class == ChannelClass::LayerTwo;
    return channel;
}

LimitPolicy policy_for(
    const std::string& op,
    const std::string& asset,
    const std::string& channel,
    Amount operator_limit,
    Amount asset_limit,
    Amount channel_limit,
    Amount global_limit,
    Amount min_order,
    Amount max_order,
    int fee_bps,
    int reserve_bps,
    int stress_floor_bps,
    int max_orders
) {
    LimitPolicy policy;
    policy.key = PolicyKey(op, asset, channel);
    policy.operator_limit = operator_limit;
    policy.asset_limit = asset_limit;
    policy.channel_limit = channel_limit;
    policy.global_limit = global_limit;
    policy.min_order_notional = min_order;
    policy.max_order_notional = max_order;
    policy.fee_bps = fee_bps;
    policy.max_fee_bps = 25;
    policy.reserve_bps = reserve_bps;
    policy.stress_floor_bps = stress_floor_bps;
    policy.session_ttl_seconds = 900;
    policy.max_orders_per_session = max_orders;
    policy.allow_netting = true;
    return policy;
}

}  // namespace

RiskEngine::RiskEngine() = default;

void RiskEngine::bootstrap_default() {
    policy_ = PolicyBook{};
    ledger_ = LedgerBook{};
    exposure_ = ExposureBook{};
    sessions_ = SessionManager{};
    receipts_.clear();
    decisions_.clear();
    stats_ = EngineStats{};
    sequence_ = 0;

    policy_.register_asset(asset_profile("USDC", "USDC", AssetKind::Stablecoin, 6, 40, 10000, 1000));
    policy_.register_asset(asset_profile("EURC", "EURC", AssetKind::Stablecoin, 6, 75, 9900, 1000));
    policy_.register_asset(asset_profile("SOL", "SOL", AssetKind::Native, 9, 900, 8400, 1));
    policy_.register_asset(asset_profile("TBILL", "TBL", AssetKind::TreasuryBill, 6, 15, 10200, 1000));

    policy_.register_operator(operator_profile("atlas", "principal-alpha", OperatorTier::Prime, 9800, 700, 12));
    policy_.register_operator(operator_profile("boron", "liquidity-beta", OperatorTier::MarketMaker, 9200, 550, 8));
    policy_.register_operator(operator_profile("cygnus", "agency-gamma", OperatorTier::Agency, 8600, 400, 6));
    policy_.register_operator(operator_profile("delta", "internal-delta", OperatorTier::Internal, 10000, 1000, 16));

    policy_.register_channel(channel_profile("rtgs", "PylonRTGS", ChannelClass::RealTime, 8, 1, 60, 120));
    policy_.register_channel(channel_profile("netex", "NetEx", ChannelClass::Netting, 120, 1, 120, 180));
    policy_.register_channel(channel_profile("l2fast", "L2Fast", ChannelClass::LayerTwo, 3, 2, 180, 220));
    policy_.register_channel(channel_profile("custody", "NorthVault", ChannelClass::Custody, 600, 3, 220, 260));

    policy_.set_policy(policy_for(
        "atlas",
        "USDC",
        "rtgs",
        1800000000,
        2200000000,
        2000000000,
        2400000000,
        50000000,
        900000000,
        3,
        150,
        6500,
        8
    ));
    policy_.set_policy(policy_for(
        "atlas",
        "USDC",
        "netex",
        2200000000,
        2600000000,
        2400000000,
        3000000000,
        25000000,
        1000000000,
        2,
        120,
        6800,
        10
    ));
    policy_.set_policy(policy_for(
        "atlas",
        "EURC",
        "rtgs",
        900000000,
        1200000000,
        1000000000,
        1400000000,
        25000000,
        450000000,
        4,
        180,
        7000,
        6
    ));
    policy_.set_policy(policy_for(
        "boron",
        "USDC",
        "netex",
        1100000000,
        1800000000,
        1600000000,
        1900000000,
        10000000,
        500000000,
        4,
        180,
        7000,
        8
    ));
    policy_.set_policy(policy_for(
        "boron",
        "SOL",
        "l2fast",
        700000000,
        900000000,
        850000000,
        1000000000,
        5000000,
        300000000,
        6,
        300,
        7400,
        6
    ));
    policy_.set_policy(policy_for(
        "cygnus",
        "USDC",
        "custody",
        600000000,
        900000000,
        750000000,
        1000000000,
        10000000,
        250000000,
        5,
        260,
        7600,
        5
    ));
    policy_.set_policy(policy_for(
        "delta",
        "TBILL",
        "netex",
        2500000000,
        2800000000,
        2600000000,
        3200000000,
        100000000,
        1200000000,
        1,
        80,
        7200,
        12
    ));

    for (const ChannelProfile& channel : policy_.channels()) {
        ledger_.create_account(channel.fee_account());
        ledger_.create_account(channel.reserve_account());
    }

    ledger_.deposit("atlas-funding", "USDC", 6000000000, "bootstrap");
    ledger_.deposit("atlas-funding", "EURC", 1800000000, "bootstrap");
    ledger_.deposit("boron-funding", "USDC", 3000000000, "bootstrap");
    ledger_.deposit("boron-funding", "SOL", 1500000000, "bootstrap");
    ledger_.deposit("cygnus-funding", "USDC", 1200000000, "bootstrap");
    ledger_.deposit("delta-funding", "TBILL", 5000000000, "bootstrap");
    ledger_.deposit("merchant-a", "USDC", 100000000, "bootstrap");
    ledger_.deposit("merchant-b", "USDC", 50000000, "bootstrap");
    ledger_.deposit("settlement-maker", "USDC", 200000000, "bootstrap");
    ledger_.create_account("prime-broker");
    ledger_.create_account("agency-broker");
    ledger_.create_account("clearing-house");
    ledger_.create_account("beneficiary-a");
    ledger_.create_account("beneficiary-b");
}

const PolicyBook& RiskEngine::policies() const {
    return policy_;
}

PolicyBook& RiskEngine::policies() {
    return policy_;
}

const LedgerBook& RiskEngine::ledger() const {
    return ledger_;
}

LedgerBook& RiskEngine::ledger() {
    return ledger_;
}

const ExposureBook& RiskEngine::exposure() const {
    return exposure_;
}

ExposureBook& RiskEngine::exposure() {
    return exposure_;
}

const SessionManager& RiskEngine::sessions() const {
    return sessions_;
}

SessionManager& RiskEngine::sessions() {
    return sessions_;
}

EngineStats RiskEngine::stats() const {
    return stats_;
}

Sequence RiskEngine::sequence() const {
    return sequence_;
}

Session RiskEngine::open_session(
    const std::string& id,
    const std::string& operator_id,
    const std::string& asset_id,
    const std::string& channel_id,
    const std::string& funding_account,
    SessionMode mode
) {
    const PolicyKey key(operator_id, asset_id, channel_id);
    const OperatorProfile& op = policy_.operator_profile(operator_id);
    if (sessions_.open_count_for_operator(operator_id) >= op.max_sessions) {
        throw RuntimeError("operator session capacity reached: " + operator_id);
    }
    if (!ledger_.has_account(funding_account)) {
        throw RuntimeError("unknown funding account: " + funding_account);
    }
    const LimitSnapshot snapshot = policy_.capture_snapshot(key, exposure_.totals());
    const Sequence sequence = next_sequence();
    Session session = sessions_.open_session(
        id,
        operator_id,
        asset_id,
        channel_id,
        funding_account,
        mode,
        snapshot,
        sequence
    );
    Session* stored = sessions_.find_session(id);
    if (stored) {
        stored->desk = op.desk;
    }
    stats_.sessions_opened = sessions_.opened();
    return session;
}

AdmissionDecision RiskEngine::admit(const OrderIntent& intent) const {
    AdmissionDecision decision;
    decision.code = AdmissionCode::Accepted;
    decision.reason = "accepted";

    const Session* session = sessions_.find_session(intent.session_id);
    if (!session) {
        decision.code = AdmissionCode::UnknownSession;
        decision.reason = "session not found";
        return decision;
    }
    if (!session->open) {
        decision.code = AdmissionCode::InactiveSession;
        decision.reason = "session closed";
        return decision;
    }
    if (session->operator_id != intent.operator_id) {
        decision.code = AdmissionCode::OperatorMismatch;
        decision.reason = "operator mismatch";
        return decision;
    }
    if (session->asset_id != intent.asset_id) {
        decision.code = AdmissionCode::AssetMismatch;
        decision.reason = "asset mismatch";
        return decision;
    }
    if (session->channel_id != intent.channel_id) {
        decision.code = AdmissionCode::ChannelMismatch;
        decision.reason = "channel mismatch";
        return decision;
    }
    if (!session->snapshot.enabled || !session->has_order_capacity()) {
        decision.code = AdmissionCode::PolicyClosed;
        decision.reason = "policy not available";
        return decision;
    }

    const auto maybe_policy = policy_.maybe_policy(intent.key());
    if (!maybe_policy || !maybe_policy->enabled) {
        decision.code = AdmissionCode::PolicyClosed;
        decision.reason = "policy not available";
        return decision;
    }
    const LimitPolicy policy = *maybe_policy;

    if (intent.notional < policy.min_order_notional) {
        decision.code = AdmissionCode::AmountTooSmall;
        decision.reason = "notional below minimum";
        return decision;
    }
    if (policy.max_order_notional > 0 && intent.notional > policy.max_order_notional) {
        decision.code = AdmissionCode::LimitExceeded;
        decision.reason = "notional above maximum order";
        return decision;
    }
    if (intent.units <= 0) {
        decision.code = AdmissionCode::UnitsTooSmall;
        decision.reason = "units below minimum";
        return decision;
    }
    if (session->snapshot.fee_bps > intent.requested_fee_bps ||
        !session->snapshot.accepts_fee(intent.requested_fee_bps)) {
        decision.code = AdmissionCode::FeeTooHigh;
        decision.reason = "fee envelope exceeded";
        return decision;
    }

    decision.projected_fee = session->snapshot.fee_for(intent.notional);
    decision.projected_reserve = session->snapshot.reserve_for(intent.notional);
    decision.projected_notional = checked_add(intent.notional, decision.projected_reserve);

    const Amount local = checked_add(session->committed_notional, session->committed_reserve);
    decision.remaining_before = session->snapshot.strictest_remaining(local);
    if (decision.projected_notional > decision.remaining_before) {
        decision.code = AdmissionCode::LimitExceeded;
        decision.reason = "exposure limit exceeded";
        decision.remaining_after = 0;
        return decision;
    }
    decision.remaining_after = checked_sub(decision.remaining_before, decision.projected_notional);

    if (ledger_.balance(intent.source_account, intent.asset_id) < intent.notional) {
        decision.code = AdmissionCode::LedgerRejected;
        decision.reason = "source balance below notional";
        return decision;
    }

    return decision;
}

ExecutionReceipt RiskEngine::execute(const OrderIntent& intent) {
    ++stats_.orders_submitted;
    AdmissionDecision decision = admit(intent);
    decisions_.push_back(decision);

    ExecutionReceipt receipt;
    receipt.id = "rcpt-" + amount_to_string(static_cast<Amount>(receipts_.size() + 1));
    receipt.order_id = intent.id;
    receipt.session_id = intent.session_id;
    receipt.operator_id = intent.operator_id;
    receipt.asset_id = intent.asset_id;
    receipt.channel_id = intent.channel_id;
    receipt.source_account = intent.source_account;
    receipt.beneficiary_account = intent.beneficiary_account;
    receipt.epoch_id = policy_.epoch().id;
    receipt.sequence = next_sequence();
    receipt.notional = intent.notional;
    receipt.fee = decision.projected_fee;
    receipt.reserve = decision.projected_reserve;
    receipt.net_amount = clamp_non_negative(intent.notional - receipt.fee - receipt.reserve);
    receipt.code = decision.code;
    receipt.status = decision.accepted() ? "pending" : "rejected";

    if (decision.accepted()) {
        apply_receipt(intent, decision, receipt);
    } else {
        ++stats_.orders_rejected;
    }

    receipt.digest_value = receipt.digest();
    receipts_.push_back(receipt);
    return receipt;
}

QuoteResult RiskEngine::quote(const PolicyKey& key, Amount notional, int fee_bps) const {
    QuoteResult result;
    result.key = key;
    result.requested_notional = notional;
    if (notional <= 0) {
        result.reason = "notional must be positive";
        return result;
    }
    const auto maybe_policy = policy_.maybe_policy(key);
    if (!maybe_policy) {
        result.reason = "policy not found";
        return result;
    }
    const QuotaState quota = policy_.compute_quota(key, exposure_.totals());
    result.fee = mul_bps(notional, quota.fee_bps);
    result.reserve = mul_bps(notional, quota.reserve_bps);
    result.net_amount = clamp_non_negative(notional - result.fee - result.reserve);
    result.remaining = quota.strictest_remaining();
    result.accepted = quota.enabled && fee_bps >= quota.fee_bps &&
                      checked_add(notional, result.reserve) <= result.remaining;
    result.reason = result.accepted ? "accepted" : "outside envelope";
    return result;
}

void RiskEngine::close_session(const std::string& session_id) {
    sessions_.close_session(session_id, next_sequence());
    stats_.sessions_closed = sessions_.closed();
}

void RiskEngine::rotate_epoch(
    const std::string& label,
    int risk_bias_bps,
    int liquidity_bias_bps,
    int settlement_bias_bps
) {
    policy_.rotate_epoch(label, risk_bias_bps, liquidity_bias_bps, settlement_bias_bps);
    next_sequence();
}

const std::vector<ExecutionReceipt>& RiskEngine::receipts() const {
    return receipts_;
}

const std::vector<AdmissionDecision>& RiskEngine::decisions() const {
    return decisions_;
}

std::string RiskEngine::state_digest() const {
    return digest_fields({
        policy_.digest(exposure_.totals()),
        ledger_.digest(),
        exposure_.digest(),
        sessions_.digest(),
        amount_to_string(sequence_),
    });
}

Sequence RiskEngine::next_sequence() {
    ++sequence_;
    return sequence_;
}

void RiskEngine::apply_receipt(
    const OrderIntent& intent,
    const AdmissionDecision& decision,
    ExecutionReceipt& receipt
) {
    const ChannelProfile& channel = policy_.channel(intent.channel_id);
    const Amount fee = decision.projected_fee;
    const Amount reserve = decision.projected_reserve;
    const Amount net = clamp_non_negative(intent.notional - fee - reserve);

    LedgerBook candidate_ledger = ledger_;
    bool ok = true;
    ok = ok && candidate_ledger.transfer(intent.source_account, intent.beneficiary_account, intent.asset_id, net, intent.id);
    ok = ok && candidate_ledger.transfer(intent.source_account, channel.fee_account(), intent.asset_id, fee, intent.id);
    ok = ok && candidate_ledger.transfer(intent.source_account, channel.reserve_account(), intent.asset_id, reserve, intent.id);
    if (!ok) {
        receipt.code = AdmissionCode::LedgerRejected;
        receipt.status = "rejected";
        ++stats_.orders_rejected;
        return;
    }

    ledger_ = std::move(candidate_ledger);

    exposure_.record(intent.key(), intent.notional, reserve);
    sessions_.note_execution(intent.session_id, intent.notional, reserve, receipt.sequence);
    receipt.status = "settled";
    ++stats_.orders_accepted;
    stats_.settled_notional = checked_add(stats_.settled_notional, intent.notional);
    stats_.collected_fees = checked_add(stats_.collected_fees, fee);
    stats_.reserved_notional = checked_add(stats_.reserved_notional, reserve);
}

}  // namespace pylon
