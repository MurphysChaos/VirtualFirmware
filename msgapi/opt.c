#include "opt.h"

/*
 * Reads the file optrc from the current directory. This function 
 * performs is non-robust. It ignores lines that don't have the form
 * <token>=<value>.
 */

const char *default_file = OPTRC;
struct options OPT;

void ReadOptions(const char *filename) {
    FILE *f = NULL;
    char line[OPTBUFLEN];
    char *eqpos;
    
    int rc = 0;

    if (filename == NULL) {
        filename = default_file;
    }

    strcpy(OPT.mcastip, MULTICAST_ADDR);
    strcpy(OPT.mcastport, MULTICAST_PORT);
    strcpy(OPT.tcpport, TCP_PORT);
    OPT.mcastttl = MULTICAST_TTL;
    OPT.magicnum = MAGIC_NUMBER;
    
    f = fopen(filename, "r");
    if (f) {
        while (!feof(f)) {
            rc = fscanf(f, "%s\n", line);
	    strtok(line, "#"); // Remove comment from string
            eqpos = strchr(line, '=');
            if (eqpos) {
                eqpos[0] = '\0';
                
                if (strcmp(line, "mcastip") == 0) {
		    strcpy(OPT.mcastip, eqpos+1);
                } else if (strcmp(line, "mcastport") == 0) {
		     strcpy(OPT.mcastport, eqpos+1);
		} else if (strcmp(line, "mcastttl") == 0) {
		     OPT.mcastttl = atoi(eqpos + 1);
                } else if (strcmp(line, "tcpport") == 0) {
		     strcpy(OPT.tcpport, eqpos+1);
                } else if (strcmp(line, "magicnum") == 0) {
                    OPT.magicnum = atoi(eqpos + 1);
                }
            }
        }
        fclose(f);
    }
}

