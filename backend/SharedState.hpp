#pragma once

#include <boost/container_hash/hash.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <filesystem>
#include <mutex>
#include <unordered_map>

class WebSocketSession;

class SharedState {
public:
    explicit SharedState(std::filesystem::path tmp_storage);

    boost::uuids::uuid createUuid() noexcept;

    std::filesystem::path tmpStoragePath() const noexcept { return tmp_storage_; }

    void join(WebSocketSession* ws);
    void leave(WebSocketSession* ws);

private:
    std::filesystem::path tmp_storage_;
    boost::uuids::random_generator random_gen_;

    std::mutex mutex_;
    std::unordered_map<boost::uuids::uuid, WebSocketSession*, boost::hash<boost::uuids::uuid>> sessions_;
};