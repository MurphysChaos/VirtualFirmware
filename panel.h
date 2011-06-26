/* panel.h -- Panels are a conceptual extension of a socket. They are set up
 * as bi-directional, although, they need not be used as such.
 *
 * Panels are implemented for TCP/IP, UDP, and RAW sockets for IPv4 and IPv6.
 * * * */

#ifndef PANEL_H
#define PANEL_H

#ifdef _cplusplus
extern "C" {
#endif

#define SP_BUFSIZE 1024					// Allows 1kb + 1 null.

/* Functions in this library are intended to take a pointer to a panel struct 
 * and any options as arguments. Functions that take something else are helper
 * functions, not intended to be called by the user.
 * * * */
typedef struct panel {
	uint32_t		sp_socket;
	char			*sp_ibuf;
	char			*sp_obuf;
	struct addrinfo	*sp_iface;
	struct addrinfo *sp_bind;
	uint32_t		sp_af;
} PANEL;

/* "New" functions create a new panel and return a pointer to it. "Make" functions
 * significantly alter an exisitng socket.
 * * * */

PANEL *GetEmptyPanel();
PANEL *GetNewPanel(char *addr, char *port, int af, int type, int proto);
PANEL *GetNewPanelWithSocket(int s, struct sockaddr *addr);
void FreePanel(PANEL *p);
int MakeMulticast(PANEL *p, int ttl);
int SetMulticastTTL(PANEL *p, int ttl);
void PrintPanel(PANEL *p);
void PrintAddrinfo(struct addrinfo *ai);
void PrintSockaddr(struct sockaddr *sa);

#ifdef _cplusplus
}
#endif

#endif
