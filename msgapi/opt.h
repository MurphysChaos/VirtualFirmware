#ifndef OPT_H_
#define OPT_H_
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


/*
 * A structure for handling options.
 */

struct options {
    char mcastip[16];  // IP only, URL not allowed
    char mcastport[6];
    uint32_t magicnum; // Magic number for broadcast packets
    char tcpport[6];
};

extern struct options OPT;

void ReadOptions(const char *filename);
#endif
