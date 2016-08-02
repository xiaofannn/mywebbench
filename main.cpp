
#include "ClientHttp.hpp"

int main(int argc, char const *argv[]) {
  ClientHttp http;
  http.run(argc, const_cast<char **>(argv));
  return 0;
}
