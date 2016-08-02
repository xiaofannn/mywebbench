#include "ParseParams.hpp"

#include <algorithm>
#include <iostream>

const std::string ParseParams::version("0.1");
const std::string ParseParams::http_begin_flage("http://");
const std::string ParseParams::http_end_flage("/");
const std::string ParseParams::http_port_flage(":");
const int ParseParams::max_size(1500);

void ParseParams::usage() {
  std::cout
      << "webbench [option]... URL\n"
         "  -f|--force               Don't wait for reply from server.\n"
         "  -r|--reload              Send reload request - Pragma: no-cache.\n"
         "  -t|--time <sec>          Run benchmark for <sec> seconds. Default "
         "30.\n"
         "  -p|--proxy <server:port> Use proxy server for request.\n"
         "  -c|--clients <n>         Run <n> HTTP clients at once. Default "
         "one.\n"
         "  -9|--http09              Use HTTP/0.9 style requests.\n"
         "  -0|--http10              Use HTTP/1.0 protocol.\n"
         "  -1|--http11              Use HTTP/1.1 protocol.\n"
         "  --get                    Use GET request method.\n"
         "  --head                   Use HEAD request method.\n"
         "  --options                Use OPTIONS request method.\n"
         "  -?|-h|--help             This information.\n"
         "  -V|--version             Display program version.\n"
      << std::endl;
};

const option ParseParams::long_options[15] = {
    {"force", no_argument, NULL, 'f'},
    {"reload", no_argument, NULL, 'r'},
    {"time", required_argument, NULL, 't'},
    {"help", no_argument, NULL, '?'},
    {"http09", no_argument, NULL, '0'},
    {"http10", no_argument, NULL, '1'},
    {"http11", no_argument, NULL, '2'},
    {"get", no_argument, NULL, 'g'},
    {"head", no_argument, NULL, 'h'},
    {"options", no_argument, NULL, 'o'},
    {"trace", no_argument, NULL, 't'},
    {"version", no_argument, NULL, 'V'},
    {"proxy", required_argument, NULL, 'p'},
    {"clients", required_argument, NULL, 'c'},
    {NULL, 0, NULL, 0}};

void ParseParams::parse(int argc, char *argv[]) {
  int opt = 0;
  int options_index = 0;
  while ((opt = getopt_long(argc, argv, "901frt:p:c:?ghotV", long_options,
                            &options_index)) != EOF) {
    switch (opt) {
    case '9':
      m_http_version = http09;
      break;
    case '0':
      m_http_version = http10;
      break;
    case '1':
      m_http_version = http11;
      break;
    case 'f':
      m_is_force = true;
      break;
    case 'r':
      m_is_force_load = true;
      break;
    case 't':
      m_benchtime = atoi(optarg);
      break;
    case 'p': {
      char *tmp = strrchr(optarg, ':');
      if (NULL == tmp) {
        ParseParams::usage();
        exit(2);
      } else if (tmp == optarg) {
        std::cerr << "Error in option --proxy " << optarg
                  << " Missing hostname." << std::endl;
        exit(2);
      } else if (tmp == optarg + strlen(optarg) - 1) {
        std::cerr << "Error in option --proxy " << optarg
                  << " Port number is missing." << std::endl;
        exit(2);
      }

      *tmp = '\0';
      m_host.assign(optarg);
      m_port = atoi(tmp + 1);
      m_is_proxy = true;
    } break;
    case 'c':
      m_clients_count = atoi(optarg);
      break;
    case '?':
      ParseParams::usage();
      exit(2);
      break;
    case 'g':
      break;
    case 'h':
      m_http_method = head;
      break;
    case 'o':
      m_http_method = options;
      break;
    case 'V':
      std::cout << ParseParams::version << std::endl;
      break;
    }
  }

  if (optind == argc) {
    std::cerr << "webbench: Missing URL" << std::endl;
    ParseParams::usage();
    exit(2);
  }

  m_url.assign(argv[optind]);
  buildRequest();
  displayInfo();
  std::cout << "Webbench - Simple Web Benchmark " << ParseParams::version
            << std::endl;
  std::cout << "Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software"
            << std::endl;
}

void ParseParams::buildRequest() {
  if (m_is_force_load && m_is_proxy && m_http_version < http10) {
    m_http_version = http10;
  }

  if (m_http_method == head && m_http_version < http10) {
    m_http_version = http10;
  } else if (m_http_method == options && m_http_version < http11) {
    m_http_version = http11;
  }

  switch (m_http_method) {
  case get:
    m_request.assign("GET ");
    break;
  case head:
    m_request.assign("HEAD ");
    break;
  case options:
    m_request.assign("OPTIONS ");
    break;
  }

  if (m_url.find(ParseParams::http_begin_flage) == std::string::npos) {
    std::cerr << m_url << " is not a valid URL" << std::endl;
    exit(2);
  }

  if (m_url.size() > ParseParams::max_size) {
    std::cerr << "URL is too long" << std::endl;
    exit(2);
  }

  if (!m_is_proxy) {
    std::transform(m_url.begin(), m_url.end(), m_url.begin(), ::tolower);
    if (m_url.compare(0, ParseParams::http_begin_flage.size(),
                      ParseParams::http_begin_flage) != 0) {
      std::cerr << "Only HTTP protocol is directly supported, set --proxy "
                   "for others"
                << std::endl;
      exit(2);
    }
  }

  std::string::size_type http_begin_index =
      m_url.find(ParseParams::http_begin_flage) +
      ParseParams::http_begin_flage.size();
  std::string::size_type http_end_index =
      m_url.find(ParseParams::http_end_flage, http_begin_index);

  if (http_end_index == std::string::npos) {
    std::cerr << "Invalid URL syntax - hostname don't ends with '/'."
              << std::endl;
    exit(2);
  }

  if (!m_is_proxy) {
    std::string::size_type port_index =
        m_url.find(ParseParams::http_port_flage, http_begin_index);
    if (port_index != std::string::npos && port_index < http_end_index) {
      m_host.assign(m_url.c_str() + http_begin_index,
                    port_index - http_begin_index);

      std::string port;
      port.assign(m_url.c_str() + port_index + 1,
                  http_end_index - port_index - 1);

      m_port = atoi(port.c_str());
      if (0 == m_port) {
        m_port = 80;
      }
    } else {
      m_host.assign(m_url.c_str() + http_begin_index,
                    http_end_index - http_begin_index);
    }
    m_request.append(m_url.c_str() + http_end_index + 1);
  } else {
    m_request.append(m_url);
  }

  if (m_http_version == http10) {
    m_request.append(" HTTP/1.0");
  } else if (m_http_version == http11) {
    m_request.append(" HTTP/1.1");
  }
  m_request.append("\r\n");

  if (m_http_version > http09) {
    m_request.append("User-Agent: WebBench " + ParseParams::version + "\r\n");
  }

  if (!m_is_proxy) {
    m_request.append("Host: " + m_host + "\r\n");
  }

  if (m_is_force_load && !m_is_proxy) {
    m_request.append("Pragma: no-cache\r\n");
  }

  if (m_http_version > http10) {
    m_request.append("Connection: close\r\n");
  }

  if (m_http_version > http09) {
    m_request.append("\r\n");
  }
}

void ParseParams::displayInfo() {
  std::cout << " Benchmarking" << std::endl;
  switch (m_http_method) {
  case get:
    std::cout << "Get ";
    break;
  case head:
    std::cout << "Head ";
    break;
  case options:
    std::cout << "Options ";
    break;
  }
  std::cout << m_url;

  switch (m_http_version) {
  case http09:
    std::cout << " using HTTP/0.9";
    break;
  case http10:
    std::cout << " using HTTP/1.0";
    break;
  case http11:
    std::cout << " using HTTP/1.1";
    break;
  }
  std::cout << std::endl;

  std::cout << m_clients_count << "Clients ";
  std::cout << "running " << m_benchtime << " sec ";
  if (m_is_force)
    std::cout << ", early close socket";
  if (m_is_force_load)
    std::cout << ", forcing reload";
  std::cout << std::endl;
}
