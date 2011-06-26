#ifndef _SOCK_ERROR_H
#define _SOCK_ERROR_H

#ifdef _WIN32
#include "wsa_strerror.h"
#else
#include <errno.h>
#endif

#ifdef _cplusplus
extern "C" {
#endif

const char* sock_error();

#ifdef _cplusplus
}
#endif

#endif
