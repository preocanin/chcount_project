cmake_minimum_required(VERSION 3.5)
project(chcount_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Werror -Wextra -Wpedantic)

find_package(Boost 1.83.0 COMPONENTS program_options json REQUIRED)

add_subdirectory(backend)
add_subdirectory(cli)
