#include "pylon.hpp"

namespace pylon {

void PolicyBook::register_asset(const AssetProfile& asset) {
    if (asset.id.empty()) {
        throw RuntimeError("asset id required");
    }
    assets_[asset.id] = asset;
}

void PolicyBook::register_operator(const OperatorProfile& op) {
    if (op.id.empty()) {
        throw RuntimeError("operator id required");
    }
    operators_[op.id] = op;
}

void PolicyBook::register_channel(const ChannelProfile& channel) {
    if (channel.id.empty()) {
        throw RuntimeError("channel id required");
    }
    channels_[channel.id] = channel;
}

void PolicyBook::set_policy(const LimitPolicy& policy) {
    if (policy.key.empty()) {
        throw RuntimeError("policy key required");
    }
    if (!has_operator(policy.key.operator_id)) {
        throw RuntimeError("policy references unknown operator: " + policy.key.operator_id);
    }
    if (!has_asset(policy.key.asset_id)) {
        throw RuntimeError("policy references unknown asset: " + policy.key.asset_id);
    }
    if (!has_channel(policy.key.channel_id)) {
        throw RuntimeError("policy references unknown channel: " + policy.key.channel_id);
    }
    policies_[policy.key] = policy;
}

bool PolicyBook::has_asset(const std::string& asset_id) const {
    return assets_.find(asset_id) != assets_.end();
}

bool PolicyBook::has_operator(const std::string& operator_id) const {
    return operators_.find(operator_id) != operators_.end();
}

bool PolicyBook::has_channel(const std::string& channel_id) const {
    return channels_.find(channel_id) != channels_.end();
}

bool PolicyBook::has_policy(const PolicyKey& key) const {
    return policies_.find(key) != policies_.end();
}

const AssetProfile& PolicyBook::asset(const std::string& asset_id) const {
    const auto it = assets_.find(asset_id);
    if (it == assets_.end()) {
        throw RuntimeError("unknown asset: " + asset_id);
    }
    return it->second;
}

const OperatorProfile& PolicyBook::operator_profile(const std::string& operator_id) const {
    const auto it = operators_.find(operator_id);
    if (it == operators_.end()) {
        throw RuntimeError("unknown operator: " + operator_id);
    }
    return it->second;
}

const ChannelProfile& PolicyBook::channel(const std::string& channel_id) const {
    const auto it = channels_.find(channel_id);
    if (it == channels_.end()) {
        throw RuntimeError("unknown channel: " + channel_id);
    }
    return it->second;
}

const LimitPolicy& PolicyBook::policy(const PolicyKey& key) const {
    const auto it = policies_.find(key);
    if (it == policies_.end()) {
        throw RuntimeError("unknown policy: " + key.str());
    }
    return it->second;
}

std::optional<LimitPolicy> PolicyBook::maybe_policy(const PolicyKey& key) const {
    const auto it = policies_.find(key);
    if (it == policies_.end()) {
        return std::nullopt;
    }
    return it->second;
}

EpochState PolicyBook::epoch() const {
    return epoch_;
}

void PolicyBook::rotate_epoch(
    const std::string& label,
    int risk_bias_bps,
    int liquidity_bias_bps,
    int settlement_bias_bps
) {
    ++epoch_.id;
    epoch_.label = label;
    epoch_.risk_bias_bps = static_cast<int>(bounded_amount(risk_bias_bps, 1000, 15000));
    epoch_.liquidity_bias_bps = static_cast<int>(bounded_amount(liquidity_bias_bps, 1000, 15000));
    epoch_.settlement_bias_bps = static_cast<int>(bounded_amount(settlement_bias_bps, 1000, 15000));
    ++epoch_.sequence;
}

void PolicyBook::set_epoch(
    EpochId id,
    const std::string& label,
    int risk_bias_bps,
    int liquidity_bias_bps,
    int settlement_bias_bps
) {
    if (id <= 0) {
        throw RuntimeError("epoch id must be positive");
    }
    epoch_.id = id;
    epoch_.label = label;
    epoch_.risk_bias_bps = static_cast<int>(bounded_amount(risk_bias_bps, 1000, 15000));
    epoch_.liquidity_bias_bps = static_cast<int>(bounded_amount(liquidity_bias_bps, 1000, 15000));
    epoch_.settlement_bias_bps = static_cast<int>(bounded_amount(settlement_bias_bps, 1000, 15000));
    ++epoch_.sequence;
}

QuotaState PolicyBook::compute_quota(const PolicyKey& key, const ExposureTotals& totals) const {
    const LimitPolicy& p = policy(key);
    const OperatorProfile& op = operator_profile(key.operator_id);
    const AssetProfile& a = asset(key.asset_id);
    const ChannelProfile& c = channel(key.channel_id);

    QuotaState quota;
    quota.key = key;
    quota.epoch_id = epoch_.id;
    quota.operator_limit = adjusted_limit(p.operator_limit, op, a, c, p);
    quota.asset_limit = adjusted_limit(p.asset_limit, op, a, c, p);
    quota.channel_limit = adjusted_limit(p.channel_limit, op, a, c, p);
    quota.global_limit = adjusted_limit(p.global_limit, op, a, c, p);
    quota.operator_observed = totals.operator_exposure(key.operator_id);
    quota.asset_observed = totals.asset_exposure(key.asset_id);
    quota.channel_observed = totals.channel_exposure(key.channel_id);
    quota.global_observed = totals.global;
    quota.fee_bps = p.fee_bps + c.settlement_fee_bps;
    quota.reserve_bps = std::max(p.reserve_bps, c.reserve_floor_bps);
    quota.max_fee_bps = p.max_fee_bps;
    quota.enabled = p.enabled && op.enabled && a.enabled && c.enabled;
    return quota;
}

LimitSnapshot PolicyBook::capture_snapshot(const PolicyKey& key, const ExposureTotals& totals) const {
    const LimitPolicy& p = policy(key);
    const QuotaState quota = compute_quota(key, totals);
    LimitSnapshot snapshot;
    snapshot.key = key;
    snapshot.epoch_id = quota.epoch_id;
    snapshot.captured_at = epoch_.sequence;
    snapshot.operator_limit = quota.operator_limit;
    snapshot.asset_limit = quota.asset_limit;
    snapshot.channel_limit = quota.channel_limit;
    snapshot.global_limit = quota.global_limit;
    snapshot.operator_observed = quota.operator_observed;
    snapshot.asset_observed = quota.asset_observed;
    snapshot.channel_observed = quota.channel_observed;
    snapshot.global_observed = quota.global_observed;
    snapshot.session_reserved = 0;
    snapshot.fee_bps = quota.fee_bps;
    snapshot.reserve_bps = quota.reserve_bps;
    snapshot.max_fee_bps = quota.max_fee_bps;
    snapshot.ttl_seconds = p.session_ttl_seconds;
    snapshot.max_orders = p.max_orders_per_session;
    snapshot.enabled = quota.enabled;
    return snapshot;
}

std::vector<QuotaState> PolicyBook::all_quotas(const ExposureTotals& totals) const {
    std::vector<QuotaState> quotas;
    quotas.reserve(policies_.size());
    for (const auto& pair : policies_) {
        quotas.push_back(compute_quota(pair.first, totals));
    }
    return quotas;
}

std::vector<AssetProfile> PolicyBook::assets() const {
    std::vector<AssetProfile> result;
    result.reserve(assets_.size());
    for (const auto& pair : assets_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<OperatorProfile> PolicyBook::operators() const {
    std::vector<OperatorProfile> result;
    result.reserve(operators_.size());
    for (const auto& pair : operators_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<ChannelProfile> PolicyBook::channels() const {
    std::vector<ChannelProfile> result;
    result.reserve(channels_.size());
    for (const auto& pair : channels_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<LimitPolicy> PolicyBook::policies() const {
    std::vector<LimitPolicy> result;
    result.reserve(policies_.size());
    for (const auto& pair : policies_) {
        result.push_back(pair.second);
    }
    return result;
}

std::string PolicyBook::digest(const ExposureTotals& totals) const {
    std::vector<std::string> fields;
    fields.push_back(epoch_.digest());
    for (const auto& pair : assets_) {
        fields.push_back(pair.second.digest());
    }
    for (const auto& pair : operators_) {
        fields.push_back(pair.second.digest());
    }
    for (const auto& pair : channels_) {
        fields.push_back(pair.second.digest());
    }
    for (const QuotaState& quota : all_quotas(totals)) {
        fields.push_back(quota.digest());
    }
    return digest_fields(fields);
}

Amount PolicyBook::adjusted_limit(
    Amount base,
    const OperatorProfile& op,
    const AssetProfile& asset,
    const ChannelProfile& channel,
    const LimitPolicy& policy
) const {
    if (base <= 0 || !policy.enabled || !op.enabled || !asset.enabled || !channel.enabled) {
        return 0;
    }
    Amount value = base;
    value = mul_bps(value, epoch_.risk_bias_bps);
    value = mul_bps(value, epoch_.liquidity_bias_bps);
    value = mul_bps(value, epoch_.settlement_bias_bps);
    value = mul_bps(value, op.session_weight_bps());
    value = mul_bps(value, op.trust_bps);
    value = mul_bps(value, asset.collateral_weight_bps);
    value = mul_bps(value, channel.class_weight_bps());
    value = mul_bps(value, channel.latency_weight_bps());
    const int volatility_drag =
        static_cast<int>(bounded_amount(kBpsScale - asset.volatility_bps, 5000, 12000));
    const int channel_drag =
        static_cast<int>(bounded_amount(kBpsScale - channel.channel_risk_bps, 5000, 12000));
    value = mul_bps(value, volatility_drag);
    value = mul_bps(value, channel_drag);
    const Amount floor = mul_bps(base, policy.stress_floor_bps);
    return std::max(value, floor);
}

}  // namespace pylon
