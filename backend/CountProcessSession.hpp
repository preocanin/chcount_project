#pragma once

#include <array>
#include <boost/asio/io_context.hpp>
#include <boost/process.hpp>
#include <filesystem>

class SharedState;

class CountProcessSession : public std::enable_shared_from_this<CountProcessSession> {
public:
    CountProcessSession(boost::asio::io_context& ioc, std::shared_ptr<SharedState> const& shared_state, char count_char,
                        std::filesystem::path file_path);

    ~CountProcessSession();

    void run();

private:
    void onRead(boost::system::error_code ec, std::size_t size);

    std::vector<char> buf_;
    boost::process::async_pipe ap_;
    std::filesystem::path file_path_;
    char count_char_;
    boost::process::child child_;
    std::shared_ptr<SharedState> shared_state_;
};
