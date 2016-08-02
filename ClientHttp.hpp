#ifndef CLIENT_HTTP_HPP_
#define CLIENT_HTTP_HPP_

#include <string>

class ParseParams;
class ClientSocket;

class ClientHttp {
public:
  ClientHttp();
  ~ClientHttp();
  int run(int argc, char *argv[]);
  int bench();
  void benchCore(const std::string &host, int port, const std::string &request);

private:
  ParseParams *m_parse_params;
  ClientSocket *m_client_socket;
  int m_pipe[2];
  int m_failed;
  int m_speed;
  int m_bytes;
};
#endif
