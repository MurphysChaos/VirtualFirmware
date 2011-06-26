#ifndef _WSA_STRERROR_H
#define _WSA_STRERROR_H

#include <WinSock2.h>

#ifdef _cplusplus
extern "C" {
#endif

const char *wsa_strerror(int e);

#ifdef _cplusplus
}
#endif

#endif
