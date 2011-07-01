/* 
 * panel.h -- Panels are a conceptual extension of a socket. They are 
 * intended to assist in the creation of socket programs by reducing 
 * the amount of code the author must write and by reducing code differences
 * between Windows and Linux.
 *
 * Copyright (C) 2011 Joel Murphy, Jacob Keller, All rights reserved
 * The code in this file is licensed under GPL v2. This license does not 
 * extend to any file bundled with this one.
 */

#ifndef PANEL_H
#define PANEL_H

#ifdef _cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <WinError.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include "sock_error.h"

#define SP_BUFSIZE 1024					// Allows 1kb + 1 null.

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
#define sp_setflag(var,flag)	(var | flag)
#define sp_getflag(var,flag)	(var & flag)
#define sp_clearflag(var,flag)	(var & ~(flag))
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

/*
 * A basic multicasting announcement structure. Lets distant hosts know
 * which port the TCP socket is listening on.
 */
typedef struct sp_announce {
	uint32_t sp_magic;
	uint16_t sp_port;
	uint16_t zero;
} SP_ANNOUNCE;

/* Functions for creating socket panels. */
PANEL *CreateEmptyPanel();
PANEL *CreatePanel(int af, int type, int proto);
PANEL *CreateBoundPanel(char *addr, char *svc, int af, int type, int proto);
PANEL *SocketToPanel(int s, struct sockaddr *addr);
void FreePanel(PANEL *p);
/* == UTILITY FUNCTIONS */
struct addrinfo *ResolveAddr(char *addr, char *svc, int af, int type, int proto);
/* == MANIPULATOR FUNCTIONS */
int BindPanel(PANEL *p, char *addr, char *svc);
int SetDestination(PANEL *p, char *addr, char *svc);
/* -- Functions for multicasting */
int MakeMulticast(PANEL *p);
int SetMulticastSendInterface(PANEL *p, struct sockaddr *addr);
int SetMulticastTTL(PANEL *p, int ttl);
int SetMulticastLoopback(PANEL *p, int loopval);
/* == Debugging functions */
/* -- Fuctions for getting human-readable values on sockets. */
char *FormatAddr(struct sockaddr *sa, char *buf, int buflen);
void PrintPanel(FILE *f, PANEL *p);
void PrintAddrFamily(FILE *f, int af);
void PrintAddrProtocol(FILE *f, int proto);
void PrintAddrinfo(FILE *f, struct addrinfo *ai);
void PrintAddr(FILE *f, struct sockaddr *sa);

#ifdef _cplusplus
}
#endif

#endif
