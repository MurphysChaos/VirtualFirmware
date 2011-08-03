/*
    opt -- A simple options file library.

    Reads options from a specified file into a global OPT struct.
    This is a non-robust library that depends on a text file with 
    lines of the form:
    
    <token>=<value>

    Any unrecognized line, and any text to the right of a '#' symbol
    is ignored.
*/

/*
    Copyright (c) 2011 Joel Murphy, Jacob Keller

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

#ifndef OPT_H_
#define OPT_H_
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define OPTRC "optrc"
#define OPTBUFLEN 1024

/* Default option values */
#define MULTICAST_ADDR "234.1.1.10"
#define MULTICAST_PORT "9751"
#define MAGIC_NUMBER 0xe1000
#define TCP_PORT "9752"
#define CONNECT_TIMEOUT 0       //10
#define DBGLVL 3                //0

/*
 * A structure for handling options.
 */

struct options {
    char mcastip[16];           // IP only, URL not allowed
    char mcastport[6];
    uint32_t magicnum;          // Magic number for broadcast packets
    char tcpport[6];
    int timeout;
    int dbglvl;
    int set;
};

extern struct options OPT;

void ReadOptions(const char *filename);
#endif
