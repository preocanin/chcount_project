#include "SharedState.hpp"

#include <boost/json/serialize.hpp>
#include <boost/json/value.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

#include "WebSocketSession.hpp"

namespace fs = std::filesystem;
namespace uuids = boost::uuids;
namespace json = boost::json;

SharedState::SharedState(fs::path docs, fs::path tmp_storage, fs::path chcount_executable)
    : docs_{std::move(docs)},
      tmp_storage_{std::move(tmp_storage)},
      chcount_executable_{std::move(chcount_executable)} {}

uuids::uuid SharedState::createUuid() noexcept { return random_gen_(); }

bool SharedState::contains(boost::uuids::uuid session_id) {
    std::lock_guard lock{mutex_};
    return (sessions_.count(session_id) != 0);
}

void SharedState::send(uuids::uuid user_id, uuids::uuid request_id, std::string msg) {
    json::value value{{"type", "result"}, {"data", {{"request_id", uuids::to_string(request_id)}, {"result", msg}}}};

    auto const& ss = std::make_shared<std::string const>(json::serialize(value));

    if (!contains(user_id)) {
        std::cerr << "SharedState::send: Session with \"" << uuids::to_string(user_id) << "\" doesn't exists"
                  << std::endl;
        return;
    }

    std::weak_ptr<WebSocketSession> ws;
    {
        std::lock_guard lock{mutex_};
        ws = sessions_[user_id]->weak_from_this();
    }

    if (auto sp = ws.lock()) {
        sp->send(ss);
    }
}

void SharedState::join(WebSocketSession* ws) {
    std::lock_guard lock{mutex_};
    sessions_.emplace(std::make_pair(ws->getId(), ws));
}

void SharedState::leave(WebSocketSession* ws) {
    std::lock_guard lock{mutex_};
    sessions_.erase(ws->getId());
}
