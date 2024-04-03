add_executable(chcount_server
    # Sources
    main.cpp
    Listener.cpp
    HttpSession.cpp
    WebSocketSession.cpp
    SharedState.cpp
    CountProcessSession.cpp
    utils/MimeType.cpp

    # Headers
    Beast.hpp
    Net.hpp
    Listener.hpp
    HttpSession.hpp
    WebSocketSession.hpp
    SharedState.hpp
    CountProcessSession.hpp
    utils/Response.hpp
    utils/ContentType.hpp
    utils/MimeType.hpp
    dto/CountDto.hpp
)

target_link_libraries(chcount_server PRIVATE Boost::program_options Boost::json)

install(TARGETS chcount_server)
