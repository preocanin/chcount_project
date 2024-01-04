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
    explicit SharedState(std::filesystem::path tmp_storage, std::filesystem::path chcount_executable);

    boost::uuids::uuid createUuid() noexcept;

    std::filesystem::path tmpStoragePath() const noexcept { return tmp_storage_; }
    std::filesystem::path chcountExecutablePath() const noexcept { return chcount_executable_; }

    bool contains(boost::uuids::uuid session_id);

    void send(boost::uuids::uuid user_id, boost::uuids::uuid request_id, std::string result);

    void join(WebSocketSession* ws);
    void leave(WebSocketSession* ws);

private:
    std::mutex mutex_;

    std::filesystem::path tmp_storage_;
    std::filesystem::path chcount_executable_;
    boost::uuids::random_generator random_gen_;

    std::unordered_map<boost::uuids::uuid, WebSocketSession*, boost::hash<boost::uuids::uuid>> sessions_;
};
