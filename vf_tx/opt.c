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
