#pragma once

#include "Beast.hpp"
#include "Net.hpp"

class SharedState;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(net::io_context& ioc, tcp::socket&& socket, std::shared_ptr<SharedState> const& doc_path);

    /**
     * @brief Run HttpSession and wait for incoming data
     */
    void run();

private:
    /**
     * @brief Called when the error occured during the session lifetime
     *
     * @param ec Error code
     * @param what What produced the error
     */
    void fail(beast::error_code ec, char const* what);

    /**
     * @brief Starts the async read from the socket stream
     */
    void doRead();

    /**
     * @brief Handler called after async read is done.
     * Initiates sesstion upgrade to WebSocket is update is requested.
     * Handles all http requests and create appropriate responses.
     *
     * @param ec Error code
     */
    void onRead(beast::error_code ec, std::size_t);

    /**
     * @brief Handler called after async write is done.
     * Initiate new reading from socket stream is conditions are met.
     *
     * @param ec Async write error code
     * @param keep_alive Keep alive flag
     */
    void onWrite(beast::error_code ec, std::size_t, bool keep_alive);

    net::io_context& ioc_;

    beast::tcp_stream stream_;
    std::shared_ptr<SharedState> shared_state_;
    beast::flat_buffer buffer_;

    std::optional<http::request_parser<http::string_body>> parser_;
};
