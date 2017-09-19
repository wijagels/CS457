#ifndef FILE_HPP_
#define FILE_HPP_
extern "C" {
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
}
#include <string>

/*
 * RAII-style file descriptor.
 * Move only.
 */
class File {
 public:
  File() {}

  ~File() { close(); }

  File(const std::string &path, int flags) : d_fd{open(path.c_str(), flags)} {}

  File(const File &) = delete;

  File(File &&other) : d_fd{other.d_fd} { other.d_fd = -1; }

  File &operator=(const File &) = delete;

  File &operator=(File &&other) {
    close();
    d_fd = other.d_fd;
    other.d_fd = -1;
    return *this;
  }

  explicit operator int() {
    return d_fd;
  }

  int fd() {
    return d_fd;
  }

  void close() {
    if (d_fd != -1) {
      ::close(d_fd);
      d_fd = -1;
    }
  }

 private:
  int d_fd = -1;
};

#endif
