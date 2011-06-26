#include "sock_error.h"

const char* sock_error() {
#ifdef _WIN32
  return wsa_strerror(WSAGetLastError());
#else
  return strerror(errno);
#endif
}
