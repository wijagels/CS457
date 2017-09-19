#ifndef MIMEDB_HPP_
#define MIMEDB_HPP_
#include <string>
#include <unordered_map>

/**
 * Essentially a wrapper around an unordered_map.
 * Reads in data from a mime.types file, and does not allow further modification after construction.
 */
class MimeDb {
 public:
  /**
   * Constructor, takes path to mime.types file and a default type to fall back to.
   * MT-Safety: Safe
   */
  MimeDb(const std::string &mime_file = "/etc/mime.types",
         std::string default_type = "application/octet-stream");

  /**
   * Lookup the extension in the database, if not found, return the default type provided at
   * construction time.
   * MT-Safety: Safe
   */
  std::string mime_of_ext(const std::string &extension) const;

 private:
  std::unordered_map<std::string, std::string> d_ext_to_type;
  std::string d_default;
};

#endif
