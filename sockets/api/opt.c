#include "opt.h"

/*
 * Reads the file optrc from the current directory. This function 
 * performs is non-robust. It ignores lines that don't have the form
 * <token>=<value>.
 */

void ReadOptions() {
    FILE *f = NULL;
    char line[OPTBUFLEN];
    char *eqpos;
    
    OPT.mcastip[0] = '\0';
    OPT.mcastport[0] = '\0';
    OPT.tcpport[0] = '\0';
    OPT.magicnum = 0;
    
    f = fopen(OPTRC, "r");
    if (f) {
        while (!feof(f)) {
            fscanf(f, "%s\n", line);
            strtok(line, "#"); // Remove comment from string
            eqpos = strchr(line, '=');
            if (eqpos) {
                eqpos[0] = '\0';
                
                if (strcmp(line, "mcastip") == 0) {
                    strcpy(OPT.mcastip, eqpos+1);
                } else if (strcmp(line, "mcastport") == 0) {
                    strcpy(OPT.mcastport, eqpos+1);
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

int main() {
    ReadOptions();
    if (OPT.mcastip[0] != '\0') {
        printf("mcastip=%s\n", OPT.mcastip);
    }
    if (OPT.mcastport[0] != '\0') {
        printf("mcastport=%s\n", OPT.mcastport);
    }
    if (OPT.tcpport[0] != '\0') {
        printf("tcpport=%s\n", OPT.tcpport);
    }
    if (OPT.magicnum) {
        printf("magicnum=%d\n", OPT.magicnum);
    }
    
    return 0;
}
