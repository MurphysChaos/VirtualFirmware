/* 
    panel -- Socket simplification tool.

    Panels are a conceptual extension of a socket. They are 
    intended to assist in the creation of socket programs by reducing 
    the amount of code the author must write and by reducing code differences
    between Windows and Linux.
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

#ifndef PANEL_H
#define PANEL_H

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <WinError.h>
#include "stdint.h"
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sockerr.h"

/*
 * A trio of Winsock constants. 
 */
#ifndef NO_ERROR
#define NO_ERROR 0
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (~0)
#endif

/* Flag Macros & Flags
 */
#define sp_setflag(var,flag)	var = (var | flag)
#define sp_getflag(var,flag)	(var & flag)
#define sp_clearflag(var,flag)	var = (var & ~(flag))
#define SP_F_VALID		0x0001
#define SP_F_LISTENER	0x0002
#define SP_F_SENDER		0x0004
#define SP_F_MULTICAST	0x0010

/*
 * A "socket-panel" structure that keeps track of relevant creation
 * data.
 */
typedef struct socket_panel {
    uint32_t sp_socket;
    struct sockaddr sp_bind;
    struct sockaddr sp_dest;
    uint32_t sp_family;
    uint32_t sp_socktype;
    uint32_t sp_protocol;
    uint16_t sp_flags;
    uint16_t sp_zero;
} SOCKET_PANEL, PANEL;

/* == FUNCTIONS FOR CREATING SOCKET PANELS. */

/* Used as a helper function to create the "pre-zeroed" panel.
 */
PANEL *CreateEmptyPanel(void);

/* Creates a panel with only an active socket. Useful for most of your 
 * socket-panel needs.
 */
PANEL *CreatePanel(int af, int type, int proto);

/* Creates a panel that is pre-bound to an outgoing port. The contained 
 * socket will listen on all active interfaces. Use CreatePanel() then 
 * BindPanel() to listen on a specific interface.
 */
PANEL *CreateBoundPanel(const char *svc, int af, int type, int proto);

/* To be called after an accept to insert a socket into a panel.
 */
PANEL *SocketToPanel(int s, struct sockaddr *addr);

/* Nicely cleans up after an unused panel.
 */
void FreePanel(PANEL * p);

/* Nicely delets panel information but leaves socket intact*/
void DissociatePanel(PANEL * p);


/* == UTILITY FUNCTIONS */

/* Resolves a general address and service specifier into a net-standard 
 * internet address and port. The service an be a generalized service 
 * specifier (e.g. "http", "ftp", "smtp", etc.) or a specific port.
 * Returns NULL on error. Use freeaddrinfo() on the return result after 
 * you are done with it.
 */
struct addrinfo *ResolveAddr(const char *addr, const char *svc, int af,
                             int type, int proto);

/* == MANIPULATOR FUNCTIONS */

/* Causes a panel to bind to the stored bind address. Must be used on a 
 * valid panel.
 */
int BindPanel(PANEL * p, const char *addr, const char *svc, int reuse);

/* Aligns outgoing traffic for the panel. Must be used on a valid panel.
 */
int SetDestination(PANEL * p, const char *addr, const char *svc);

/* Sets the socket option with the linger structure. This doesn't work in 
 * Windows.
 */
int SetOptionLinger(PANEL * p, uint16_t opt_yesno, uint16_t opt_delay);

/* Used to set any true or false option
 */
int SetOption(PANEL * p, int level, int option, int optval);

/* -- Functions for multicasting */

/* Make an existing panel into a multicast panel by joining the multicast group
 * and setting the send interface. Uses p->sp_dest which must be set prior to 
 * calling.
 */
int JoinMulticastGroup(PANEL * p, const char *ifaddr);

/* Gracefully leave the multicast group.
 */
int LeaveMulticastGroup(PANEL * p, const char *ifaddr);

/* Override the default interface to send multicast messages from. If this 
 * is not called, the system default is used.
 */
int SetMulticastSendInterface(PANEL * p, struct sockaddr *addr);

/* Set the time-to-live (TTL) value for a multicast panel.
 */
int SetMulticastTTL(PANEL * p, int ttl);

/* Sets whether multicast packets, sent by a given interface, are processed by 
 * the originating host. Loopback is on by default.
 */
int SetMulticastLoopback(PANEL * p, int loopval);

/* == Debugging functions */
/* -- Fuctions for getting human-readable values on sockets. */

/* Turns an address into presentation format. The return value is the pointer
 * to buf.
 */
char *FormatAddr(struct sockaddr *sa, char *buf, int buflen);

/* Print information on an entire panel.
 */
void PrintPanel(FILE * f, PANEL * p);

/* Prints address family value */
void PrintAddrFamily(FILE * f, int af);

/* Prints address protocol value */
void PrintAddrProtocol(FILE * f, int proto);

/* Print all addresses in an Addrinfo linked list.
 */
void PrintAddrinfo(FILE * f, struct addrinfo *ai);

/* Print information on an address.
 */
void PrintAddr(FILE * f, struct sockaddr *sa);

#endif
