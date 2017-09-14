#ifndef MIMEDB_HPP_
#define MIMEDB_HPP_
#include <string>
#include <unordered_map>
#include <unordered_set>

class MimeDb {
 public:
  MimeDb(const std::string &path = "/etc/mime.types",
         std::string default_type = "application/octet-stream");

  std::string mime_of_ext(const std::string &extension);

 private:
  std::unordered_map<std::string, std::string> d_ext_to_type;
  std::unordered_set<std::string> d_types;
  std::string d_default;
};

#endif