#include "Listener.hpp"

#include <iostream>

#include "HttpSession.hpp"
#include "SharedState.hpp"

Listener::Listener(net::io_context& ioc, tcp::endpoint endpoint,
                   std::shared_ptr<SharedState> const& shared_state)
    : ioc_{ioc}, acceptor_{net::make_strand(ioc)}, shared_state_{shared_state} {
    beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "Listener > Open");
        return;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        fail(ec, "Listener > SetOption");
        return;
    }

    acceptor_.bind(endpoint, ec);
    if (ec) {
        fail(ec, "Listener > Bind");
        return;
    }

    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "Listener > Listen");
        return;
    }
}

void Listener::run() {
    net::dispatch(
        acceptor_.get_executor(),
        beast::bind_front_handler(&Listener::doAccept, shared_from_this()));
}

void Listener::fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << std::endl;
}

void Listener::doAccept() {
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(&Listener::onAccept, shared_from_this()));
}

void Listener::onAccept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        fail(ec, "Listener::on_accept");
        return;
    } else {
        std::make_shared<HttpSession>(std::move(socket), shared_state_)->run();
    }

    doAccept();
}
