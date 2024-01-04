#include "SharedState.hpp"

#include "WebSocketSession.hpp"

namespace fs = std::filesystem;
namespace uuids = boost::uuids;

SharedState::SharedState(fs::path tmp_storage) : tmp_storage_{std::move(tmp_storage)} {}

uuids::uuid SharedState::createUuid() noexcept { return random_gen_(); }

void SharedState::join(WebSocketSession* ws) {
    std::lock_guard lock{mutex_};
    sessions_.emplace(std::make_pair(ws->getId(), ws));
}

void SharedState::leave(WebSocketSession* ws) {
    std::lock_guard lock{mutex_};
    sessions_.erase(ws->getId());
}
