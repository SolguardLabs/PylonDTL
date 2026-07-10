#include "pylon.hpp"

#include <fstream>

namespace pylon {

ScenarioReport ScriptRunner::run_file(const std::string& path) const {
    std::ifstream in(path);
    if (!in) {
        throw RuntimeError("could not open script: " + path);
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    return run_lines(lines);
}

ScenarioReport ScriptRunner::run_lines(const std::vector<std::string>& lines) const {
    ScenarioReport report;
    report.name = "scripted";
    report.engine.bootstrap_default();
    for (std::size_t i = 0; i < lines.size(); ++i) {
        execute_line(report.engine, report, lines[i], static_cast<int>(i + 1));
    }
    if (report.notes.empty()) {
        report.notes.push_back("script completed");
    }
    return report;
}

void ScriptRunner::execute_line(
    RiskEngine& engine,
    ScenarioReport& report,
    const std::string& line,
    int line_no
) const {
    const std::string cleaned = trim(line);
    if (cleaned.empty() || starts_with(cleaned, "#")) {
        return;
    }
    const std::vector<std::string> tokens = split_ws(cleaned);
    if (tokens.empty()) {
        return;
    }
    const std::string command = uppercase(tokens[0]);
    if (command == "SCENARIO") {
        if (tokens.size() != 2) {
            throw RuntimeError("line " + amount_to_string(line_no) + ": SCENARIO expects name");
        }
        report.name = require_identifier(tokens[1], "scenario name");
        return;
    }
    if (command == "NOTE") {
        const std::size_t pos = cleaned.find(' ');
        if (pos != std::string::npos) {
            report.notes.push_back(cleaned.substr(pos + 1));
        }
        return;
    }
    if (command == "OPEN") {
        if (tokens.size() != 7) {
            throw RuntimeError(
                "line " + amount_to_string(line_no) +
                ": OPEN id operator asset channel account mode"
            );
        }
        engine.open_session(
            require_identifier(tokens[1], "session id"),
            require_identifier(tokens[2], "operator id"),
            require_identifier(tokens[3], "asset id"),
            require_identifier(tokens[4], "channel id"),
            require_identifier(tokens[5], "account id"),
            parse_session_mode(tokens[6])
        );
        return;
    }
    if (command == "CLOSE") {
        if (tokens.size() != 2) {
            throw RuntimeError("line " + amount_to_string(line_no) + ": CLOSE expects session id");
        }
        engine.close_session(tokens[1]);
        return;
    }
    if (command == "EPOCH") {
        if (tokens.size() != 5) {
            throw RuntimeError(
                "line " + amount_to_string(line_no) +
                ": EPOCH label risk_bps liquidity_bps settlement_bps"
            );
        }
        engine.rotate_epoch(tokens[1], parse_int(tokens[2]), parse_int(tokens[3]), parse_int(tokens[4]));
        return;
    }
    if (command == "ORDER") {
        engine.execute(build_order(tokens, line_no));
        return;
    }
    if (command == "QUOTE") {
        if (tokens.size() != 6) {
            throw RuntimeError(
                "line " + amount_to_string(line_no) +
                ": QUOTE operator asset channel notional fee_bps"
            );
        }
        report.quotes.push_back(engine.quote(
            PolicyKey(tokens[1], tokens[2], tokens[3]),
            parse_amount(tokens[4]),
            parse_int(tokens[5])
        ));
        return;
    }
    if (command == "DEPOSIT") {
        if (tokens.size() != 4) {
            throw RuntimeError("line " + amount_to_string(line_no) + ": DEPOSIT account asset amount");
        }
        engine.ledger().deposit(tokens[1], tokens[2], parse_amount(tokens[3]), "script-deposit");
        return;
    }
    throw RuntimeError("line " + amount_to_string(line_no) + ": unknown command " + tokens[0]);
}

OrderIntent ScriptRunner::build_order(const std::vector<std::string>& tokens, int line_no) const {
    if (tokens.size() != 12) {
        throw RuntimeError(
            "line " + amount_to_string(line_no) +
            ": ORDER id session operator asset channel source beneficiary notional units max_fee_bps priority"
        );
    }
    OrderIntent intent;
    intent.id = require_identifier(tokens[1], "order id");
    intent.session_id = require_identifier(tokens[2], "session id");
    intent.operator_id = require_identifier(tokens[3], "operator id");
    intent.asset_id = require_identifier(tokens[4], "asset id");
    intent.channel_id = require_identifier(tokens[5], "channel id");
    intent.source_account = require_identifier(tokens[6], "source account");
    intent.beneficiary_account = require_identifier(tokens[7], "beneficiary account");
    intent.notional = parse_amount(tokens[8]);
    intent.units = parse_amount(tokens[9]);
    intent.requested_fee_bps = parse_int(tokens[10]);
    intent.priority = parse_int(tokens[11]);
    intent.side = OrderSide::Debit;
    intent.settlement_ref = "script:" + intent.id;
    return intent;
}

}  // namespace pylon
