#ifndef PARSEPARAMS_HPP_
#define PARSEPARAMS_HPP_

#include <getopt.h>
#include <string>

class ParseParams {
public:
  ParseParams()
      : m_is_force(false), m_is_force_load(false), m_is_proxy(false),
        m_port(80), m_benchtime(30), m_clients_count(1), m_http_version(http11),
        m_http_method(get) {}

  void parse(int argc, char *argv[]);
  void buildRequest();
  void displayInfo();

  std::string &getRequest() { return m_request; }
  std::string &getHost() { return m_host; }
  int getPort() { return m_port; }
  int getBenchTime() { return m_benchtime; }

  int getClientNum() { return m_clients_count; }
  bool isZeroClient() { return 0 == m_clients_count; }
  void subOneClient() { --m_clients_count; }

  bool isForce() { return m_is_force; }
  bool isForceLoad() { return m_is_force_load; }
  bool isProxy() { return m_is_proxy; }
  bool isGreateThanHtpp10() { return m_http_version > http10; }
  bool isHttp09() { return m_http_version == http09; }
  static void usage();

private:
  ParseParams(const ParseParams &) {}
  ParseParams &operator=(const ParseParams &) {}

private:
  std::string m_url;
  std::string m_host;
  std::string m_request;

  bool m_is_force;
  bool m_is_force_load;
  bool m_is_proxy;
  int m_port;
  int m_benchtime;
  int m_clients_count;

  enum http_version { http09, http10, http11 };
  http_version m_http_version;

  enum http_method { get, head, options };
  http_method m_http_method;

  static const std::string version;
  static const std::string http_begin_flage;
  static const std::string http_end_flage;
  static const std::string http_port_flage;
  static const int max_size;
  static const option long_options[15];
};

#endif
