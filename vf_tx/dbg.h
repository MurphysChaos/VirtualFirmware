#ifndef DBG_H
#define DBG_H

#include <stdio.h>
#include <stdarg.h>

#define DBG_STATUS 1
#define DBG_ERROR  2
#define DBG_ALL    3

void dbg(int level, const char *msg, ...);

#endif
