#include "pylon.hpp"

namespace pylon {

void LedgerBook::create_account(const std::string& account_id) {
    require_identifier(account_id, "account id");
    balances_.try_emplace(account_id, std::map<std::string, Amount>{});
}

bool LedgerBook::has_account(const std::string& account_id) const {
    return balances_.find(account_id) != balances_.end();
}

void LedgerBook::deposit(
    const std::string& account_id,
    const std::string& asset_id,
    Amount amount,
    const std::string& ref
) {
    if (amount < 0) {
        throw RuntimeError("deposit amount must be positive");
    }
    create_account(account_id);
    balances_[account_id][asset_id] = checked_add(balances_[account_id][asset_id], amount);
    initial_totals_[asset_id] = checked_add(initial_totals_[asset_id], amount);
    append_movement(MovementKind::Deposit, asset_id, "mint", account_id, amount, ref);
}

bool LedgerBook::withdraw(
    const std::string& account_id,
    const std::string& asset_id,
    Amount amount,
    const std::string& ref
) {
    if (amount < 0) {
        return false;
    }
    const Amount current = balance(account_id, asset_id);
    if (current < amount) {
        return false;
    }
    balances_[account_id][asset_id] = checked_sub(current, amount);
    append_movement(MovementKind::Withdrawal, asset_id, account_id, "burn", amount, ref);
    return true;
}

bool LedgerBook::transfer(
    const std::string& from_account,
    const std::string& to_account,
    const std::string& asset_id,
    Amount amount,
    const std::string& ref
) {
    if (amount < 0) {
        return false;
    }
    create_account(from_account);
    create_account(to_account);
    if (balance(from_account, asset_id) < amount) {
        return false;
    }
    balances_[from_account][asset_id] = checked_sub(balances_[from_account][asset_id], amount);
    balances_[to_account][asset_id] = checked_add(balances_[to_account][asset_id], amount);
    append_movement(MovementKind::Transfer, asset_id, from_account, to_account, amount, ref);
    return true;
}

Amount LedgerBook::balance(const std::string& account_id, const std::string& asset_id) const {
    const auto account_it = balances_.find(account_id);
    if (account_it == balances_.end()) {
        return 0;
    }
    const auto asset_it = account_it->second.find(asset_id);
    if (asset_it == account_it->second.end()) {
        return 0;
    }
    return asset_it->second;
}

Amount LedgerBook::total_asset(const std::string& asset_id) const {
    Amount total = 0;
    for (const auto& account : balances_) {
        const auto asset_it = account.second.find(asset_id);
        if (asset_it != account.second.end()) {
            total = checked_add(total, asset_it->second);
        }
    }
    return total;
}

Amount LedgerBook::initial_total_asset(const std::string& asset_id) const {
    const auto it = initial_totals_.find(asset_id);
    return it == initial_totals_.end() ? 0 : it->second;
}

bool LedgerBook::conservation_ok() const {
    std::set<std::string> assets;
    for (const auto& pair : initial_totals_) {
        assets.insert(pair.first);
    }
    for (const auto& account : balances_) {
        for (const auto& balance_pair : account.second) {
            assets.insert(balance_pair.first);
        }
    }
    for (const std::string& asset_id : assets) {
        if (total_asset(asset_id) != initial_total_asset(asset_id)) {
            return false;
        }
    }
    return true;
}

std::vector<AccountSnapshot> LedgerBook::accounts() const {
    std::vector<AccountSnapshot> snapshots;
    snapshots.reserve(balances_.size());
    for (const auto& pair : balances_) {
        AccountSnapshot snapshot;
        snapshot.id = pair.first;
        snapshot.balances = pair.second;
        snapshots.push_back(snapshot);
    }
    return snapshots;
}

const std::vector<LedgerMovement>& LedgerBook::movements() const {
    return movements_;
}

std::map<std::string, Amount> LedgerBook::totals() const {
    std::set<std::string> assets;
    for (const auto& account : balances_) {
        for (const auto& asset_pair : account.second) {
            assets.insert(asset_pair.first);
        }
    }
    std::map<std::string, Amount> result;
    for (const std::string& asset_id : assets) {
        result[asset_id] = total_asset(asset_id);
    }
    return result;
}

std::string LedgerBook::digest() const {
    std::vector<std::string> fields;
    for (const auto& account : balances_) {
        fields.push_back(account.first);
        for (const auto& asset_pair : account.second) {
            fields.push_back(asset_pair.first);
            fields.push_back(amount_to_string(asset_pair.second));
        }
    }
    for (const LedgerMovement& movement : movements_) {
        fields.push_back(movement.digest());
    }
    return digest_fields(fields);
}

Sequence LedgerBook::next_sequence() {
    ++sequence_;
    return sequence_;
}

void LedgerBook::append_movement(
    MovementKind kind,
    const std::string& asset,
    const std::string& from,
    const std::string& to,
    Amount amount,
    const std::string& ref
) {
    LedgerMovement movement;
    movement.sequence = next_sequence();
    movement.kind = kind;
    movement.asset_id = asset;
    movement.from_account = from;
    movement.to_account = to;
    movement.amount = amount;
    movement.reference = ref;
    movements_.push_back(movement);
}

void ExposureBook::record(const PolicyKey& key, Amount notional, Amount reserve) {
    const Amount weighted = checked_add(notional, reserve);
    totals_.global = checked_add(totals_.global, weighted);
    totals_.by_operator[key.operator_id] = checked_add(totals_.by_operator[key.operator_id], weighted);
    totals_.by_asset[key.asset_id] = checked_add(totals_.by_asset[key.asset_id], weighted);
    totals_.by_channel[key.channel_id] = checked_add(totals_.by_channel[key.channel_id], weighted);
    totals_.by_policy[key.str()] = checked_add(totals_.by_policy[key.str()], weighted);
    totals_.debt_by_operator[key.operator_id] = checked_add(totals_.debt_by_operator[key.operator_id], notional);
}

void ExposureBook::release(const PolicyKey& key, Amount notional, Amount reserve) {
    const Amount weighted = checked_add(notional, reserve);
    totals_.global = clamp_non_negative(totals_.global - weighted);
    totals_.by_operator[key.operator_id] = clamp_non_negative(totals_.by_operator[key.operator_id] - weighted);
    totals_.by_asset[key.asset_id] = clamp_non_negative(totals_.by_asset[key.asset_id] - weighted);
    totals_.by_channel[key.channel_id] = clamp_non_negative(totals_.by_channel[key.channel_id] - weighted);
    totals_.by_policy[key.str()] = clamp_non_negative(totals_.by_policy[key.str()] - weighted);
    totals_.debt_by_operator[key.operator_id] = clamp_non_negative(totals_.debt_by_operator[key.operator_id] - notional);
}

void ExposureBook::clear() {
    totals_ = ExposureTotals{};
}

ExposureTotals ExposureBook::totals() const {
    return totals_;
}

Amount ExposureBook::global() const {
    return totals_.global;
}

Amount ExposureBook::by_operator(const std::string& operator_id) const {
    return totals_.operator_exposure(operator_id);
}

Amount ExposureBook::by_asset(const std::string& asset_id) const {
    return totals_.asset_exposure(asset_id);
}

Amount ExposureBook::by_channel(const std::string& channel_id) const {
    return totals_.channel_exposure(channel_id);
}

Amount ExposureBook::by_policy(const PolicyKey& key) const {
    return totals_.policy_exposure(key);
}

Amount ExposureBook::debt_by_operator(const std::string& operator_id) const {
    return totals_.operator_debt(operator_id);
}

std::string ExposureBook::digest() const {
    std::vector<std::string> fields;
    fields.push_back(amount_to_string(totals_.global));
    for (const auto& pair : totals_.by_operator) {
        fields.push_back("op:" + pair.first + ":" + amount_to_string(pair.second));
    }
    for (const auto& pair : totals_.by_asset) {
        fields.push_back("asset:" + pair.first + ":" + amount_to_string(pair.second));
    }
    for (const auto& pair : totals_.by_channel) {
        fields.push_back("channel:" + pair.first + ":" + amount_to_string(pair.second));
    }
    for (const auto& pair : totals_.by_policy) {
        fields.push_back("policy:" + pair.first + ":" + amount_to_string(pair.second));
    }
    for (const auto& pair : totals_.debt_by_operator) {
        fields.push_back("debt:" + pair.first + ":" + amount_to_string(pair.second));
    }
    return digest_fields(fields);
}

}  // namespace pylon
