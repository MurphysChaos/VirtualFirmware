/*
    dbg -- Functions for console output based on an error threshold.
*/

/*
    Copyright (C) 2011 Jacob Keller, Joel Murphy, All rights reserved

    The code in this file is licensed under GPL v2. This license does not 
    extend to any file bundled with this one.
*/

#include "dbg.h"
#include "opt.h"

void fdbg(FILE * f, int level, const char *msg, ...)
{
    if (OPT.dbglvl >= level) {
        va_list args;
        va_start(args, msg);
        vfprintf(f, msg, args);
        va_end(args);
    }
}

void dbg(int level, const char *msg, ...)
{
    if (OPT.dbglvl >= level) {
        va_list args;
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        va_end(args);
    }
}
