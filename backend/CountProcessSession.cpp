#include "CountProcessSession.hpp"

#include <boost/beast.hpp>
#include <iostream>

#include "SharedState.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace bp = boost::process;
namespace fs = std::filesystem;

CountProcessSession::CountProcessSession(boost::asio::io_context& ioc, std::shared_ptr<SharedState> const& shared_state,
                                         char count_char, fs::path file_path)
    : buf_(2000), ap_{ioc}, file_path_{fs::absolute(file_path)}, count_char_{count_char}, shared_state_{shared_state} {}

CountProcessSession::~CountProcessSession() { fs::remove(file_path_); }

void CountProcessSession::run() {
    std::cout << "C: " << std::string(1, count_char_) << std::endl;

    child_ = bp::child(shared_state_->chcountExecutablePath().string(), "-c", std::string(1, count_char_), "-f",
                       file_path_.string(), bp::std_out > ap_);

    net::async_read(ap_, boost::asio::buffer(buf_),
                    beast::bind_front_handler(&CountProcessSession::onRead, shared_from_this()));
};

void CountProcessSession::onRead(boost::system::error_code ec, std::size_t size) {
    if (ec != boost::asio::error::eof) {
        std::cerr << "CountProcessSession::onRead: " << ec.message() << std::endl;
        return;
    }

    std::cout << "SIZE: " << size << std::endl;
    std::cout << "RESULT: " << std::string(buf_.cbegin(), buf_.cbegin() + size) << std::endl;
}
