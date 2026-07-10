#include "pylon.hpp"

namespace pylon {

Amount AssetProfile::normalize_lot(Amount requested) const {
    if (requested <= 0) {
        return 0;
    }
    if (settlement_lot <= 1) {
        return requested;
    }
    return div_round_up(requested, settlement_lot) * settlement_lot;
}

Amount AssetProfile::risk_weighted(Amount amount) const {
    const Amount collateral = mul_bps(amount, collateral_weight_bps);
    const Amount volatility = mul_bps(amount, volatility_bps);
    return checked_add(collateral, volatility);
}

std::string AssetProfile::digest() const {
    return digest_fields({
        id,
        symbol,
        to_string(kind),
        amount_to_string(decimals),
        amount_to_string(volatility_bps),
        amount_to_string(collateral_weight_bps),
        amount_to_string(settlement_lot),
        enabled ? "1" : "0",
    });
}

int OperatorProfile::tier_weight_bps() const {
    switch (tier) {
        case OperatorTier::Retail:
            return 6500;
        case OperatorTier::Agency:
            return 7800;
        case OperatorTier::MarketMaker:
            return 9000;
        case OperatorTier::Prime:
            return 10400;
        case OperatorTier::Clearing:
            return 11200;
        case OperatorTier::Internal:
            return 12500;
    }
    return 7000;
}

int OperatorProfile::session_weight_bps() const {
    const int normalized = std::max(1, max_sessions);
    const int session_bonus = std::min(1200, normalized * 80);
    return static_cast<int>(bounded_amount(tier_weight_bps() + session_bonus, 6000, 13000));
}

std::string OperatorProfile::digest() const {
    return digest_fields({
        id,
        desk,
        to_string(tier),
        amount_to_string(trust_bps),
        amount_to_string(internal_buffer_bps),
        amount_to_string(max_sessions),
        enabled ? "1" : "0",
    });
}

int ChannelProfile::class_weight_bps() const {
    switch (channel_class) {
        case ChannelClass::RealTime:
            return 9300;
        case ChannelClass::Batch:
            return 8200;
        case ChannelClass::Netting:
            return 10800;
        case ChannelClass::Custody:
            return 8900;
        case ChannelClass::LayerTwo:
            return 9800;
    }
    return 9000;
}

int ChannelProfile::latency_weight_bps() const {
    if (latency_seconds <= 5) {
        return 10400;
    }
    if (latency_seconds <= 30) {
        return 10000;
    }
    if (latency_seconds <= 180) {
        return 9400;
    }
    if (latency_seconds <= 900) {
        return 8600;
    }
    return 7600;
}

std::string ChannelProfile::fee_account() const {
    return "fee-" + id;
}

std::string ChannelProfile::reserve_account() const {
    return "reserve-" + id;
}

std::string ChannelProfile::digest() const {
    return digest_fields({
        id,
        venue,
        to_string(channel_class),
        amount_to_string(latency_seconds),
        amount_to_string(settlement_fee_bps),
        amount_to_string(channel_risk_bps),
        amount_to_string(reserve_floor_bps),
        enabled ? "1" : "0",
        netting_supported ? "1" : "0",
    });
}

PolicyKey::PolicyKey(std::string op, std::string asset, std::string channel)
    : operator_id(std::move(op)), asset_id(std::move(asset)), channel_id(std::move(channel)) {}

std::string PolicyKey::str() const {
    return operator_id + ":" + asset_id + ":" + channel_id;
}

bool PolicyKey::empty() const {
    return operator_id.empty() || asset_id.empty() || channel_id.empty();
}

bool operator<(const PolicyKey& left, const PolicyKey& right) {
    if (left.operator_id != right.operator_id) {
        return left.operator_id < right.operator_id;
    }
    if (left.asset_id != right.asset_id) {
        return left.asset_id < right.asset_id;
    }
    return left.channel_id < right.channel_id;
}

bool operator==(const PolicyKey& left, const PolicyKey& right) {
    return left.operator_id == right.operator_id && left.asset_id == right.asset_id &&
           left.channel_id == right.channel_id;
}

PolicyKey parse_policy_key(const std::string& value) {
    const std::vector<std::string> parts = split_char(value, ':');
    if (parts.size() != 3) {
        throw RuntimeError("invalid policy key: " + value);
    }
    return PolicyKey(
        require_identifier(parts[0], "operator id"),
        require_identifier(parts[1], "asset id"),
        require_identifier(parts[2], "channel id")
    );
}

std::string make_policy_key(const std::string& op, const std::string& asset, const std::string& channel) {
    return PolicyKey(op, asset, channel).str();
}

Amount LimitPolicy::strictest_limit() const {
    Amount strict = global_limit;
    strict = std::min(strict, operator_limit);
    strict = std::min(strict, asset_limit);
    strict = std::min(strict, channel_limit);
    if (max_order_notional > 0) {
        strict = std::min(strict, max_order_notional);
    }
    return strict;
}

Amount LimitPolicy::fee_for(Amount notional) const {
    return mul_bps(notional, fee_bps);
}

Amount LimitPolicy::reserve_for(Amount notional) const {
    const int bps = std::max(reserve_bps, 0);
    return mul_bps(notional, bps);
}

bool LimitPolicy::accepts_fee(int requested_fee_bps) const {
    return requested_fee_bps >= 0 && requested_fee_bps <= max_fee_bps;
}

std::string LimitPolicy::digest() const {
    return digest_fields({
        key.str(),
        amount_to_string(operator_limit),
        amount_to_string(asset_limit),
        amount_to_string(channel_limit),
        amount_to_string(global_limit),
        amount_to_string(min_order_notional),
        amount_to_string(max_order_notional),
        amount_to_string(fee_bps),
        amount_to_string(max_fee_bps),
        amount_to_string(reserve_bps),
        amount_to_string(stress_floor_bps),
        amount_to_string(session_ttl_seconds),
        amount_to_string(max_orders_per_session),
        enabled ? "1" : "0",
        allow_netting ? "1" : "0",
    });
}

std::string EpochState::digest() const {
    return digest_fields({
        amount_to_string(id),
        label,
        amount_to_string(risk_bias_bps),
        amount_to_string(liquidity_bias_bps),
        amount_to_string(settlement_bias_bps),
        amount_to_string(sequence),
    });
}

Amount ExposureTotals::operator_exposure(const std::string& id) const {
    const auto it = by_operator.find(id);
    return it == by_operator.end() ? 0 : it->second;
}

Amount ExposureTotals::asset_exposure(const std::string& id) const {
    const auto it = by_asset.find(id);
    return it == by_asset.end() ? 0 : it->second;
}

Amount ExposureTotals::channel_exposure(const std::string& id) const {
    const auto it = by_channel.find(id);
    return it == by_channel.end() ? 0 : it->second;
}

Amount ExposureTotals::policy_exposure(const PolicyKey& key) const {
    const auto it = by_policy.find(key.str());
    return it == by_policy.end() ? 0 : it->second;
}

Amount ExposureTotals::operator_debt(const std::string& id) const {
    const auto it = debt_by_operator.find(id);
    return it == debt_by_operator.end() ? 0 : it->second;
}

Amount QuotaState::remaining_operator() const {
    return clamp_non_negative(operator_limit - operator_observed);
}

Amount QuotaState::remaining_asset() const {
    return clamp_non_negative(asset_limit - asset_observed);
}

Amount QuotaState::remaining_channel() const {
    return clamp_non_negative(channel_limit - channel_observed);
}

Amount QuotaState::remaining_global() const {
    return clamp_non_negative(global_limit - global_observed);
}

Amount QuotaState::remaining_policy() const {
    return std::min({remaining_operator(), remaining_asset(), remaining_channel(), remaining_global()});
}

Amount QuotaState::strictest_remaining() const {
    if (!enabled) {
        return 0;
    }
    return remaining_policy();
}

std::string QuotaState::digest() const {
    return digest_fields({
        key.str(),
        amount_to_string(epoch_id),
        amount_to_string(operator_limit),
        amount_to_string(asset_limit),
        amount_to_string(channel_limit),
        amount_to_string(global_limit),
        amount_to_string(operator_observed),
        amount_to_string(asset_observed),
        amount_to_string(channel_observed),
        amount_to_string(global_observed),
        amount_to_string(fee_bps),
        amount_to_string(reserve_bps),
        amount_to_string(max_fee_bps),
        enabled ? "1" : "0",
    });
}

Amount LimitSnapshot::remaining_operator(Amount local) const {
    return clamp_non_negative(operator_limit - operator_observed - local);
}

Amount LimitSnapshot::remaining_asset(Amount local) const {
    return clamp_non_negative(asset_limit - asset_observed - local);
}

Amount LimitSnapshot::remaining_channel(Amount local) const {
    return clamp_non_negative(channel_limit - channel_observed - local);
}

Amount LimitSnapshot::remaining_global(Amount local) const {
    return clamp_non_negative(global_limit - global_observed - local);
}

Amount LimitSnapshot::strictest_remaining(Amount local) const {
    if (!enabled) {
        return 0;
    }
    return std::min({
        remaining_operator(local),
        remaining_asset(local),
        remaining_channel(local),
        remaining_global(local),
    });
}

Amount LimitSnapshot::fee_for(Amount notional) const {
    return mul_bps(notional, fee_bps);
}

Amount LimitSnapshot::reserve_for(Amount notional) const {
    return mul_bps(notional, reserve_bps);
}

bool LimitSnapshot::accepts_fee(int requested_fee_bps) const {
    return requested_fee_bps >= 0 && requested_fee_bps <= max_fee_bps;
}

std::string LimitSnapshot::digest() const {
    return digest_fields({
        key.str(),
        amount_to_string(epoch_id),
        amount_to_string(captured_at),
        amount_to_string(operator_limit),
        amount_to_string(asset_limit),
        amount_to_string(channel_limit),
        amount_to_string(global_limit),
        amount_to_string(operator_observed),
        amount_to_string(asset_observed),
        amount_to_string(channel_observed),
        amount_to_string(global_observed),
        amount_to_string(session_reserved),
        amount_to_string(fee_bps),
        amount_to_string(reserve_bps),
        amount_to_string(max_fee_bps),
        amount_to_string(ttl_seconds),
        amount_to_string(max_orders),
        enabled ? "1" : "0",
    });
}

PolicyKey Session::key() const {
    return PolicyKey(operator_id, asset_id, channel_id);
}

Amount Session::projected_total(Amount notional) const {
    return checked_add(committed_notional, notional);
}

bool Session::has_order_capacity() const {
    return open && orders < snapshot.max_orders;
}

std::string Session::digest() const {
    return digest_fields({
        id,
        operator_id,
        asset_id,
        channel_id,
        funding_account,
        desk,
        to_string(mode),
        snapshot.digest(),
        amount_to_string(opened_sequence),
        amount_to_string(last_sequence),
        amount_to_string(committed_notional),
        amount_to_string(committed_reserve),
        amount_to_string(orders),
        open ? "1" : "0",
    });
}

PolicyKey OrderIntent::key() const {
    return PolicyKey(operator_id, asset_id, channel_id);
}

Amount OrderIntent::requested_fee() const {
    return mul_bps(notional, requested_fee_bps);
}

std::string OrderIntent::digest() const {
    return digest_fields({
        id,
        session_id,
        operator_id,
        asset_id,
        channel_id,
        source_account,
        beneficiary_account,
        to_string(side),
        amount_to_string(notional),
        amount_to_string(units),
        amount_to_string(requested_fee_bps),
        amount_to_string(priority),
        settlement_ref,
    });
}

bool AdmissionDecision::accepted() const {
    return code == AdmissionCode::Accepted;
}

std::string AdmissionDecision::digest() const {
    return digest_fields({
        to_string(code),
        reason,
        amount_to_string(projected_notional),
        amount_to_string(projected_fee),
        amount_to_string(projected_reserve),
        amount_to_string(remaining_before),
        amount_to_string(remaining_after),
    });
}

bool ExecutionReceipt::settled() const {
    return code == AdmissionCode::Accepted && status == "settled";
}

std::string ExecutionReceipt::digest() const {
    if (!digest_value.empty()) {
        return digest_value;
    }
    return digest_fields({
        id,
        order_id,
        session_id,
        operator_id,
        asset_id,
        channel_id,
        source_account,
        beneficiary_account,
        amount_to_string(epoch_id),
        amount_to_string(sequence),
        amount_to_string(notional),
        amount_to_string(fee),
        amount_to_string(reserve),
        amount_to_string(net_amount),
        to_string(code),
        status,
    });
}

std::string LedgerMovement::digest() const {
    return digest_fields({
        amount_to_string(sequence),
        to_string(kind),
        asset_id,
        from_account,
        to_account,
        amount_to_string(amount),
        reference,
    });
}

Amount AccountSnapshot::balance(const std::string& asset_id) const {
    const auto it = balances.find(asset_id);
    return it == balances.end() ? 0 : it->second;
}

Amount AccountSnapshot::total() const {
    Amount value = 0;
    for (const auto& pair : balances) {
        value = checked_add(value, pair.second);
    }
    return value;
}

std::string QuoteResult::digest() const {
    return digest_fields({
        key.str(),
        accepted ? "1" : "0",
        amount_to_string(requested_notional),
        amount_to_string(fee),
        amount_to_string(reserve),
        amount_to_string(net_amount),
        amount_to_string(remaining),
        reason,
    });
}

}  // namespace pylon
