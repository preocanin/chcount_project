add_executable(chcount main.cpp)

target_link_libraries(chcount PRIVATE Boost::program_options)

install(TARGETS chcount)
