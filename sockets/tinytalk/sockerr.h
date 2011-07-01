/* sock_error -- Simplifies log output for string-based errors.
 *
 * Copyright (C) 2011 Jacob Keller, All rights reserved
 * The code in this file is licensed under GPL v2. This license does not 
 * extend to any file bundled with this one.
 * * * */

#ifndef _SOCK_ERROR_H
#define _SOCK_ERROR_H

#ifdef _WIN32
#include "wsa_strerror.h"
#else
#include <errno.h>
#include <string.h>
#endif

#ifdef _cplusplus
extern "C" {
#endif

const char* sock_error();

#ifdef _cplusplus
}
#endif

#endif
