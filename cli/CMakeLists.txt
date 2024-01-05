cmake_minimum_required(VERSION 3.5)
project(chcount_project_cli)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror -Wextra -Wpedantic)

find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(chcount main.cpp)

target_link_libraries(chcount PRIVATE Boost::program_options)
