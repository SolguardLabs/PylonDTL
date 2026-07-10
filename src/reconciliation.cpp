#include "pylon.hpp"

namespace pylon {

std::string ReconciliationFinding::digest() const {
    return digest_fields({
        code,
        subject,
        detail,
        amount_to_string(priority),
    });
}

std::string ReconciliationSummary::digest() const {
    std::vector<std::string> fields;
    fields.push_back(ok ? "1" : "0");
    fields.push_back(amount_to_string(checked_receipts));
    fields.push_back(amount_to_string(checked_sessions));
    fields.push_back(amount_to_string(checked_policies));
    fields.push_back(amount_to_string(receipt_notional));
    fields.push_back(amount_to_string(receipt_fees));
    fields.push_back(amount_to_string(receipt_reserve));
    for (const ReconciliationFinding& finding : findings) {
        fields.push_back(finding.digest());
    }
    return digest_fields(fields);
}

ReconciliationSummary Reconciler::inspect(const RiskEngine& engine) const {
    ReconciliationSummary summary;
    inspect_ledger(engine, summary);
    inspect_receipts(engine, summary);
    inspect_sessions(engine, summary);
    inspect_policies(engine, summary);
    return summary;
}

void Reconciler::inspect_ledger(const RiskEngine& engine, ReconciliationSummary& summary) const {
    if (!engine.ledger().conservation_ok()) {
        add_finding(summary, "ledger-conservation", "ledger", "asset totals diverged from bootstrap", 1);
    }
    for (const AccountSnapshot& account : engine.ledger().accounts()) {
        for (const auto& balance : account.balances) {
            if (balance.second < 0) {
                add_finding(
                    summary,
                    "negative-balance",
                    account.id,
                    "negative balance for " + balance.first,
                    1
                );
            }
        }
    }
}

void Reconciler::inspect_receipts(const RiskEngine& engine, ReconciliationSummary& summary) const {
    for (const ExecutionReceipt& receipt : engine.receipts()) {
        ++summary.checked_receipts;
        if (!receipt.settled()) {
            continue;
        }
        summary.receipt_notional = checked_add(summary.receipt_notional, receipt.notional);
        summary.receipt_fees = checked_add(summary.receipt_fees, receipt.fee);
        summary.receipt_reserve = checked_add(summary.receipt_reserve, receipt.reserve);
        const Amount composed = checked_add(receipt.net_amount, checked_add(receipt.fee, receipt.reserve));
        if (composed != receipt.notional) {
            add_finding(
                summary,
                "receipt-composition",
                receipt.id,
                "net, fee and reserve do not compose to notional",
                2
            );
        }
        if (receipt.digest().empty()) {
            add_finding(summary, "receipt-digest", receipt.id, "missing receipt digest", 3);
        }
    }
}

void Reconciler::inspect_sessions(const RiskEngine& engine, ReconciliationSummary& summary) const {
    for (const Session& session : engine.sessions().sessions()) {
        ++summary.checked_sessions;
        Amount receipt_notional = 0;
        Amount receipt_reserve = 0;
        int receipt_count = 0;
        for (const ExecutionReceipt& receipt : engine.receipts()) {
            if (receipt.session_id == session.id && receipt.settled()) {
                receipt_notional = checked_add(receipt_notional, receipt.notional);
                receipt_reserve = checked_add(receipt_reserve, receipt.reserve);
                ++receipt_count;
            }
        }
        if (receipt_notional != session.committed_notional) {
            add_finding(
                summary,
                "session-notional",
                session.id,
                "session committed notional differs from settled receipts",
                2
            );
        }
        if (receipt_reserve != session.committed_reserve) {
            add_finding(
                summary,
                "session-reserve",
                session.id,
                "session committed reserve differs from settled receipts",
                2
            );
        }
        if (receipt_count != session.orders) {
            add_finding(
                summary,
                "session-order-count",
                session.id,
                "session order count differs from settled receipts",
                3
            );
        }
    }
}

void Reconciler::inspect_policies(const RiskEngine& engine, ReconciliationSummary& summary) const {
    const ExposureTotals totals = engine.exposure().totals();
    for (const QuotaState& quota : engine.policies().all_quotas(totals)) {
        ++summary.checked_policies;
        if (!quota.enabled) {
            add_finding(summary, "policy-disabled", quota.key.str(), "policy is not active", 4);
            continue;
        }
        if (quota.operator_limit <= 0 || quota.asset_limit <= 0 || quota.channel_limit <= 0 ||
            quota.global_limit <= 0) {
            add_finding(summary, "policy-empty-limit", quota.key.str(), "computed limit is zero", 2);
        }
        if (quota.fee_bps > quota.max_fee_bps) {
            add_finding(summary, "policy-fee-envelope", quota.key.str(), "computed fee exceeds max fee", 2);
        }
    }
}

void Reconciler::add_finding(
    ReconciliationSummary& summary,
    const std::string& code,
    const std::string& subject,
    const std::string& detail,
    int priority
) const {
    ReconciliationFinding finding;
    finding.code = code;
    finding.subject = subject;
    finding.detail = detail;
    finding.priority = priority;
    summary.findings.push_back(finding);
    if (priority <= 2) {
        summary.ok = false;
    }
}

}  // namespace pylon
