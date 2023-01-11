#include "FileScanner.h"

#include <boost/crc.hpp>
#include <boost/iostreams/device/file.hpp>

FileScanner::FileScanner(const std::vector<std::string> &pathsToScan,
                         const std::vector<std::string> &pathsNoScan,
                         const std::vector<boost::regex> &masks,
                         HashType type,
                         size_t blockSize,
                         size_t minimumSize)
  : m_minimumSize(minimumSize),
    m_blockSize(blockSize),
    m_type(type),
    m_masks(masks),
    m_pathsToScan(pathsToScan),
    m_pathsNoScan(pathsNoScan) {
}

boost::container::map<std::string, boost::vector<std::string>>
FileScanner::scan(bool isRecursive) {
  for (const auto &pathToScan : m_pathsToScan) {
    if (!boost::filesystem::exists(pathToScan)) {
      continue;
    }
    if (!isFind(m_pathsNoScan, pathToScan)) {
      isRecursive ? recursive_scan(pathToScan) : scan(pathToScan);
    }
  }
  find_duplicate(m_files_to_scan);
  return m_duplicates;
}

void FileScanner::scan(const std::string &path) {
  for (const auto &dir_entry :
       boost::filesystem::directory_iterator(path)) {
    if (dir_entry.status().type() == boost::filesystem::regular_file) {
      try_to_collect(dir_entry);
    }
  }
}

void FileScanner::recursive_scan(const std::string &path) {
  for (const auto &dir_entry :
       boost::filesystem::recursive_directory_iterator(path)) {
    if (dir_entry.status().type() == boost::filesystem::regular_file) {
      try_to_collect(dir_entry);
    }
  }
}

void FileScanner::try_to_collect(
    const boost::filesystem::directory_entry &dir) {
  if (boost::filesystem::file_size(dir.path()) < m_minimumSize) {
    return;
  }
  auto fileName = dir.path().string();
  if (!is_exactly(fileName)) {
    return;
  }
  m_files_to_scan.push_back(fileName);
}

bool FileScanner::is_exactly(const std::string &file) const {
  if (m_masks.empty()) {
    return true;
  }
  for (const auto &re : m_masks) {
    if (boost::regex_match(file.c_str(), re)) {
      return true;
    }
  }

  return false;
}

void FileScanner::find_duplicate(boost::vector<std::string> &files) {
  if (files.size() <= 1) {
    return;
  }
  auto lhs = files.back();
  files.pop_back();
  for (const auto &rhs : files) {
    if (is_equal_file(lhs, rhs)) {
      m_duplicates[lhs].push_back(rhs);
      files.erase(std::remove(files.begin(), files.end(), rhs));
    }
  }
  find_duplicate(files);
}

bool FileScanner::is_equal_file(const std::string &lhs,
                                const std::string &rhs) {
  if (lhs.empty() || rhs.empty()) {
    return false;
  }
  std::ifstream l_ifstr(lhs, std::ios::in);
  std::ifstream r_ifstr(rhs, std::ios::in);
  if (!l_ifstr.is_open() || !r_ifstr.is_open()) {
    return false;
  }

  bool equal = false;
  for (size_t pos = 0;
        (l_ifstr.seekg(pos) && r_ifstr.seekg(pos));
          pos += m_blockSize) {
    std::string l_block(m_blockSize, '\0');
    std::string r_block(m_blockSize, '\0');

    if(l_ifstr.read(&l_block[0], m_blockSize) &&
       r_ifstr.read(&r_block[0], m_blockSize)) {
      equal = is_equal_blocks(l_block, r_block);
      if (!equal) {
        return false;
      }
    }
  }

  return equal;
}

bool FileScanner::is_equal_blocks(const std::string &lhs,
                                  const std::string &rhs) const {
  return hash(lhs) == hash(rhs);
}

size_t FileScanner::hash(const std::string &str) const {
  return (m_type == HashType::CRC16) ? crc16(str) : crc32(str);
}

size_t FileScanner::crc16(const std::string &str) const {
  boost::crc_16_type result;
  result.process_bytes(str.data(), str.size());
  return result.checksum();
}

size_t FileScanner::crc32(const std::string &str) const {
  boost::crc_32_type result;
  result.process_bytes(str.data(), str.size());
  return result.checksum();
}
