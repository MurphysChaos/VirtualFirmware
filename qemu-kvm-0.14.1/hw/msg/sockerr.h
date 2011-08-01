/* 
    sockerr -- Simplifies log output for string-based errors.
*/

/*
    Copyright (C) 2011 Jacob Keller, All rights reserved

    The code in this file is licensed under GPL v2. This license does not 
    extend to any file bundled with this one.
*/

#ifndef _SOCKERR_H
#define _SOCKERR_H

#include <errno.h>
#include <string.h>

#ifdef _WIN32
#include "wsa_strerror.h"
#endif

void set_error(int err);
const char *sock_error(void);

#endif
