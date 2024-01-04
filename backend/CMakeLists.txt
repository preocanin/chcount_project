cmake_minimum_required(VERSION 3.5)
project(chcount_project_backend)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall -Werror -Wextra -Wpedantic)

find_package(Boost COMPONENTS program_options json REQUIRED)

add_executable(chcount_server
    # Sources
    main.cpp
    Listener.cpp
    HttpSession.cpp
    WebSocketSession.cpp
    SharedState.cpp
    CountProcessSession.cpp

    # Headers
    Listener.hpp
    HttpSession.hpp
    Response.hpp
    WebSocketSession.hpp
    SharedState.hpp
    CountProcessSession.hpp
    utils/ContentType.hpp
)

target_link_libraries(chcount_server PRIVATE Boost::program_options Boost::json)
