/*
    dbg -- Functions for console output based on an error threshold.
*/

/*
    Copyright (C) 2011 Jacob Keller, Joel Murphy, All rights reserved

    The code in this file is licensed under GPL v2. This license does not 
    extend to any file bundled with this one.
*/

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
