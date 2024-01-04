#pragma once

#include <boost/beast.hpp>

#include "utils/ContentType.hpp"

namespace http = boost::beast::http;

namespace response {

template <class Body, class Allocator>
http::response<http::string_body> createBadRequest(http::request<Body, http::basic_fields<Allocator>> const& req,
                                                   std::string_view why) {
    http::response<http::string_body> res{http::status::bad_request, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, utils::content_type::text_plain);
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
};

template <class Body, class Allocator>
http::response<http::string_body> createNotFound(http::request<Body, http::basic_fields<Allocator>> const& req,
                                                 std::string_view target) {
    http::response<http::string_body> res{http::status::not_found, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, utils::content_type::text_html);
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(target) + "' was not found.";
    res.prepare_payload();
    return res;
};

template <class Body, class Allocator>
http::response<http::string_body> createServerError(http::request<Body, http::basic_fields<Allocator>> const& req,
                                                    std::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, utils::content_type::text_html);
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
}

}  // namespace response
