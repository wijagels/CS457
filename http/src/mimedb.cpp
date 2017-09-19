#include "mimedb.hpp"
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <regex>

MimeDb::MimeDb(const std::string &mime_file, std::string default_type)
    : d_default{std::move(default_type)} {
  std::ifstream file{mime_file};
  std::string line;
  const std::regex sep{"\\s+"};
  const std::regex comment{"^\\s*#.*"};
  const std::sregex_token_iterator tok_end;
  while (std::getline(file, line)) {
    if (std::regex_match(line, comment)) continue;
    std::sregex_token_iterator tokenizer{line.begin(), line.end(), sep, -1};
    if (tokenizer == std::sregex_token_iterator{}) continue;
    std::string mime_type = *tokenizer;
    for (; tokenizer != tok_end; ++tokenizer) {
      d_ext_to_type.insert({*tokenizer, mime_type});
    }
  }
}

std::string MimeDb::mime_of_ext(const std::string &extension) const {
  auto it = d_ext_to_type.find(extension);
  if (it == d_ext_to_type.end()) {
    return d_default;
  }
  return it->second;
}
