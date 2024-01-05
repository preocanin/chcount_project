#include "WebSocketSession.hpp"

#include <boost/json.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>

#include "SharedState.hpp"

namespace uuids = boost::uuids;
namespace json = boost::json;

WebSocketSession::WebSocketSession(tcp::socket&& socket, std::shared_ptr<SharedState> const& shared_state)
    : id_{shared_state_->createUuid()}, ws_{std::move(socket)}, shared_state_{shared_state} {}

WebSocketSession::~WebSocketSession() { shared_state_->leave(this); }

void WebSocketSession::fail(beast::error_code ec, char const* what) {
    if (ec == net::error::operation_aborted || ec == websocket::error::closed) {
        return;
    }

    std::cerr << what << ": " << ec.message() << std::endl;
}

void WebSocketSession::onAccept(beast::error_code ec) {
    if (ec) {
        return fail(ec, "WebSocketSession::onAccept");
    }

    std::cout << "Connection accepted" << std::endl;

    shared_state_->join(this);

    json::value value{{"type", "id"}, {"data", uuids::to_string(id_)}};
    send(std::make_shared<std::string>(json::serialize(value)));

    ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketSession::onRead, shared_from_this()));
}

void WebSocketSession::onRead(beast::error_code ec, std::size_t) {
    if (ec) {
        return fail(ec, "WebSocketSession::onRead");
    }

    buffer_.consume(buffer_.size());

    ws_.async_read(buffer_, beast::bind_front_handler(&WebSocketSession::onRead, shared_from_this()));
}

void WebSocketSession::send(std::shared_ptr<std::string const> const& msg) {
    net::post(ws_.get_executor(), beast::bind_front_handler(&WebSocketSession::onSend, shared_from_this(), msg));
}

void WebSocketSession::onSend(std::shared_ptr<std::string const> const& msg) {
    queue_.push_back(msg);

    if (queue_.size() > 1) {
        return;
    }

    ws_.async_write(net::buffer(*queue_.front()),
                    beast::bind_front_handler(&WebSocketSession::onWrite, shared_from_this()));
}

void WebSocketSession::onWrite(beast::error_code ec, std::size_t) {
    if (ec) {
        return fail(ec, "WebSocketSession::onWrite");
    }

    queue_.erase(queue_.begin());

    if (!queue_.empty()) {
        ws_.async_write(net::buffer(*queue_.front()),
                        beast::bind_front_handler(&WebSocketSession::onWrite, shared_from_this()));
    }
}

uuids::uuid WebSocketSession::getId() const noexcept { return id_; }
