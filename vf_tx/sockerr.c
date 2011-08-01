/* 
    sockerr -- Simplifies log output for string-based errors.
*/

/*
    Copyright (C) 2011 Jacob Keller, All rights reserved

    The code in this file is licensed under GPL v2. This license does not 
    extend to any file bundled with this one.
*/

#include "sockerr.h"

/* Set error value
 */
void set_error(int err)
{
#ifdef _WIN32
    WSASetLastError(err);
#else
    errno = err;
#endif
}

/* Return error message
 */
const char *sock_error(void)
{
    int errval = errno;
#ifdef _WIN32
    return wsa_strerror(WSAGetLastError());
#else
    return strerror(errval);
#endif
}
