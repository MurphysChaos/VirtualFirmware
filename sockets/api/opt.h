#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

const char *OPTRC = "optrc";
#define OPTBUFLEN 1024

/*
 * A structure for handling options.
 */

struct {
    char mcastip[16];  // IP only, URL not allowed
    char mcastport[6];
    uint32_t magicnum; // Magic number for broadcast packets
    char tcpport[6];
} OPT;

void ReadOptions(const char *filename);
