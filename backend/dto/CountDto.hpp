#pragma once

#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

namespace dto {

class CountDto {
public:
    template <class RequestBody>
    static CountDto parse(RequestBody const& body);

    boost::uuids::uuid getId() const noexcept { return id_; }
    void setId(boost::uuids::uuid id) { id_ = std::move(id); }

    std::string getData() const noexcept { return data_; }
    void getData(std::string_view data) { data_ = data; }

private:
    boost::uuids::uuid id_;
    std::string data_;
};

// DEFINITIONS

template <class RequestBody>
CountDto CountDto::parse(RequestBody const& body) {
    namespace json = boost::json;
    namespace uuids = boost::uuids;

    CountDto result;

    boost::system::error_code ec;
    json::value json_body = json::parse(body, ec);

    if (ec || !json_body.is_object()) {
        throw std::runtime_error("Request body is not in valid json format");
    }

    auto obj_body = json_body.as_object();

    if (!obj_body.contains("id") || !obj_body.contains("data") || !obj_body.at("id").is_string() ||
        !obj_body.at("data").is_string()) {
        throw std::runtime_error("Request body is not valid json object");
    }

    auto id_value = obj_body.at("id");
    auto data_value = obj_body.at("data");

    try {
        result.id_ = boost::lexical_cast<uuids::uuid>(id_value.as_string().c_str());
    } catch (boost::bad_lexical_cast const&) {
        throw std::runtime_error("Request \"id\" is not in valid format");
    }

    result.data_ = data_value.as_string().c_str();

    return result;
}

}  // namespace dto
