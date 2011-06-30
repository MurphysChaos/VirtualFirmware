/* panel.h -- Panels are a conceptual extension of a socket. They are 
 * intended to assist in the creation of socket programs by reducing 
 * the amount of code the author must write and by reducing code differences
 * between Windows and Linux.
 *
 * Copyright (C) 2011 Joel Murphy, Jacob Keller, All rights reserved
 * The code in this file is licensed under GPL v2. This license does not 
 * extend to any file bundled with this one.
 * * * */ 

#ifndef PANEL_H
#define PANEL_H

#ifdef _cplusplus
extern "C" {
#endif

#define SP_BUFSIZE 1024					// Allows 1kb + 1 null.

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (~0)
#endif

/* Functions in this library are intended to take a pointer to a panel struct 
 * and any options as arguments. Functions that take something else are helper
 * functions, not intended to be called by the user.
 * * * */
typedef struct panel {
	uint32_t		sp_socket;
	struct addrinfo	*sp_iface;
	struct addrinfo *sp_bind;
} PANEL;

/* "New" functions create a new panel and return a pointer to it. "Make" functions
 * significantly alter an exisitng socket.
 * * * */

PANEL *GetEmptyPanel();
PANEL *GetNewPanel(char *addr, char *port, int af, int type, int proto);
PANEL *GetNewPanelWithSocket(int s, struct sockaddr *addr);
void FreePanel(PANEL *p);
int MakeMulticast(PANEL *p);
int SetMulticastSendInterface(PANEL *p, struct sockaddr *addr);
int SetMulticastTTL(PANEL *p, int ttl);
int SetMulticastLoopback(PANEL *p, int loopval);
void PrintPanel(PANEL *p);
void PrintAddrinfo(struct addrinfo *ai);
void PrintSockaddr(FILE* f, struct sockaddr *sa);

#ifdef _cplusplus
}
#endif

#endif
