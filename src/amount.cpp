#include "pylon.hpp"

#include <cctype>
#include <climits>
#include <cstdlib>
#include <limits>

namespace pylon {

RuntimeError::RuntimeError(const std::string& message) : std::runtime_error(message) {}

std::string to_string(AssetKind value) {
    switch (value) {
        case AssetKind::Stablecoin:
            return "stablecoin";
        case AssetKind::Native:
            return "native";
        case AssetKind::TokenizedDeposit:
            return "tokenized_deposit";
        case AssetKind::TreasuryBill:
            return "treasury_bill";
        case AssetKind::SyntheticCredit:
            return "synthetic_credit";
    }
    return "stablecoin";
}

std::string to_string(ChannelClass value) {
    switch (value) {
        case ChannelClass::RealTime:
            return "real_time";
        case ChannelClass::Batch:
            return "batch";
        case ChannelClass::Netting:
            return "netting";
        case ChannelClass::Custody:
            return "custody";
        case ChannelClass::LayerTwo:
            return "layer_two";
    }
    return "real_time";
}

std::string to_string(OperatorTier value) {
    switch (value) {
        case OperatorTier::Retail:
            return "retail";
        case OperatorTier::Agency:
            return "agency";
        case OperatorTier::MarketMaker:
            return "market_maker";
        case OperatorTier::Prime:
            return "prime";
        case OperatorTier::Clearing:
            return "clearing";
        case OperatorTier::Internal:
            return "internal";
    }
    return "retail";
}

std::string to_string(SessionMode value) {
    switch (value) {
        case SessionMode::Balanced:
            return "balanced";
        case SessionMode::Immediate:
            return "immediate";
        case SessionMode::NetOnly:
            return "net_only";
        case SessionMode::Agency:
            return "agency";
    }
    return "balanced";
}

std::string to_string(OrderSide value) {
    switch (value) {
        case OrderSide::Debit:
            return "debit";
        case OrderSide::Credit:
            return "credit";
        case OrderSide::Rebalance:
            return "rebalance";
    }
    return "debit";
}

std::string to_string(AdmissionCode value) {
    switch (value) {
        case AdmissionCode::Accepted:
            return "accepted";
        case AdmissionCode::UnknownSession:
            return "unknown_session";
        case AdmissionCode::InactiveSession:
            return "inactive_session";
        case AdmissionCode::OperatorMismatch:
            return "operator_mismatch";
        case AdmissionCode::AssetMismatch:
            return "asset_mismatch";
        case AdmissionCode::ChannelMismatch:
            return "channel_mismatch";
        case AdmissionCode::AmountTooSmall:
            return "amount_too_small";
        case AdmissionCode::UnitsTooSmall:
            return "units_too_small";
        case AdmissionCode::FeeTooHigh:
            return "fee_too_high";
        case AdmissionCode::PolicyClosed:
            return "policy_closed";
        case AdmissionCode::LimitExceeded:
            return "limit_exceeded";
        case AdmissionCode::LedgerRejected:
            return "ledger_rejected";
    }
    return "accepted";
}

std::string to_string(MovementKind value) {
    switch (value) {
        case MovementKind::Deposit:
            return "deposit";
        case MovementKind::Withdrawal:
            return "withdrawal";
        case MovementKind::Transfer:
            return "transfer";
        case MovementKind::Fee:
            return "fee";
        case MovementKind::Reserve:
            return "reserve";
        case MovementKind::Release:
            return "release";
    }
    return "transfer";
}

std::string to_string(ReportVerbosity value) {
    switch (value) {
        case ReportVerbosity::Compact:
            return "compact";
        case ReportVerbosity::Standard:
            return "standard";
        case ReportVerbosity::Full:
            return "full";
    }
    return "standard";
}

AssetKind parse_asset_kind(const std::string& value) {
    const std::string key = lowercase(value);
    if (key == "stablecoin" || key == "stable") {
        return AssetKind::Stablecoin;
    }
    if (key == "native") {
        return AssetKind::Native;
    }
    if (key == "tokenized_deposit" || key == "deposit") {
        return AssetKind::TokenizedDeposit;
    }
    if (key == "treasury_bill" || key == "tbill") {
        return AssetKind::TreasuryBill;
    }
    if (key == "synthetic_credit" || key == "synthetic") {
        return AssetKind::SyntheticCredit;
    }
    throw RuntimeError("unknown asset kind: " + value);
}

ChannelClass parse_channel_class(const std::string& value) {
    const std::string key = lowercase(value);
    if (key == "real_time" || key == "realtime" || key == "rt") {
        return ChannelClass::RealTime;
    }
    if (key == "batch") {
        return ChannelClass::Batch;
    }
    if (key == "netting" || key == "net") {
        return ChannelClass::Netting;
    }
    if (key == "custody") {
        return ChannelClass::Custody;
    }
    if (key == "layer_two" || key == "l2") {
        return ChannelClass::LayerTwo;
    }
    throw RuntimeError("unknown channel class: " + value);
}

OperatorTier parse_operator_tier(const std::string& value) {
    const std::string key = lowercase(value);
    if (key == "retail") {
        return OperatorTier::Retail;
    }
    if (key == "agency") {
        return OperatorTier::Agency;
    }
    if (key == "market_maker" || key == "mm") {
        return OperatorTier::MarketMaker;
    }
    if (key == "prime") {
        return OperatorTier::Prime;
    }
    if (key == "clearing") {
        return OperatorTier::Clearing;
    }
    if (key == "internal") {
        return OperatorTier::Internal;
    }
    throw RuntimeError("unknown operator tier: " + value);
}

SessionMode parse_session_mode(const std::string& value) {
    const std::string key = lowercase(value);
    if (key == "balanced") {
        return SessionMode::Balanced;
    }
    if (key == "immediate") {
        return SessionMode::Immediate;
    }
    if (key == "net_only" || key == "net") {
        return SessionMode::NetOnly;
    }
    if (key == "agency") {
        return SessionMode::Agency;
    }
    throw RuntimeError("unknown session mode: " + value);
}

OrderSide parse_order_side(const std::string& value) {
    const std::string key = lowercase(value);
    if (key == "debit") {
        return OrderSide::Debit;
    }
    if (key == "credit") {
        return OrderSide::Credit;
    }
    if (key == "rebalance") {
        return OrderSide::Rebalance;
    }
    throw RuntimeError("unknown order side: " + value);
}

ReportVerbosity parse_report_verbosity(const std::string& value) {
    const std::string key = lowercase(value);
    if (key == "compact") {
        return ReportVerbosity::Compact;
    }
    if (key == "standard") {
        return ReportVerbosity::Standard;
    }
    if (key == "full") {
        return ReportVerbosity::Full;
    }
    throw RuntimeError("unknown report verbosity: " + value);
}

Amount checked_add(Amount left, Amount right) {
    if (right > 0 && left > std::numeric_limits<Amount>::max() - right) {
        throw RuntimeError("amount overflow");
    }
    if (right < 0 && left < std::numeric_limits<Amount>::min() - right) {
        throw RuntimeError("amount underflow");
    }
    return left + right;
}

Amount checked_sub(Amount left, Amount right) {
    if (right > 0 && left < std::numeric_limits<Amount>::min() + right) {
        throw RuntimeError("amount underflow");
    }
    if (right < 0 && left > std::numeric_limits<Amount>::max() + right) {
        throw RuntimeError("amount overflow");
    }
    return left - right;
}

Amount clamp_non_negative(Amount value) {
    return value < 0 ? 0 : value;
}

Amount mul_bps(Amount amount, int bps) {
    if (amount == 0 || bps == 0) {
        return 0;
    }
    const long double scaled = static_cast<long double>(amount) * static_cast<long double>(bps);
    const long double value = scaled / static_cast<long double>(kBpsScale);
    if (value > static_cast<long double>(std::numeric_limits<Amount>::max())) {
        throw RuntimeError("basis point multiplication overflow");
    }
    if (value < static_cast<long double>(std::numeric_limits<Amount>::min())) {
        throw RuntimeError("basis point multiplication underflow");
    }
    return static_cast<Amount>(value);
}

Amount div_round_up(Amount amount, Amount divisor) {
    if (divisor <= 0) {
        throw RuntimeError("invalid divisor");
    }
    if (amount <= 0) {
        return 0;
    }
    return 1 + ((amount - 1) / divisor);
}

Amount min_amount(Amount left, Amount right) {
    return std::min(left, right);
}

Amount max_amount(Amount left, Amount right) {
    return std::max(left, right);
}

Amount bounded_amount(Amount value, Amount low, Amount high) {
    if (low > high) {
        throw RuntimeError("invalid amount bounds");
    }
    return std::min(std::max(value, low), high);
}

Amount parse_amount(const std::string& value) {
    const std::string cleaned = trim(value);
    if (cleaned.empty()) {
        throw RuntimeError("empty amount");
    }
    char* end = nullptr;
    const long long parsed = std::strtoll(cleaned.c_str(), &end, 10);
    if (end == cleaned.c_str() || *end != '\0') {
        throw RuntimeError("invalid amount: " + value);
    }
    if (parsed < 0) {
        throw RuntimeError("negative amount: " + value);
    }
    return parsed;
}

int parse_int(const std::string& value) {
    const long long parsed = parse_i64(value);
    if (parsed < static_cast<long long>(std::numeric_limits<int>::min()) ||
        parsed > static_cast<long long>(std::numeric_limits<int>::max())) {
        throw RuntimeError("integer out of range: " + value);
    }
    return static_cast<int>(parsed);
}

long long parse_i64(const std::string& value) {
    const std::string cleaned = trim(value);
    if (cleaned.empty()) {
        throw RuntimeError("empty integer");
    }
    char* end = nullptr;
    const long long parsed = std::strtoll(cleaned.c_str(), &end, 10);
    if (end == cleaned.c_str() || *end != '\0') {
        throw RuntimeError("invalid integer: " + value);
    }
    return parsed;
}

std::string amount_to_string(Amount amount) {
    std::ostringstream out;
    out << amount;
    return out.str();
}

std::string fixed_decimals(Amount amount, int decimals) {
    if (decimals <= 0) {
        return amount_to_string(amount);
    }
    Amount divisor = 1;
    for (int i = 0; i < decimals; ++i) {
        divisor *= 10;
    }
    const Amount whole = amount / divisor;
    Amount fraction = amount % divisor;
    if (fraction < 0) {
        fraction = -fraction;
    }
    std::ostringstream out;
    out << whole << "." << std::setw(decimals) << std::setfill('0') << fraction;
    return out.str();
}

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string uppercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

std::string trim(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(start, end - start);
}

std::vector<std::string> split_ws(const std::string& line) {
    std::vector<std::string> tokens;
    std::istringstream in(line);
    std::string token;
    while (in >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> split_char(const std::string& line, char delimiter) {
    std::vector<std::string> result;
    std::string current;
    for (char ch : line) {
        if (ch == delimiter) {
            result.push_back(current);
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    result.push_back(current);
    return result;
}

bool starts_with(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

bool is_identifier(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    for (char ch : value) {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (!std::isalnum(uch) && ch != '-' && ch != '_' && ch != ':') {
            return false;
        }
    }
    return true;
}

std::string require_identifier(const std::string& value, const std::string& field) {
    if (!is_identifier(value)) {
        throw RuntimeError("invalid " + field + ": " + value);
    }
    return value;
}

std::string json_escape(const std::string& value) {
    std::ostringstream out;
    for (char ch : value) {
        switch (ch) {
            case '"':
                out << "\\\"";
                break;
            case '\\':
                out << "\\\\";
                break;
            case '\b':
                out << "\\b";
                break;
            case '\f':
                out << "\\f";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                if (static_cast<unsigned char>(ch) < 0x20) {
                    out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(ch));
                } else {
                    out << ch;
                }
                break;
        }
    }
    return out.str();
}

std::string hex_u64(std::uint64_t value) {
    std::ostringstream out;
    out << "0x" << std::hex << std::setw(16) << std::setfill('0') << value;
    return out.str();
}

std::uint64_t stable_hash(const std::string& value) {
    std::uint64_t hash = 1469598103934665603ull;
    for (unsigned char ch : value) {
        hash ^= static_cast<std::uint64_t>(ch);
        hash *= 1099511628211ull;
    }
    return hash;
}

std::uint64_t mix_hash(std::uint64_t seed, std::uint64_t value) {
    seed ^= value + 0x9e3779b97f4a7c15ull + (seed << 6u) + (seed >> 2u);
    return seed;
}

std::string digest_fields(const std::vector<std::string>& values) {
    std::uint64_t seed = 0xcbf29ce484222325ull;
    for (const std::string& value : values) {
        seed = mix_hash(seed, stable_hash(value));
    }
    return hex_u64(seed);
}

std::string join_strings(const std::vector<std::string>& values, const std::string& delimiter) {
    std::ostringstream out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            out << delimiter;
        }
        out << values[i];
    }
    return out.str();
}

Money::Money(std::string asset_id, Amount units) : asset(std::move(asset_id)), amount(units) {}

bool Money::is_positive() const {
    return amount > 0;
}

bool Money::is_zero() const {
    return amount == 0;
}

std::string Money::label() const {
    return amount_to_string(amount) + ":" + asset;
}

}  // namespace pylon
