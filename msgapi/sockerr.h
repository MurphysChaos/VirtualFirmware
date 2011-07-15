/* sock_error -- Simplifies log output for string-based errors.
 *
 * Copyright (C) 2011 Jacob Keller, All rights reserved
 * The code in this file is licensed under GPL v2. This license does not 
 * extend to any file bundled with this one.
 */

#ifndef _SOCKERR_H
#define _SOCKERR_H

#ifdef _cplusplus
extern "C" {
#endif

#include <errno.h>
#include <string.h>
#include "wsa_strerror.h"

void set_error(int err);
const char* sock_error(void);

#ifdef _cplusplus
}
#endif

#endif
