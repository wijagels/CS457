#include "mimedb.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/none.hpp>
#include <boost/token_functions.hpp>
#include <boost/token_iterator.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

MimeDb::MimeDb(const std::string &path, std::string default_type)
    : d_default{std::move(default_type)} {
  std::ifstream file{path};
  std::string line;
  const boost::char_separator<char> sep{" \t"};
  while (std::getline(file, line)) {
    boost::tokenizer<boost::char_separator<char>> tokens{line, sep};
    if (boost::algorithm::starts_with(line, "#")) continue;
    if (tokens.begin() != tokens.end()) continue;
    std::string mime_type = *tokens.begin();
    d_types.insert(mime_type);
    for (auto t = ++tokens.begin(); t != tokens.end(); ++t) {
      d_ext_to_type.insert({*t, mime_type});
    }
  }
}

std::string MimeDb::mime_of_ext(const std::string &extension) {
  auto it = d_ext_to_type.find(extension);
  if (it == d_ext_to_type.end()) {
    return d_default;
  }
  return it->second;
}
