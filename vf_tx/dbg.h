#ifndef DBG_H
#define DBG_H

#include <stdio.h>
#include <stdarg.h>

#define DBG_STATUS 1
#define DBG_ERROR  2
#define DBG_WARN   3
#define DBG_VERB   4

void fdbg(FILE *f, int level, const char *msg, ...);
void dbg(int level, const char *msg, ...);

#endif
