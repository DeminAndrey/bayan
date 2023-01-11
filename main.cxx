#include "FileScanner.h"

#include <iostream>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

static const char* HELP =        "help";
static const char* BLOCK_SIZE =  "block";
static const char* ALGORITHM =   "hash";
static const char* MIN_SIZE =    "min_size";
static const char* DIR_TO_SCAN = "to_scan";
static const char* DIR_NO_SCAN = "no_scan";
static const char* MASKS =       "masks";
static const char* RECURSIVE =   "recursive";
static const char* CRC16 =       "crc16";
static const char* CRC32 =       "crc32";


int main(int argc, const char *argv[]) {
  try {
    bool isRecursive = false;
    size_t minimumSize = 1;
    size_t blockSize = 5;
    HashType type = HashType::CRC32;
    std::vector<boost::regex> masks;
    std::vector<std::string> dirToScan;
    std::vector<std::string> dirNoScan;

    po::options_description opt_desc("Allowed options");
    opt_desc.add_options()
        (HELP,                                                                          "Print this message")
        (BLOCK_SIZE,   po::value<std::size_t>()->required()->default_value(5),          "Block size (in kylobytes) used to compare files (at least 1)")
        (ALGORITHM,    po::value<std::string>()->required()->default_value(CRC32),      "Hash algorithm used to compare byte blocks, one of 'crc32', 'crc16'")
        (MIN_SIZE,     po::value<std::size_t>()->required()->default_value(1),          "Minimum file size to compare")
        (DIR_TO_SCAN,  po::value<std::vector<std::string>>()->required()->multitoken(), "Directories to search duplicates into")
        (DIR_NO_SCAN,  po::value<std::vector<std::string>>()->multitoken(),             "Direcroties to exclude from search")
        (MASKS,        po::value<std::vector<std::string>>()->multitoken(),             "Include only files corresponding to these masks in search")
        (RECURSIVE,    po::bool_switch(&isRecursive),                                   "Use this option to enable recursive subdirectory scanning");

    po::variables_map var_map;
    try {
      auto parsed = po::command_line_parser(argc, argv)
          .options(opt_desc)
          .run();
      po::store(parsed, var_map);
      if (var_map.count(HELP) != 0) {
        std::cout << opt_desc << "\n";
        return 0;
      }
      po::notify(var_map);
    }
    catch (const po::error& err) {
      std::cerr << "Error while parsing command-line arguments: "
                << err.what() << "\nPlease use --help to see help message\n";
      return 1;
    }

    blockSize = var_map[BLOCK_SIZE].as<std::size_t>();
    if (blockSize == 0) {
      std::cerr << "Block size must be at least 1\n";
      return 1;
    }
    minimumSize = var_map[MIN_SIZE].as<std::size_t>();
    dirToScan = var_map[DIR_TO_SCAN].as<std::vector<std::string>>();
    dirNoScan = (var_map.count(DIR_NO_SCAN) == 0) ? std::vector<std::string>()
                                                    : var_map[DIR_NO_SCAN].as<std::vector<std::string>>();

    auto mask_strings = (var_map.count(MASKS) == 0) ? std::vector<std::string>()
                                                    : var_map[MASKS].as<std::vector<std::string>>();
    for (const auto& mask_string : mask_strings) {
      masks.push_back(boost::regex(mask_string, boost::regex_constants::ECMAScript));
    }

    auto hash = var_map[ALGORITHM].as<std::string>();
    if (hash == CRC16) {
      type = HashType::CRC16;
    }
    else if (hash == CRC32) {
      type = HashType::CRC32;
    }
    else {
      throw std::invalid_argument(std::string("Incorrect hash algorithm ")
                                  + hash + ". Use --help options for more details");
    }

    FileScanner scanner(
          dirToScan, dirNoScan, masks, type, blockSize, minimumSize);
    auto duplicate = scanner.scan(isRecursive);

    for (const auto &group : duplicate) {
      std::cout << group.first << "  ";
      for (const auto &name : group.second) {
        std::cout << name << "  ";
      }
      std::cout << std::endl;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
