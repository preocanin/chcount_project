#pragma once

#include <array>
#include <boost/process.hpp>
#include <boost/uuid/uuid.hpp>
#include <filesystem>

#include "Net.hpp"

class SharedState;

class CountProcessSession : public std::enable_shared_from_this<CountProcessSession> {
public:
    CountProcessSession(net::io_context& ioc, std::shared_ptr<SharedState> const& shared_state,
                        boost::uuids::uuid user_id, boost::uuids::uuid request_id, char count_char,
                        std::filesystem::path file_path);

    ~CountProcessSession();

    /**
     * @brief Runs new count process and read from a pipe in provided context
     */
    void run();

private:
    /**
     * @brief Handle the data after the async pipe read is done
     *
     * @param ec Error code
     * @param size Readed data size
     */
    void onRead(boost::system::error_code ec, std::size_t size);

    std::vector<char> buf_;
    boost::uuids::uuid user_id_;
    boost::uuids::uuid request_id_;
    boost::process::async_pipe ap_;
    std::filesystem::path file_path_;
    char count_char_;
    boost::process::child child_;
    std::shared_ptr<SharedState> shared_state_;
};
