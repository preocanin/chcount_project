#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class SharedState;

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(net::io_context& ioc, tcp::endpoint endpoint, std::shared_ptr<SharedState> const& shared_state);

    void run();

private:
    void fail(beast::error_code ec, char const* what);

    void doAccept();

    void onAccept(beast::error_code ec, tcp::socket socket);

    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<SharedState> shared_state_;
};
