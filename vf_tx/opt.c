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

#include "opt.h"

const char *default_file = OPTRC;
struct options OPT = { MULTICAST_ADDR, MULTICAST_PORT, MAGIC_NUMBER, TCP_PORT, CONNECT_TIMEOUT, DBGLVL, 0 };

/* Read in neccessary information and put
 * them in the "options" structure
 */
void ReadOptions(const char *filename)
{
    FILE *f = NULL;
    char line[OPTBUFLEN];
    char *eqpos;

    int rc = 0;

    if (OPT.set) {
        return;
    }

    if (filename == NULL) {
        filename = default_file;
    }

    OPT.set = 1;

    f = fopen(filename, "r");
    if (f) {
        while (!feof(f)) {
            rc = fscanf(f, "%s\n", line);
            strtok(line, "#");  // Remove comment from string
            eqpos = strchr(line, '=');
            if (eqpos) {
                eqpos[0] = '\0';

                if (strcmp(line, "mcastip") == 0) {
                    strcpy(OPT.mcastip, eqpos + 1);
                } else if (strcmp(line, "mcastport") == 0) {
                    strcpy(OPT.mcastport, eqpos + 1);
                } else if (strcmp(line, "tcpport") == 0) {
                    strcpy(OPT.tcpport, eqpos + 1);
                } else if (strcmp(line, "magicnum") == 0) {
                    OPT.magicnum = atoi(eqpos + 1);
                } else if (strcmp(line, "timeout") == 0) {
                    OPT.timeout = atoi(eqpos + 1);
                } else if (strcmp(line, "dbglvl") == 0) {
                    OPT.dbglvl = atoi(eqpos + 1);
                }
            }
        }

        if (fclose(f)) {
            fprintf(stderr, "Failed to close %s.\n", filename);
        }
    }
}
