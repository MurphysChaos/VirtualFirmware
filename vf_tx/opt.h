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
	Copyright (C) 2011 Joel Murphy, All rights reserved

	The code in this file is licensed under GPLv2. This license does not 
	extend to any other files bundlded with this file.
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
#define CONNECT_TIMEOUT 0 //10
#define DBGLVL 3 //0

/*
 * A structure for handling options.
 */

struct options {
    char mcastip[16];  // IP only, URL not allowed
    char mcastport[6];
    uint32_t magicnum; // Magic number for broadcast packets
    char tcpport[6];
    int timeout;
    int dbglvl;
};

extern struct options OPT;

void ReadOptions(const char *filename);
#endif
