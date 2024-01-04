#pragma once

#include <boost/beast.hpp>
#include <boost/uuid/uuid.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

class SharedState;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    WebSocketSession(tcp::socket&& socket, std::shared_ptr<SharedState> const& state);
    ~WebSocketSession();

    template <class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req);

    void send(std::shared_ptr<std::string const> const& msg);

    boost::uuids::uuid getId() const noexcept;

private:
    void fail(beast::error_code ec, char const* what);
    void onAccept(beast::error_code ec);
    void onRead(beast::error_code ec, std::size_t bytes_transfered);
    void onWrite(beast::error_code ec, std::size_t bytes_transfered);
    void onSend(std::shared_ptr<std::string const> const& msg);

    boost::uuids::uuid id_;
    beast::flat_buffer buffer_;
    websocket::stream<beast::tcp_stream> ws_;
    std::shared_ptr<SharedState> shared_state_;
    std::vector<std::shared_ptr<std::string const>> queue_;
};

// DEFINITIONS

template <class Body, class Allocator>
inline void WebSocketSession::run(http::request<Body, http::basic_fields<Allocator>> req) {
    // Set suggested timeout settings for the websocket
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
        res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-chcount-server");
    }));

    // Accept the websocket handshake
    ws_.async_accept(req, beast::bind_front_handler(&WebSocketSession::onAccept, shared_from_this()));
}
