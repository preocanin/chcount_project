#include <boost/format.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <numeric>
#include <thread>

using FutureUint64 = std::future<std::uint64_t>;

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
std::uint64_t count_if_n(InputIt first, InputIt last, std::uint64_t n,
                         T const& value);

int main(int argc, char** argv) {
    auto const options = parseArgumentOptions(argc, argv);

    auto const file_size = std::filesystem::file_size(options.file_path);

    auto threads_count = std::thread::hardware_concurrency();
    auto thread_chunk_size = file_size / threads_count;

    if (thread_chunk_size == 0) {
        threads_count = 1;
        thread_chunk_size = file_size;
    }

    std::vector<FutureUint64> workers;

    for (int i = 0; i < threads_count; ++i) {
        auto worker = std::async(
            std::launch::async,
            [seekg_pos = i * thread_chunk_size, options, thread_chunk_size]() {
                std::ifstream fin(options.file_path);
                fin.seekg(seekg_pos);

                return count_if_n(std::istream_iterator<char>(fin),
                                  std::istream_iterator<char>(),
                                  thread_chunk_size - 1, options.character);
            });

        workers.emplace_back(std::move(worker));
    }

    std::vector<std::uint64_t> worker_results(workers.size(), 0ULL);

    // Wait for workers to finish and save results
    std::transform(workers.begin(), workers.end(), worker_results.begin(),
                   std::mem_fn(&FutureUint64::get));

    // Calculate sum of characters
    std::cout << std::accumulate(worker_results.cbegin(), worker_results.cend(),
                                 0ULL)
              << std::endl;

    return 0;
}

// DEFINITIONS

namespace po = boost::program_options;

template <typename InputIt, typename T>
std::uint64_t count_if_n(InputIt first, InputIt last, std::uint64_t n,
                         T const& value) {
    unsigned long long result = 0;
    unsigned long long i = 0;

    for (; first != last && i < n; ++first, ++i) {
        if (*first == value) ++result;
    }

    return result;
}

void exitWithError(std::string const& error_message,
                   po::options_description const& desc) {
    std::cerr << "Error: " << error_message << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << desc << std::endl;
    exit(EXIT_FAILURE);
}

Options parseArgumentOptions(int argc, char** argv) {
    Options result;

    po::options_description desc("Options");

    desc.add_options()("help", "Help message")(
        "character,c", po::value<char>(&result.character),
        "Character which we count")("input-file,f",
                                    po::value<std::string>(&result.file_path),
                                    "Path to an input file");

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
            auto const msg =
                (boost::format("Input file \"%1%\" doesn't exist") %
                 result.file_path)
                    .str();
            exitWithError(msg, desc);
        }

        if (!std::filesystem::is_regular_file(result.file_path)) {
            auto const msg = (boost::format("\"%1%\" is not a regular file") %
                              result.file_path)
                                 .str();
            exitWithError(msg, desc);
        }

        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << desc << std::endl;
        exit(EXIT_FAILURE);
    }
}
