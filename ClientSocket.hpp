#ifndef CLIENTSOCKET_HPP_
#define CLIENTSOCKET_HPP_

#include <string>
#include <sys/socket.h>
#include <unistd.h>

class ClientSocket {
public:
  ClientSocket() : m_socket_fd(-1) {}
  ~ClientSocket() {}
  bool init(const std::string &host, int port);
  bool sendBuff(const std::string &request);
  int recvBuff();
  bool closeSocket() { return close(m_socket_fd) == 0 ? true : false; }
  bool shutDownSocket() { return shutdown(m_socket_fd, 1) == 0 ? true : false; }

private:
  ClientSocket(const ClientSocket &) {}
  ClientSocket &operator=(const ClientSocket &) {}

public:
  const static int recvMaxLength;

private:
  int m_socket_fd;
};
#endif
