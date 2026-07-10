#include "pylon.hpp"

#include <limits>

namespace pylon {
namespace {

Amount limit_for_operator(const RiskEngine& engine, const std::string& operator_id) {
    Amount value = 0;
    for (const QuotaState& quota : engine.policies().all_quotas(engine.exposure().totals())) {
        if (quota.key.operator_id == operator_id) {
            value = std::max(value, quota.operator_limit);
        }
    }
    return value;
}

Amount limit_for_asset(const RiskEngine& engine, const std::string& asset_id) {
    Amount value = 0;
    for (const QuotaState& quota : engine.policies().all_quotas(engine.exposure().totals())) {
        if (quota.key.asset_id == asset_id) {
            value = std::max(value, quota.asset_limit);
        }
    }
    return value;
}

Amount limit_for_channel(const RiskEngine& engine, const std::string& channel_id) {
    Amount value = 0;
    for (const QuotaState& quota : engine.policies().all_quotas(engine.exposure().totals())) {
        if (quota.key.channel_id == channel_id) {
            value = std::max(value, quota.channel_limit);
        }
    }
    return value;
}

}  // namespace

std::vector<PortfolioMetric> Analytics::operator_metrics(const RiskEngine& engine) const {
    std::vector<PortfolioMetric> metrics;
    const ExposureTotals totals = engine.exposure().totals();
    for (const OperatorProfile& op : engine.policies().operators()) {
        PortfolioMetric metric;
        metric.id = op.id;
        metric.value = totals.operator_exposure(op.id);
        metric.limit = limit_for_operator(engine, op.id);
        metric.utilization_bps = utilization_bps(metric.value, metric.limit);
        metric.status = status_for_bps(metric.utilization_bps);
        metrics.push_back(metric);
    }
    return metrics;
}

std::vector<PortfolioMetric> Analytics::asset_metrics(const RiskEngine& engine) const {
    std::vector<PortfolioMetric> metrics;
    const ExposureTotals totals = engine.exposure().totals();
    for (const AssetProfile& asset : engine.policies().assets()) {
        PortfolioMetric metric;
        metric.id = asset.id;
        metric.value = totals.asset_exposure(asset.id);
        metric.limit = limit_for_asset(engine, asset.id);
        metric.utilization_bps = utilization_bps(metric.value, metric.limit);
        metric.status = status_for_bps(metric.utilization_bps);
        metrics.push_back(metric);
    }
    return metrics;
}

std::vector<PortfolioMetric> Analytics::channel_metrics(const RiskEngine& engine) const {
    std::vector<PortfolioMetric> metrics;
    const ExposureTotals totals = engine.exposure().totals();
    for (const ChannelProfile& channel : engine.policies().channels()) {
        PortfolioMetric metric;
        metric.id = channel.id;
        metric.value = totals.channel_exposure(channel.id);
        metric.limit = limit_for_channel(engine, channel.id);
        metric.utilization_bps = utilization_bps(metric.value, metric.limit);
        metric.status = status_for_bps(metric.utilization_bps);
        metrics.push_back(metric);
    }
    return metrics;
}

std::vector<PortfolioMetric> Analytics::policy_metrics(const RiskEngine& engine) const {
    std::vector<PortfolioMetric> metrics;
    const ExposureTotals totals = engine.exposure().totals();
    for (const QuotaState& quota : engine.policies().all_quotas(totals)) {
        PortfolioMetric metric;
        metric.id = quota.key.str();
        metric.value = totals.policy_exposure(quota.key);
        metric.limit = std::min({quota.operator_limit, quota.asset_limit, quota.channel_limit, quota.global_limit});
        metric.utilization_bps = utilization_bps(metric.value, metric.limit);
        metric.status = status_for_bps(metric.utilization_bps);
        metrics.push_back(metric);
    }
    return metrics;
}

Amount Analytics::total_liquidity(const RiskEngine& engine, const std::string& asset_id) const {
    Amount total = 0;
    for (const AccountSnapshot& account : engine.ledger().accounts()) {
        total = checked_add(total, account.balance(asset_id));
    }
    return total;
}

int Analytics::utilization_bps(Amount value, Amount limit) const {
    if (limit <= 0) {
        return value > 0 ? kBpsScale : 0;
    }
    const long double scaled = static_cast<long double>(value) * static_cast<long double>(kBpsScale);
    const long double ratio = scaled / static_cast<long double>(limit);
    if (ratio > static_cast<long double>(std::numeric_limits<int>::max())) {
        return std::numeric_limits<int>::max();
    }
    return static_cast<int>(ratio);
}

std::string Analytics::status_for_bps(int value) const {
    if (value >= 9500) {
        return "saturated";
    }
    if (value >= 8000) {
        return "elevated";
    }
    if (value >= 5000) {
        return "active";
    }
    if (value > 0) {
        return "low";
    }
    return "idle";
}

}  // namespace pylon
