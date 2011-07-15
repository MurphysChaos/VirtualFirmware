/* wsa_strerror -- Adds functionality similar to strerror to Windows 
 *   Sockets, allowing for human-readable errors.
 *
 * Copyright (C) 2011 Joel Murphy, All rights reserved
 *
 * The code in this file is licensed under GPL v2. This license does not 
 * extend to any file bundled with this one.
 * * * */

#ifndef _WSA_STRERROR_H
#define _WSA_STRERROR_H
#ifdef _WIN32

#include <WinSock2.h>

#ifdef _cplusplus
extern "C" {
#endif

const char *wsa_strerror(int e);

#ifdef _cplusplus
}
#endif

#endif
#endif
