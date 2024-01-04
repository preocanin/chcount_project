#include "HttpSession.hpp"

#include <boost/format.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "Request.hpp"
#include "SharedState.hpp"
#include "WebSocketSession.hpp"

namespace websocket = beast::websocket;
namespace uuids = boost::uuids;
namespace fs = std::filesystem;

// Utilities

std::string_view mime_type(std::string_view path);

template <class Body, class Allocator>
http::message_generator handle_request(fs::path tmp_storage, http::request<Body, http::basic_fields<Allocator>>&& req);

fs::path createTmpFilePath(fs::path tmp_storage) {
    uuids::random_generator rgen;

    auto const& uuid = uuids::to_string(rgen());
    tmp_storage += "/";
    tmp_storage += (boost::format("%1%.txt") % uuid).str();

    return tmp_storage;
}

template <class Body>
fs::path writeBodyToTempFile(fs::path tmp_storage, Body&& body) {
    auto const tmp_file_path = createTmpFilePath(tmp_storage);

    try {
        std::ofstream fout{tmp_file_path};

        if (!fout.is_open()) {
            return {};
        }

        fout << body;
        fout.close();

        return tmp_file_path;
    } catch (std::exception const& e) {
        std::cerr << "Cannot create temporary file \"" << tmp_file_path << "\"" << std::endl;
        return {};
    }
}

// --------------

HttpSession::HttpSession(tcp::socket&& socket, std::shared_ptr<SharedState> const& shared_state)
    : stream_{std::move(socket)}, shared_state_{shared_state} {}

void HttpSession::run() {
    net::dispatch(stream_.get_executor(), beast::bind_front_handler(&HttpSession::doRead, shared_from_this()));
}

void HttpSession::fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << std::endl;
}

void HttpSession::doRead() {
    parser_.emplace();
    parser_->body_limit(20000);

    // Closes socket if we didn't get
    stream_.expires_after(std::chrono::seconds(30));

    http::async_read(stream_, buffer_, parser_->get(),
                     beast::bind_front_handler(&HttpSession::onRead, shared_from_this()));
}

void HttpSession::onRead(beast::error_code ec, std::size_t) {
    // Client closed the connection
    if (ec == http::error::end_of_stream) {
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    if (ec) {
        return fail(ec, "HttpSession::onRead");
    }

    // Upgrade to websocket
    if (websocket::is_upgrade(parser_->get())) {
        std::make_shared<WebSocketSession>(stream_.release_socket(), shared_state_)->run(parser_->release());
        return;
    }

    // Handle request
    http::message_generator msg = handle_request(shared_state_->tmpStoragePath(), parser_->release());

    bool const keep_alive = msg.keep_alive();

    auto self = shared_from_this();

    beast::async_write(stream_, std::move(msg), [self, keep_alive](beast::error_code ec, std::size_t bytes) {
        self->onWrite(ec, bytes, keep_alive);
    });
}

void HttpSession::onWrite(beast::error_code ec, std::size_t, bool keep_alive) {
    if (ec) {
        return fail(ec, "HttpSession::onWrite");
    }

    if (!keep_alive) {
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    doRead();
}

// Utilities DEFINITIONS

std::string_view mime_type(std::string_view path) {
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == std::string_view::npos) return std::string_view{};
        return path.substr(pos);
    }();

    if (ext == ".htm") return "text/html";
    if (ext == ".html") return "text/html";
    if (ext == ".php") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".txt") return "text/plain";
    if (ext == ".js") return "application/javascript";
    if (ext == ".json") return "application/json";
    if (ext == ".xml") return "application/xml";
    if (ext == ".swf") return "application/x-shockwave-flash";
    if (ext == ".flv") return "video/x-flv";
    if (ext == ".png") return "image/png";
    if (ext == ".jpe") return "image/jpeg";
    if (ext == ".jpeg") return "image/jpeg";
    if (ext == ".jpg") return "image/jpeg";
    if (ext == ".gif") return "image/gif";
    if (ext == ".bmp") return "image/bmp";
    if (ext == ".ico") return "image/vnd.microsoft.icon";
    if (ext == ".tiff") return "image/tiff";
    if (ext == ".tif") return "image/tiff";
    if (ext == ".svg") return "image/svg+xml";
    if (ext == ".svgz") return "image/svg+xml";
    return "application/text";
}

template <class Body, class Allocator>
http::message_generator handle_request(fs::path tmp_storage, http::request<Body, http::basic_fields<Allocator>>&& req) {
    using namespace request;

    auto const& method = req.method();
    auto const& target = req.target();
    auto const& content_type = req[http::field::content_type];
    auto const& body = req.body();

    std::cout << "METHOD: " << method << std::endl;
    std::cout << "TARGET: " << target << std::endl;
    std::cout << "CONTENT TYPE: " << content_type << std::endl;

    // GET: /
    if (method == http::verb::get && target == "/") {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, content_type::text_plain);
        res.content_length(0);
        res.keep_alive(req.keep_alive());

        return res;
    }
    // METHOD: POST
    // PATH: /api/count
    // CONTENT_TYPE: application/json
    else if (method == http::verb::post && target == "/api/count" && content_type == content_type::application_json) {
        auto const tmp_file = writeBodyToTempFile(tmp_storage, body);

        std::cout << "TMP_FILE: " << tmp_file << std::endl;

        if (!tmp_file.empty()) {
            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, content_type::text_plain);
            res.content_length(0);
            res.keep_alive(req.keep_alive());
            return res;
        } else {
            return createBadRequest(req, "Cannot create tmp file");
        }
    }
    // Unsupported
    else {
        return createBadRequest(req, "Unsupported HTTP-method or Content-Type");
    }

    /*
    // Make sure we can handle the method
    if (req.method() != http::verb::get && req.method() != http::verb::head) {
        return createBadRequest(req, "Unknown HTTP-method");
    }

    // Request path must be absolute and not contain "..".
    if (req.target().empty() || req.target()[0] != '/' ||
        req.target().find("..") != beast::string_view::npos) {
        return createBadRequest(req, "Illegal request-target");
    }

    // Build the path to the requested file
    auto docPath = std::filesystem::path(doc_root);
    docPath += std::filesystem::path(req.target());

    std::string path = docPath;
    if (req.target().back() == '/') path.append("index.html");

    // Attempt to open the file
    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if (ec == boost::system::errc::no_such_file_or_directory) {
        return createNotFound(req, req.target());
    }

    // Handle an unknown error
    if (ec) {
        return createServerError(req, ec.message());
    }

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if (req.method() == http::verb::head) {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    }

    // Respond to GET request
    http::response<http::file_body> res{
        std::piecewise_construct, std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return res;
    */
}
