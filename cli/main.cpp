#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <numeric>
#include <thread>

using FutureUintmax = std::future<std::uintmax_t>;

struct Options {
    char character;
    std::string file_path;
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

/**
 * @brief Returns the number of elements in ragne [first, first+n) which are
 * equal to value if first+n not exeeds last, otherwise instead of [first,
 * first+n) range is [first, last).
 *
 * @tparam InputIt Input iterator
 * @tparam T Value type
 * @param first Iterator to first element
 * @param last Iterator to last element
 * @param n Number of elements to check
 * @param value Value which we count
 * @return Value occurence count
 */
template <class InputIt, typename T>
std::uint64_t count_if_n(InputIt first, InputIt last, std::uint64_t n, T const& value);

/**
 * @brief Create a Worker object function
 *
 * @param star_pos Start position for counting the characters
 * @param chunk_size Size of the chunk on which worker searches
 * @param options Options
 * @return std::function<uintmax_t()> Function which counts character on specific part of the file
 */
std::function<uintmax_t()> createWorker(std::uintmax_t start_pos, std::uintmax_t chunk_size, Options options);

int main(int argc, char** argv) {
    auto const options = parseArgumentOptions(argc, argv);

    auto const file_size = std::filesystem::file_size(options.file_path);

    auto threads_count = std::thread::hardware_concurrency() - 1;

    std::uintmax_t result = 0;

    if (threads_count == 0 || file_size <= threads_count) {  // Single thread or file too small for separation
        std::ifstream fin(options.file_path);

        result = std::count_if(std::istream_iterator<char>(fin), std::istream_iterator<char>(),
                               [&options](char c) { return c == options.character; });
    } else {  // Multithread
        auto per_thread_chunk_size = file_size / threads_count;
        auto leftover_bytes = file_size % threads_count;

        std::vector<FutureUintmax> workers;

        for (unsigned i = 0; i < threads_count; ++i) {
            auto const chunk_size = per_thread_chunk_size + (i != threads_count - 1 ? 0 : leftover_bytes);

            auto worker = std::async(std::launch::async, createWorker(i * per_thread_chunk_size, chunk_size, options));

            workers.emplace_back(std::move(worker));
        }

        std::vector<std::uintmax_t> worker_results(workers.size(), 0ULL);

        // Wait for workers to finish and save results
        std::transform(workers.begin(), workers.end(), worker_results.begin(), std::mem_fn(&FutureUintmax::get));

        // Calculate sum of characters
        result = std::accumulate(worker_results.cbegin(), worker_results.cend(), 0ULL);
    }

    std::cout << result << std::endl;

    return EXIT_SUCCESS;
}

// DEFINITIONS

namespace po = boost::program_options;

std::mutex mutex;

template <typename InputIt, typename T>
std::uintmax_t count_if_n(InputIt first, InputIt last, std::uintmax_t n, T const& value) {
    std::uintmax_t result = 0;
    std::uintmax_t i = 0;

    for (; first != last && i < n; ++first, ++i) {
        if (*first == value) ++result;
    }

    return result;
}

std::function<uintmax_t()> createWorker(std::uintmax_t start_pos, std::uintmax_t chunk_size, Options options) {
    return [start_pos, chunk_size, file_path = options.file_path, c = options.character]() {
        std::ifstream fin{file_path};
        fin.unsetf(std::ios_base::skipws);
        fin.seekg(start_pos);

        return count_if_n(std::istream_iterator<char>(fin), std::istream_iterator<char>(), chunk_size, c);
    };
}

void exitWithError(std::string const& error_message, po::options_description const& desc) {
    std::cerr << "Error: " << error_message << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << desc << std::endl;
    exit(EXIT_FAILURE);
}

Options parseArgumentOptions(int argc, char** argv) {
    Options result;

    po::options_description desc("Options");

    // clang-format off
    desc.add_options()
        ("help", "Help message")("character,c", po::value<char>(&result.character),"Character which we count")
        ("input-file,f", po::value<std::string>(&result.file_path), "Path to an input file");
    // clang-format on

    try {
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            exit(EXIT_SUCCESS);
        }

        if (!vm.count("character")) {
            exitWithError("Character not provided", desc);
        }

        if (!vm.count("input-file")) {
            exitWithError("Input file path not provided", desc);
        }

        if (!std::filesystem::exists(result.file_path)) {
            auto const msg = (boost::format("Input file \"%1%\" doesn't exist") % result.file_path).str();
            exitWithError(msg, desc);
        }

        if (!std::filesystem::is_regular_file(result.file_path)) {
            auto const msg = (boost::format("\"%1%\" is not a regular file") % result.file_path).str();
            exitWithError(msg, desc);
        }

        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << desc << std::endl;
        exit(EXIT_FAILURE);
    }
}
