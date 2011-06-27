#include "sock_error.h"

const char* sock_error() {
  int errval = errno;
#ifdef _WIN32
  return wsa_strerror(WSAGetLastError());
#else
  return strerror(errval);
#endif
}
