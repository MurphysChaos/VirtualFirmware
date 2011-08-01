/*
    dbg -- Functions for console output based on an error threshold.
*/

/*
    Copyright (C) 2011 Jacob Keller, Joel Murphy, All rights reserved

    The code in this file is licensed under GPL v2. This license does not 
    extend to any file bundled with this one.
*/

#include "dbg.h"

/* Output debug information
 */
void fdbg(FILE * f, int level, const char *msg, ...)
{
    if (OPT.dbglvl >= level) {
        va_list args;
	time_t t;
	char str_t[36];

	/* Print the time stamp */
	time(&t);
	strftime(str_t, 35, "%c", localtime(&t));
	fprintf(f, "%s: ", str_t); 

	/* Print the rest of the message */
        va_start(args, msg);
        vfprintf(f, msg, args);
        va_end(args);
    }
}

