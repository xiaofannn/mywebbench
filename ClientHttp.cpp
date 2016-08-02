#include "ClientHttp.hpp"
#include "ClientSocket.hpp"
#include "ParseParams.hpp"

#include <cstdio>
#include <iostream>
#include <signal.h>
#include <unistd.h>

ClientHttp::ClientHttp() : m_failed(0), m_speed(0), m_bytes(0) {
  m_parse_params = new ParseParams();
  m_client_socket = new ClientSocket();
}

ClientHttp::~ClientHttp() {
  if (NULL != m_parse_params) {
    delete m_parse_params;
    m_parse_params = NULL;
  }
  if (NULL != m_client_socket) {
    delete m_client_socket;
    m_client_socket = NULL;
  }
}

int ClientHttp::run(int argc, char **argv) {
  m_parse_params->parse(argc, argv);
  return bench();
}

int ClientHttp::bench() {
  pid_t pid = 0;
  FILE *f;
  if (!m_client_socket->init(m_parse_params->getHost(),
                             m_parse_params->getPort())) {
    std::cerr << "Connect to server failed. Aborting benchmark" << std::endl;
    return 1;
  }
  m_client_socket->closeSocket();
  if (pipe(m_pipe)) {
    perror("pipe failed");
    return 3;
  }

  int i = 0;
  for (; i < m_parse_params->getClientNum(); ++i) {
    pid = fork();
    if (pid <= static_cast<pid_t>(0)) {
      sleep(1);
      break;
    }
  }

  if (pid < static_cast<pid_t>(0)) {
    std::cerr << "problems foring worker no. " << i << std::endl;
    perror("fork failed");
    return 3;
  }
  // children progress socket request
  if (pid == static_cast<pid_t>(0)) {
    benchCore(m_parse_params->getHost(), m_parse_params->getPort(),
              m_parse_params->getRequest());

    f = fdopen(m_pipe[1], "w");
    if (NULL == f) {
      perror("open pipe for writing failed.");
      return 3;
    }

    fprintf(f, "%d %d %d\n", m_failed, m_speed, m_bytes);
    fclose(f);
    return 0;
  } else {
    // parent  collect information
    f = fdopen(m_pipe[0], "r");
    if (NULL == f) {
      perror("open pipe for reading failed.");
      return 3;
    }
    setvbuf(f, NULL, _IONBF, 0);

    int tmp_failed = 0, tmp_speed = 0, tmp_bytes = 0;
    while (true) {
      pid = fscanf(f, "%d %d %d", &tmp_failed, &tmp_speed, &tmp_bytes);
      if (pid < 3) {
        std::cerr << "some of our childrens died." << std::endl;
        break;
      }

      m_failed += tmp_failed;
      m_speed += tmp_speed;
      m_bytes += tmp_bytes;
      m_parse_params->subOneClient();
      if (m_parse_params->isZeroClient())
        break;
    }

    fclose(f);
    std::cout << "Speed = " << static_cast<int>((m_failed + m_speed) /
                                                m_parse_params->getBenchTime())
              << " pages/sec " << m_bytes / m_parse_params->getBenchTime()
              << " bytes/sec. " << std::endl;
    std::cout << "Request: " << m_speed << " susceed " << m_failed
              << " failed. " << std::endl;
  }
  return i;
}

static volatile bool timerexpired = false;
static void alarmHandler(int signal) { timerexpired = true; }

void ClientHttp::benchCore(const std::string &host, int port,
                           const std::string &request) {
  struct sigaction sa;

  sa.sa_handler = alarmHandler;
  sa.sa_flags = 0;
  if (sigaction(SIGALRM, &sa, NULL))
    exit(3);
  alarm(m_parse_params->getBenchTime());

nexttry:
  while (true) {
    if (timerexpired) {
      if (m_failed > 0) {
        --m_failed;
      }
      return;
    }
    if (!m_client_socket->init(host, port)) {
      ++m_failed;
      continue;
    }
    if (!m_client_socket->sendBuff(request)) {
      ++m_failed;
      continue;
    }
    if (m_parse_params->isHttp09()) {
      if (!m_client_socket->shutDownSocket()) {
        ++m_failed;
        continue;
      }
    }
    if (!m_parse_params->isForce()) {
      while (true) {
        if (timerexpired) {
          break;
        }
        int rlen = m_client_socket->recvBuff();
        if (rlen < 0) {
          ++m_failed;
          m_client_socket->closeSocket();
          goto nexttry;
        } else {
          m_bytes += rlen;
          break;
        }
      }
    }
    if (!m_client_socket->closeSocket()) {
      ++m_failed;
      continue;
    }
    ++m_speed;
  }
}
