#include "MimeType.hpp"

#include "ContentType.hpp"

std::string_view utils::getMimeType(std::string_view path) {
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == std::string_view::npos) return std::string_view{};
        return path.substr(pos);
    }();

    if (ext == ".htm") return "text/html";
    if (ext == ".html") return "text/html";
    if (ext == ".php") return content_type::text_html;
    if (ext == ".css") return "text/css";
    if (ext == ".txt") return content_type::text_plain;
    if (ext == ".js") return "application/javascript";
    if (ext == ".json") return content_type::application_json;
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
