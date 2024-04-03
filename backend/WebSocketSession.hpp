#pragma once

#include <boost/uuid/uuid.hpp>

#include "Beast.hpp"
#include "Net.hpp"

class SharedState;

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    WebSocketSession(tcp::socket&& socket, std::shared_ptr<SharedState> const& state);

    ~WebSocketSession();

    /**
     * @brief Run and setup websocket session and accept handshake
     *
     * @tparam Body Request body
     * @tparam Allocator Basic fields Allocator type
     * @param req Session upgrade request
     */
    template <class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req);

    /**
     * @brief Send msg to the websocket client
     *
     * @param msg Message
     */
    void send(std::shared_ptr<std::string const> const& msg);

    /**
     * @brief Get the websocket id
     *
     * @return boost::uuids::uuid
     */
    boost::uuids::uuid getId() const noexcept;

private:
    /**
     * @brief Called when the error occured during the session lifetime
     *
     * @param ec Error code
     * @param what What produced the error
     */
    void fail(beast::error_code ec, char const* what);

    /**
     * @brief Handles accept of the websocket connection
     * and sends the connection id to the client
     *
     * @param ec Error code
     */
    void onAccept(beast::error_code ec);

    /**
     * @brief Empyt buffer and initiate new async read from websocket
     *
     * @param ec Error code
     */
    void onRead(beast::error_code ec, std::size_t);

    /**
     * @brief Erase first element from message queue and if queue is not empty
     * initiate another async write with next message from queue
     *
     * @param ec Error code
     */
    void onWrite(beast::error_code ec, std::size_t);

    /**
     * @brief Add msg to message queue and
     * initiate async write if no other writes are currently in progress
     *
     * @param msg Message to send
     */
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
