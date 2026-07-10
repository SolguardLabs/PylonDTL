#ifndef PYLON_DTL_PYLON_HPP
#define PYLON_DTL_PYLON_HPP

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iosfwd>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace pylon {

using Amount = long long;
using EpochId = int;
using Sequence = long long;

constexpr Amount kZeroAmount = 0;
constexpr int kBpsScale = 10000;

enum class AssetKind {
    Stablecoin,
    Native,
    TokenizedDeposit,
    TreasuryBill,
    SyntheticCredit,
};

enum class ChannelClass {
    RealTime,
    Batch,
    Netting,
    Custody,
    LayerTwo,
};

enum class OperatorTier {
    Retail,
    Agency,
    MarketMaker,
    Prime,
    Clearing,
    Internal,
};

enum class SessionMode {
    Balanced,
    Immediate,
    NetOnly,
    Agency,
};

enum class OrderSide {
    Debit,
    Credit,
    Rebalance,
};

enum class AdmissionCode {
    Accepted,
    UnknownSession,
    InactiveSession,
    OperatorMismatch,
    AssetMismatch,
    ChannelMismatch,
    AmountTooSmall,
    UnitsTooSmall,
    FeeTooHigh,
    PolicyClosed,
    LimitExceeded,
    LedgerRejected,
};

enum class MovementKind {
    Deposit,
    Withdrawal,
    Transfer,
    Fee,
    Reserve,
    Release,
};

enum class ReportVerbosity {
    Compact,
    Standard,
    Full,
};

struct RuntimeError : public std::runtime_error {
    explicit RuntimeError(const std::string& message);
};

std::string to_string(AssetKind value);
std::string to_string(ChannelClass value);
std::string to_string(OperatorTier value);
std::string to_string(SessionMode value);
std::string to_string(OrderSide value);
std::string to_string(AdmissionCode value);
std::string to_string(MovementKind value);
std::string to_string(ReportVerbosity value);

AssetKind parse_asset_kind(const std::string& value);
ChannelClass parse_channel_class(const std::string& value);
OperatorTier parse_operator_tier(const std::string& value);
SessionMode parse_session_mode(const std::string& value);
OrderSide parse_order_side(const std::string& value);
ReportVerbosity parse_report_verbosity(const std::string& value);

Amount checked_add(Amount left, Amount right);
Amount checked_sub(Amount left, Amount right);
Amount clamp_non_negative(Amount value);
Amount mul_bps(Amount amount, int bps);
Amount div_round_up(Amount amount, Amount divisor);
Amount min_amount(Amount left, Amount right);
Amount max_amount(Amount left, Amount right);
Amount bounded_amount(Amount value, Amount low, Amount high);
Amount parse_amount(const std::string& value);
int parse_int(const std::string& value);
long long parse_i64(const std::string& value);
std::string amount_to_string(Amount amount);
std::string fixed_decimals(Amount amount, int decimals);
std::string lowercase(std::string value);
std::string uppercase(std::string value);
std::string trim(const std::string& value);
std::vector<std::string> split_ws(const std::string& line);
std::vector<std::string> split_char(const std::string& line, char delimiter);
bool starts_with(const std::string& value, const std::string& prefix);
bool is_identifier(const std::string& value);
std::string require_identifier(const std::string& value, const std::string& field);
std::string json_escape(const std::string& value);
std::string hex_u64(std::uint64_t value);
std::uint64_t stable_hash(const std::string& value);
std::uint64_t mix_hash(std::uint64_t seed, std::uint64_t value);
std::string digest_fields(const std::vector<std::string>& values);
std::string join_strings(const std::vector<std::string>& values, const std::string& delimiter);

struct Money {
    std::string asset;
    Amount amount = 0;

    Money() = default;
    Money(std::string asset_id, Amount units);

    bool is_positive() const;
    bool is_zero() const;
    std::string label() const;
};

struct AssetProfile {
    std::string id;
    std::string symbol;
    AssetKind kind = AssetKind::Stablecoin;
    int decimals = 6;
    int volatility_bps = 100;
    int collateral_weight_bps = kBpsScale;
    Amount settlement_lot = 1;
    bool enabled = true;

    Amount normalize_lot(Amount requested) const;
    Amount risk_weighted(Amount amount) const;
    std::string digest() const;
};

struct OperatorProfile {
    std::string id;
    std::string desk;
    OperatorTier tier = OperatorTier::Retail;
    int trust_bps = 7000;
    int internal_buffer_bps = 500;
    int max_sessions = 4;
    bool enabled = true;

    int tier_weight_bps() const;
    int session_weight_bps() const;
    std::string digest() const;
};

struct ChannelProfile {
    std::string id;
    std::string venue;
    ChannelClass channel_class = ChannelClass::RealTime;
    int latency_seconds = 30;
    int settlement_fee_bps = 2;
    int channel_risk_bps = 100;
    int reserve_floor_bps = 250;
    bool enabled = true;
    bool netting_supported = true;

    int class_weight_bps() const;
    int latency_weight_bps() const;
    std::string fee_account() const;
    std::string reserve_account() const;
    std::string digest() const;
};

struct PolicyKey {
    std::string operator_id;
    std::string asset_id;
    std::string channel_id;

    PolicyKey() = default;
    PolicyKey(std::string op, std::string asset, std::string channel);

    std::string str() const;
    bool empty() const;
};

bool operator<(const PolicyKey& left, const PolicyKey& right);
bool operator==(const PolicyKey& left, const PolicyKey& right);
PolicyKey parse_policy_key(const std::string& value);
std::string make_policy_key(const std::string& op, const std::string& asset, const std::string& channel);

struct LimitPolicy {
    PolicyKey key;
    Amount operator_limit = 0;
    Amount asset_limit = 0;
    Amount channel_limit = 0;
    Amount global_limit = 0;
    Amount min_order_notional = 1;
    Amount max_order_notional = 0;
    int fee_bps = 1;
    int max_fee_bps = 10;
    int reserve_bps = 100;
    int stress_floor_bps = 7500;
    int session_ttl_seconds = 600;
    int max_orders_per_session = 16;
    bool enabled = true;
    bool allow_netting = true;

    Amount strictest_limit() const;
    Amount fee_for(Amount notional) const;
    Amount reserve_for(Amount notional) const;
    bool accepts_fee(int requested_fee_bps) const;
    std::string digest() const;
};

struct EpochState {
    EpochId id = 1;
    std::string label = "steady";
    int risk_bias_bps = kBpsScale;
    int liquidity_bias_bps = kBpsScale;
    int settlement_bias_bps = kBpsScale;
    Sequence sequence = 0;

    std::string digest() const;
};

struct ExposureTotals {
    Amount global = 0;
    std::map<std::string, Amount> by_operator;
    std::map<std::string, Amount> by_asset;
    std::map<std::string, Amount> by_channel;
    std::map<std::string, Amount> by_policy;
    std::map<std::string, Amount> debt_by_operator;

    Amount operator_exposure(const std::string& id) const;
    Amount asset_exposure(const std::string& id) const;
    Amount channel_exposure(const std::string& id) const;
    Amount policy_exposure(const PolicyKey& key) const;
    Amount operator_debt(const std::string& id) const;
};

struct QuotaState {
    PolicyKey key;
    EpochId epoch_id = 0;
    Amount operator_limit = 0;
    Amount asset_limit = 0;
    Amount channel_limit = 0;
    Amount global_limit = 0;
    Amount operator_observed = 0;
    Amount asset_observed = 0;
    Amount channel_observed = 0;
    Amount global_observed = 0;
    int fee_bps = 0;
    int reserve_bps = 0;
    int max_fee_bps = 0;
    bool enabled = false;

    Amount remaining_operator() const;
    Amount remaining_asset() const;
    Amount remaining_channel() const;
    Amount remaining_global() const;
    Amount remaining_policy() const;
    Amount strictest_remaining() const;
    std::string digest() const;
};

struct LimitSnapshot {
    PolicyKey key;
    EpochId epoch_id = 0;
    Sequence captured_at = 0;
    Amount operator_limit = 0;
    Amount asset_limit = 0;
    Amount channel_limit = 0;
    Amount global_limit = 0;
    Amount operator_observed = 0;
    Amount asset_observed = 0;
    Amount channel_observed = 0;
    Amount global_observed = 0;
    Amount session_reserved = 0;
    int fee_bps = 0;
    int reserve_bps = 0;
    int max_fee_bps = 0;
    int ttl_seconds = 0;
    int max_orders = 0;
    bool enabled = false;

    Amount remaining_operator(Amount local) const;
    Amount remaining_asset(Amount local) const;
    Amount remaining_channel(Amount local) const;
    Amount remaining_global(Amount local) const;
    Amount strictest_remaining(Amount local) const;
    Amount fee_for(Amount notional) const;
    Amount reserve_for(Amount notional) const;
    bool accepts_fee(int requested_fee_bps) const;
    std::string digest() const;
};

struct Session {
    std::string id;
    std::string operator_id;
    std::string asset_id;
    std::string channel_id;
    std::string funding_account;
    std::string desk;
    SessionMode mode = SessionMode::Balanced;
    LimitSnapshot snapshot;
    Sequence opened_sequence = 0;
    Sequence last_sequence = 0;
    Amount committed_notional = 0;
    Amount committed_reserve = 0;
    int orders = 0;
    bool open = true;

    PolicyKey key() const;
    Amount projected_total(Amount notional) const;
    bool has_order_capacity() const;
    std::string digest() const;
};

struct OrderIntent {
    std::string id;
    std::string session_id;
    std::string operator_id;
    std::string asset_id;
    std::string channel_id;
    std::string source_account;
    std::string beneficiary_account;
    OrderSide side = OrderSide::Debit;
    Amount notional = 0;
    Amount units = 0;
    int requested_fee_bps = 0;
    int priority = 0;
    std::string settlement_ref;

    PolicyKey key() const;
    Amount requested_fee() const;
    std::string digest() const;
};

struct AdmissionDecision {
    AdmissionCode code = AdmissionCode::Accepted;
    std::string reason;
    Amount projected_notional = 0;
    Amount projected_fee = 0;
    Amount projected_reserve = 0;
    Amount remaining_before = 0;
    Amount remaining_after = 0;

    bool accepted() const;
    std::string digest() const;
};

struct ExecutionReceipt {
    std::string id;
    std::string order_id;
    std::string session_id;
    std::string operator_id;
    std::string asset_id;
    std::string channel_id;
    std::string source_account;
    std::string beneficiary_account;
    EpochId epoch_id = 0;
    Sequence sequence = 0;
    Amount notional = 0;
    Amount fee = 0;
    Amount reserve = 0;
    Amount net_amount = 0;
    AdmissionCode code = AdmissionCode::Accepted;
    std::string status;
    std::string digest_value;

    bool settled() const;
    std::string digest() const;
};

struct LedgerMovement {
    Sequence sequence = 0;
    MovementKind kind = MovementKind::Transfer;
    std::string asset_id;
    std::string from_account;
    std::string to_account;
    Amount amount = 0;
    std::string reference;

    std::string digest() const;
};

struct AccountSnapshot {
    std::string id;
    std::map<std::string, Amount> balances;

    Amount balance(const std::string& asset_id) const;
    Amount total() const;
};

class LedgerBook {
public:
    void create_account(const std::string& account_id);
    bool has_account(const std::string& account_id) const;
    void deposit(const std::string& account_id, const std::string& asset_id, Amount amount, const std::string& ref);
    bool withdraw(const std::string& account_id, const std::string& asset_id, Amount amount, const std::string& ref);
    bool transfer(
        const std::string& from_account,
        const std::string& to_account,
        const std::string& asset_id,
        Amount amount,
        const std::string& ref
    );
    Amount balance(const std::string& account_id, const std::string& asset_id) const;
    Amount total_asset(const std::string& asset_id) const;
    Amount initial_total_asset(const std::string& asset_id) const;
    bool conservation_ok() const;
    std::vector<AccountSnapshot> accounts() const;
    const std::vector<LedgerMovement>& movements() const;
    std::map<std::string, Amount> totals() const;
    std::string digest() const;

private:
    Sequence next_sequence();
    void append_movement(MovementKind kind, const std::string& asset, const std::string& from, const std::string& to, Amount amount, const std::string& ref);
    std::map<std::string, std::map<std::string, Amount>> balances_;
    std::map<std::string, Amount> initial_totals_;
    std::vector<LedgerMovement> movements_;
    Sequence sequence_ = 0;
};

class ExposureBook {
public:
    void record(const PolicyKey& key, Amount notional, Amount reserve);
    void release(const PolicyKey& key, Amount notional, Amount reserve);
    void clear();
    ExposureTotals totals() const;
    Amount global() const;
    Amount by_operator(const std::string& operator_id) const;
    Amount by_asset(const std::string& asset_id) const;
    Amount by_channel(const std::string& channel_id) const;
    Amount by_policy(const PolicyKey& key) const;
    Amount debt_by_operator(const std::string& operator_id) const;
    std::string digest() const;

private:
    ExposureTotals totals_;
};

class PolicyBook {
public:
    void register_asset(const AssetProfile& asset);
    void register_operator(const OperatorProfile& op);
    void register_channel(const ChannelProfile& channel);
    void set_policy(const LimitPolicy& policy);
    bool has_asset(const std::string& asset_id) const;
    bool has_operator(const std::string& operator_id) const;
    bool has_channel(const std::string& channel_id) const;
    bool has_policy(const PolicyKey& key) const;
    const AssetProfile& asset(const std::string& asset_id) const;
    const OperatorProfile& operator_profile(const std::string& operator_id) const;
    const ChannelProfile& channel(const std::string& channel_id) const;
    const LimitPolicy& policy(const PolicyKey& key) const;
    std::optional<LimitPolicy> maybe_policy(const PolicyKey& key) const;
    EpochState epoch() const;
    void rotate_epoch(const std::string& label, int risk_bias_bps, int liquidity_bias_bps, int settlement_bias_bps);
    void set_epoch(EpochId id, const std::string& label, int risk_bias_bps, int liquidity_bias_bps, int settlement_bias_bps);
    QuotaState compute_quota(const PolicyKey& key, const ExposureTotals& totals) const;
    LimitSnapshot capture_snapshot(const PolicyKey& key, const ExposureTotals& totals) const;
    std::vector<QuotaState> all_quotas(const ExposureTotals& totals) const;
    std::vector<AssetProfile> assets() const;
    std::vector<OperatorProfile> operators() const;
    std::vector<ChannelProfile> channels() const;
    std::vector<LimitPolicy> policies() const;
    std::string digest(const ExposureTotals& totals) const;

private:
    Amount adjusted_limit(
        Amount base,
        const OperatorProfile& op,
        const AssetProfile& asset,
        const ChannelProfile& channel,
        const LimitPolicy& policy
    ) const;
    std::map<std::string, AssetProfile> assets_;
    std::map<std::string, OperatorProfile> operators_;
    std::map<std::string, ChannelProfile> channels_;
    std::map<PolicyKey, LimitPolicy> policies_;
    EpochState epoch_;
};

class SessionManager {
public:
    Session open_session(
        const std::string& id,
        const std::string& operator_id,
        const std::string& asset_id,
        const std::string& channel_id,
        const std::string& funding_account,
        SessionMode mode,
        const LimitSnapshot& snapshot,
        Sequence sequence
    );
    bool has_session(const std::string& session_id) const;
    Session* find_session(const std::string& session_id);
    const Session* find_session(const std::string& session_id) const;
    std::vector<Session> sessions() const;
    int open_count_for_operator(const std::string& operator_id) const;
    void close_session(const std::string& session_id, Sequence sequence);
    void note_execution(const std::string& session_id, Amount notional, Amount reserve, Sequence sequence);
    int opened() const;
    int closed() const;
    std::string digest() const;

private:
    std::map<std::string, Session> sessions_;
    int opened_ = 0;
    int closed_ = 0;
};

struct QuoteResult {
    PolicyKey key;
    bool accepted = false;
    Amount requested_notional = 0;
    Amount fee = 0;
    Amount reserve = 0;
    Amount net_amount = 0;
    Amount remaining = 0;
    std::string reason;
    std::string digest() const;
};

struct EngineStats {
    int sessions_opened = 0;
    int sessions_closed = 0;
    int orders_submitted = 0;
    int orders_accepted = 0;
    int orders_rejected = 0;
    Amount settled_notional = 0;
    Amount collected_fees = 0;
    Amount reserved_notional = 0;
};

class RiskEngine {
public:
    RiskEngine();
    void bootstrap_default();
    const PolicyBook& policies() const;
    PolicyBook& policies();
    const LedgerBook& ledger() const;
    LedgerBook& ledger();
    const ExposureBook& exposure() const;
    ExposureBook& exposure();
    const SessionManager& sessions() const;
    SessionManager& sessions();
    EngineStats stats() const;
    Sequence sequence() const;
    Session open_session(
        const std::string& id,
        const std::string& operator_id,
        const std::string& asset_id,
        const std::string& channel_id,
        const std::string& funding_account,
        SessionMode mode
    );
    AdmissionDecision admit(const OrderIntent& intent) const;
    ExecutionReceipt execute(const OrderIntent& intent);
    QuoteResult quote(const PolicyKey& key, Amount notional, int fee_bps) const;
    void close_session(const std::string& session_id);
    void rotate_epoch(const std::string& label, int risk_bias_bps, int liquidity_bias_bps, int settlement_bias_bps);
    const std::vector<ExecutionReceipt>& receipts() const;
    const std::vector<AdmissionDecision>& decisions() const;
    std::string state_digest() const;

private:
    Sequence next_sequence();
    void apply_receipt(const OrderIntent& intent, const AdmissionDecision& decision, ExecutionReceipt& receipt);
    PolicyBook policy_;
    LedgerBook ledger_;
    ExposureBook exposure_;
    SessionManager sessions_;
    std::vector<ExecutionReceipt> receipts_;
    std::vector<AdmissionDecision> decisions_;
    EngineStats stats_;
    Sequence sequence_ = 0;
};

struct ScenarioReport {
    std::string name;
    RiskEngine engine;
    std::vector<QuoteResult> quotes;
    std::vector<std::string> notes;
};

class ScenarioRunner {
public:
    ScenarioReport run(const std::string& name) const;
    ScenarioReport snapshot() const;
    ScenarioReport single() const;
    ScenarioReport epoch() const;
    ScenarioReport portfolio() const;
    ScenarioReport channels() const;
    ScenarioReport script(const std::string& path) const;
};

class ScriptRunner {
public:
    ScenarioReport run_file(const std::string& path) const;
    ScenarioReport run_lines(const std::vector<std::string>& lines) const;

private:
    void execute_line(RiskEngine& engine, ScenarioReport& report, const std::string& line, int line_no) const;
    OrderIntent build_order(const std::vector<std::string>& tokens, int line_no) const;
};

class ReportBuilder {
public:
    std::string build(const ScenarioReport& report, ReportVerbosity verbosity = ReportVerbosity::Standard) const;

private:
    void write_surface(std::ostream& out, const RiskEngine& engine) const;
    void write_epoch(std::ostream& out, const RiskEngine& engine) const;
    void write_stats(std::ostream& out, const RiskEngine& engine) const;
    void write_exposure(std::ostream& out, const RiskEngine& engine) const;
    void write_ledger(std::ostream& out, const RiskEngine& engine, ReportVerbosity verbosity) const;
    void write_sessions(std::ostream& out, const RiskEngine& engine, ReportVerbosity verbosity) const;
    void write_receipts(std::ostream& out, const RiskEngine& engine, ReportVerbosity verbosity) const;
    void write_quotes(std::ostream& out, const std::vector<QuoteResult>& quotes) const;
    void write_notes(std::ostream& out, const std::vector<std::string>& notes) const;
};

struct PortfolioMetric {
    std::string id;
    Amount value = 0;
    Amount limit = 0;
    int utilization_bps = 0;
    std::string status;
};

class Analytics {
public:
    std::vector<PortfolioMetric> operator_metrics(const RiskEngine& engine) const;
    std::vector<PortfolioMetric> asset_metrics(const RiskEngine& engine) const;
    std::vector<PortfolioMetric> channel_metrics(const RiskEngine& engine) const;
    std::vector<PortfolioMetric> policy_metrics(const RiskEngine& engine) const;
    Amount total_liquidity(const RiskEngine& engine, const std::string& asset_id) const;
    int utilization_bps(Amount value, Amount limit) const;
    std::string status_for_bps(int value) const;
};

struct ReconciliationFinding {
    std::string code;
    std::string subject;
    std::string detail;
    int priority = 0;

    std::string digest() const;
};

struct ReconciliationSummary {
    bool ok = true;
    int checked_receipts = 0;
    int checked_sessions = 0;
    int checked_policies = 0;
    Amount receipt_notional = 0;
    Amount receipt_fees = 0;
    Amount receipt_reserve = 0;
    std::vector<ReconciliationFinding> findings;

    std::string digest() const;
};

class Reconciler {
public:
    ReconciliationSummary inspect(const RiskEngine& engine) const;

private:
    void inspect_ledger(const RiskEngine& engine, ReconciliationSummary& summary) const;
    void inspect_receipts(const RiskEngine& engine, ReconciliationSummary& summary) const;
    void inspect_sessions(const RiskEngine& engine, ReconciliationSummary& summary) const;
    void inspect_policies(const RiskEngine& engine, ReconciliationSummary& summary) const;
    void add_finding(
        ReconciliationSummary& summary,
        const std::string& code,
        const std::string& subject,
        const std::string& detail,
        int priority
    ) const;
};

struct CliOptions {
    std::string command = "snapshot";
    std::string script_path;
    ReportVerbosity verbosity = ReportVerbosity::Standard;
};

CliOptions parse_cli(int argc, char** argv);
int run_cli(int argc, char** argv);

}  // namespace pylon

#endif
