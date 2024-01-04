#include "CountProcessSession.hpp"

#include <boost/beast.hpp>
#include <iostream>

#include "SharedState.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
namespace uuids = boost::uuids;
namespace bp = boost::process;
namespace fs = std::filesystem;

CountProcessSession::CountProcessSession(boost::asio::io_context& ioc, std::shared_ptr<SharedState> const& shared_state,
                                         uuids::uuid user_id, uuids::uuid request_id, char count_char,
                                         fs::path file_path)
    : buf_(2000),
      user_id_{std::move(user_id)},
      request_id_{std::move(request_id)},
      ap_{ioc},
      file_path_{fs::absolute(file_path)},
      count_char_{count_char},
      shared_state_{shared_state} {}

CountProcessSession::~CountProcessSession() { fs::remove(file_path_); }

void CountProcessSession::run() {
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

    shared_state_->send(user_id_, request_id_, std::string(buf_.cbegin(), buf_.cbegin() + size - 1));
}
