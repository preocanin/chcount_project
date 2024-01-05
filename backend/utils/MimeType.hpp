#pragma once

#include <string_view>

#include "ContentType.hpp"

namespace utils {

std::string_view getMimeType(std::string_view path);

}  // namespace utils
