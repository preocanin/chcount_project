#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class SharedState;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
public:
    HttpSession(tcp::socket&& socket, std::shared_ptr<SharedState> const& doc_path);

    void run();

private:
    void fail(beast::error_code ec, char const* what);

    void doRead();
    void onRead(beast::error_code ec, std::size_t);

    void onWrite(beast::error_code ec, std::size_t, bool keep_alive);

    beast::tcp_stream stream_;
    std::shared_ptr<SharedState> shared_state_;
    beast::flat_buffer buffer_;

    std::optional<http::request_parser<http::string_body>> parser_;
};
