#include "ClientSocket.hpp"

#include <arpa/inet.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/types.h>
const int ClientSocket::recvMaxLength = 1500;

bool ClientSocket::init(const std::string &host, int port) {
  sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;

  unsigned long inaddr = inet_addr(host.c_str());
  if (inaddr != INADDR_NONE) {
    memcpy(&addr.sin_addr, &inaddr, sizeof(inaddr));
  } else {
    hostent *hp;
    hp = gethostbyname(host.c_str());

    if (NULL == hp) {
      return false;
    } else {
      memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
    }
  }
  addr.sin_port = htons(port);

  m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket_fd < 0) {
    return false;
  }
  if (connect(m_socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    return false;
  }
  return true;
}

bool ClientSocket::sendBuff(const std::string &request) {
  int wlen = static_cast<int>(request.size());
  if (wlen != write(m_socket_fd, request.c_str(), wlen)) {
    return false;
  } else {
    return true;
  }
}

int ClientSocket::recvBuff() {
  char tmp_recv_buff[ClientSocket::recvMaxLength];
  int rlen = 0, bytes = 0;
  while (true) {
    rlen = read(m_socket_fd, tmp_recv_buff, ClientSocket::recvMaxLength);
    if (rlen > 0) {
      bytes += rlen;
    } else if (rlen == 0) {
      return bytes;
    } else {
      return rlen;
    }
  }
}
