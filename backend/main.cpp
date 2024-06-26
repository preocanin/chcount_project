#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>

#include "Listener.hpp"
#include "SharedState.hpp"

namespace net = boost::asio;
namespace fs = std::filesystem;
using tcp = boost::asio::ip::tcp;

struct Options {
    std::string host;
    fs::path docs;
    fs::path tmp_storage;
    fs::path chcount_executable;
    net::ip::port_type port;
};

/**
 * @brief Parses command line arguments and check for errors. Resturns parsed
 * program options.
 *
 * @param argc Command line arguments count
 * @param argv Command line arguments strings
 * @return Parsed program options
 */
Options parseArgumentOptions(int argc, char** argv);

int main(int argc, char** argv) {
    auto const options = parseArgumentOptions(argc, argv);

    auto const& port = options.port;

    boost::system::error_code ec;
    auto const host = net::ip::address::from_string(options.host, ec);

    if (ec) {
        std::cerr << "Error: Invalid host string \"" << options.host << "\"" << std::endl;
        return EXIT_FAILURE;
    }

    auto const threads_count = std::thread::hardware_concurrency() - 1;

    net::io_context ioc{static_cast<int>(threads_count)};

    std::make_shared<Listener>(
        ioc, tcp::endpoint{host, port},
        std::make_shared<SharedState>(options.docs, options.tmp_storage, options.chcount_executable))
        ->run();

    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](boost::system::error_code const&, int) { ioc.stop(); });

    std::vector<std::thread> threads;
    threads.reserve(threads_count);

    for (auto i = threads_count; i > 0; --i) {
        threads.emplace_back([&ioc] { ioc.run(); });
    }

    std::cout << "Server listening on: " << host.to_string() << ":" << port << std::endl;
    ioc.run();

    for (auto& t : threads) {
        t.join();
    }

    return EXIT_SUCCESS;
}

// DEFINITIONS

namespace po = boost::program_options;

void printErrorMessage(std::string const& error_message, po::options_description const& desc) {
    std::cerr << "Error: " << error_message << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << desc << std::endl;
}

void exitWithErrorMessage(std::string const& error_message, po::options_description const& desc) {
    printErrorMessage(error_message, desc);
    exit(EXIT_FAILURE);
}

Options parseArgumentOptions(int argc, char** argv) {
    Options result;
    std::int32_t port;
    std::string docs;
    std::string tmp_storage;
    std::string chcount_executable;

    po::options_description desc("Options");

    // clang-format off
    desc.add_options()
        ("help", "Help message")
        ("host,H", po::value<std::string>(&result.host)->default_value("127.0.0.1"), "Host on which server listens")
        ("port,P", po::value<std::int32_t>(&port)->default_value(3000), "Port on which server listens")
        ("docs,D", po::value<std::string>(&docs), "Served documents location directory")
        ("tmp-storage,T", po::value<std::string>(&tmp_storage)->default_value("."), "Temporary storage directory")
        ("chcount-executable", po::value<std::string>(&chcount_executable), "Chcount executable path");
    // clang-format on

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            exit(EXIT_SUCCESS);
        }

        // port checks
        if (port < 0) {
            exitWithErrorMessage("Port must be positive number", desc);
        }

        result.port = static_cast<decltype(result.port)>(port);

        if (tmp_storage.empty()) {
            exitWithErrorMessage("Temporary storage path cannot be empty", desc);
        }

        // docs
        if (!vm.count("docs")) {
            exitWithErrorMessage("Docs location path must be provided", desc);
        }

        result.docs = fs::absolute(docs);

        if (!fs::exists(result.docs)) {
            exitWithErrorMessage("Docs location path doesn't exists", desc);
        }

        if (!fs::is_directory(result.docs)) {
            exitWithErrorMessage("Docs location path must be a directory", desc);
        }

        // tmp-storage checks
        result.tmp_storage = fs::absolute(tmp_storage);

        if (!fs::exists(result.tmp_storage)) {
            exitWithErrorMessage("Temporary storage path doesn't exists", desc);
        }

        if (!fs::is_directory(result.tmp_storage)) {
            exitWithErrorMessage("Temporary storage path must be a direstory", desc);
        }

        // chcount-executable checks
        if (!vm.count("chcount-executable")) {
            exitWithErrorMessage("Chcount path must be provided", desc);
        }

        if (chcount_executable.empty()) {
            exitWithErrorMessage("Chcount path cannot be empty", desc);
        }

        result.chcount_executable = fs::absolute(chcount_executable);

        if (!fs::exists(result.chcount_executable)) {
            exitWithErrorMessage("Chcount executable doesn't exists", desc);
        }

        if (!fs::is_regular_file(result.chcount_executable)) {
            exitWithErrorMessage("Chcount executable must be a regular file", desc);
        }

        return result;
    } catch (std::exception const& e) {
        printErrorMessage(e.what(), desc);
        exit(EXIT_FAILURE);
    }
}
