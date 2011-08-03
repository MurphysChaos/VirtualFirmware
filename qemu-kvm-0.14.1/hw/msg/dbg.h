/*
    dbg -- Functions for console output based on an error threshold.
*/

/*
    Copyright (c) 2011 Jacob Keller, Joel Murphy

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in 
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
*/

#ifndef DBG_H
#define DBG_H

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "opt.h"

#define DBG_STATUS 1
#define DBG_ERROR  2
#define DBG_WARN   3
#define DBG_VERB   4

#define dbg(level, msg, ...) \
    fdbg(stderr, level, msg, ##__VA_ARGS__);

/* Output debug information
 */
void fdbg(FILE * f, int level, const char *msg, ...);


#endif
