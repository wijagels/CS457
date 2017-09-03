extern "C" {
#include <sys/socket.h>
#include <netdb.h>
}
#include <cstdio>

static constexpr size_t BUF_SIZE = 500;

int main() {
  auto tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

  ssize_t nread;
  char buf[BUF_SIZE];
  socklen_t peer_addr_len;
  int s;
  sockaddr peer_addr;

  for (;;) {
    peer_addr_len = sizeof(struct sockaddr_storage);
    nread = recvfrom(tcp_socket, buf, BUF_SIZE, 0, &peer_addr, &peer_addr_len);
    if (nread == -1) continue; /* Ignore failed request */

    char host[NI_MAXHOST], service[NI_MAXSERV];

    s = getnameinfo(&peer_addr, peer_addr_len, host, NI_MAXHOST, service,
                    NI_MAXSERV, NI_NUMERICSERV);
    if (s == 0)
      printf("Received %zd bytes from %s:%s\n", nread, host, service);
    else
      fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

    if (sendto(tcp_socket, buf, static_cast<size_t>(nread), 0, &peer_addr, peer_addr_len) != nread)
      fprintf(stderr, "Error sending response\n");
  }
}
