#pragma once

#include <boost/container/map.hpp>
#include <boost/container/vector.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>

namespace boost {
  using boost::container::map;
  using boost::container::vector;
}

enum class HashType {
  CRC16,
  CRC32
};

/**
 * @brief класс для обнаружения файлов-дубликатов
 */
class FileScanner {
  size_t m_minimumSize = 1;
  size_t m_blockSize = 5;
  HashType m_type = HashType::CRC32;
  std::vector<boost::regex> m_masks;
  std::vector<std::string> m_pathsToScan;
  std::vector<std::string> m_pathsNoScan;

  boost::vector<std::string> m_files_to_scan;
  boost::map<std::string, boost::vector<std::string>> m_duplicates;

public:
  /**
   * @brief конструктор класса
   * @param pathsToScan директории для сканирования
   * @param pathsNoScan директории для исключения из сканирования
   * @param minimumSize минимальный размер файла для сравнения
   * @param maskName маски имен файлов разрешенных для сканирования
   * @param blockSize размер блока, которым производится чтение файлов
   * @param type алгоритм хэширования
   */
  FileScanner(const std::vector<std::string> &pathsToScan,
              const std::vector<std::string> &pathsNoScan,
              const std::vector<boost::regex> &masks,
              HashType type,
              size_t blockSize,
              size_t minimumSize = 1);

  /**
   * @brief ф-я для посика файлов-дубликатов
   * @param isRecursive уровень сканирования, 1 - на все директории,
   * 0 - только указанная, без вложенных
   * @return файлы-дубликаты
   */
  boost::map<std::string, boost::vector<std::string>> scan(
      bool isRecursive = false);

private:
  void scan(const std::string &path);
  void recursive_scan(const std::string &path);

  inline bool isFind(const std::vector<std::string> &source,
                     const std::string &str) const noexcept {
    return std::find(source.cbegin(), source.cend(),
                     str) != source.cend();
  }

  void try_to_collect(const boost::filesystem::directory_entry &dir);
  bool is_exactly(const std::string &file) const;
  void find_duplicate(boost::vector<std::string> &files);
  bool is_equal_file(const std::string &lhs, const std::string &rhs);
  bool is_equal_blocks(const std::string &lhs, const std::string &rhs) const;

  size_t crc16(const std::string &str) const;
  size_t crc32(const std::string &str) const;
  size_t hash(const std::string &str) const;
};
