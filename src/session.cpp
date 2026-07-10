#include "pylon.hpp"

namespace pylon {

Session SessionManager::open_session(
    const std::string& id,
    const std::string& operator_id,
    const std::string& asset_id,
    const std::string& channel_id,
    const std::string& funding_account,
    SessionMode mode,
    const LimitSnapshot& snapshot,
    Sequence sequence
) {
    require_identifier(id, "session id");
    if (sessions_.find(id) != sessions_.end()) {
        throw RuntimeError("session already exists: " + id);
    }
    Session session;
    session.id = id;
    session.operator_id = operator_id;
    session.asset_id = asset_id;
    session.channel_id = channel_id;
    session.funding_account = funding_account;
    session.mode = mode;
    session.snapshot = snapshot;
    session.opened_sequence = sequence;
    session.last_sequence = sequence;
    session.open = true;
    sessions_[id] = session;
    ++opened_;
    return session;
}

bool SessionManager::has_session(const std::string& session_id) const {
    return sessions_.find(session_id) != sessions_.end();
}

Session* SessionManager::find_session(const std::string& session_id) {
    const auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return nullptr;
    }
    return &it->second;
}

const Session* SessionManager::find_session(const std::string& session_id) const {
    const auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return nullptr;
    }
    return &it->second;
}

std::vector<Session> SessionManager::sessions() const {
    std::vector<Session> result;
    result.reserve(sessions_.size());
    for (const auto& pair : sessions_) {
        result.push_back(pair.second);
    }
    return result;
}

int SessionManager::open_count_for_operator(const std::string& operator_id) const {
    int count = 0;
    for (const auto& pair : sessions_) {
        const Session& session = pair.second;
        if (session.operator_id == operator_id && session.open) {
            ++count;
        }
    }
    return count;
}

void SessionManager::close_session(const std::string& session_id, Sequence sequence) {
    Session* session = find_session(session_id);
    if (!session) {
        throw RuntimeError("unknown session: " + session_id);
    }
    if (session->open) {
        session->open = false;
        session->last_sequence = sequence;
        ++closed_;
    }
}

void SessionManager::note_execution(
    const std::string& session_id,
    Amount notional,
    Amount reserve,
    Sequence sequence
) {
    Session* session = find_session(session_id);
    if (!session) {
        throw RuntimeError("unknown session: " + session_id);
    }
    session->committed_notional = checked_add(session->committed_notional, notional);
    session->committed_reserve = checked_add(session->committed_reserve, reserve);
    session->last_sequence = sequence;
    ++session->orders;
}

int SessionManager::opened() const {
    return opened_;
}

int SessionManager::closed() const {
    return closed_;
}

std::string SessionManager::digest() const {
    std::vector<std::string> fields;
    fields.push_back(amount_to_string(opened_));
    fields.push_back(amount_to_string(closed_));
    for (const auto& pair : sessions_) {
        fields.push_back(pair.second.digest());
    }
    return digest_fields(fields);
}

}  // namespace pylon
