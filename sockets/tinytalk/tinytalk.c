/* tinytalk -- A small test program to evaluate the effectiveness of panel.c 
 * and panel.h.
 *
 * Copyright (C) 2011 Joel Murphy, All rights reserved
 * The code in this file is licensed under GPL v2. This license does not 
 * extend to any file bundled with this one.
 * * * */ 

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include "panel.h"
#include "sock_error.h"

#ifndef _WIN32
#define Sleep(x) usleep(1000*x)
#endif

#define UDP_ADDR "234.1.1.10"	// Globally-scoped IPv4 multicast (same subnet)
#define UDP_PORT "9751"			// IANA unassigned port
#define UDP_TTL 16				// Time to live -- gives a max lifespan of 5 seconds
#define TCP_PORT "9752"			// IANA unassigned port
#define TT_BUFSIZE 1024
#define HS_MAGICNUM 0x1e36

int		bIsServer = 0;

/* A simple handshake packet for telling the client what port 
 * the server is listening on.
 * * * */
typedef struct hsPacket {
	uint32_t magic;
	uint16_t port;
	uint16_t flags; // reserved
} HSPACKET;

void ProcessArgs(int argc, char **argv)
{
	int i;
	for (i=1;i<argc;i++)
	{
		if (argv[i][0] == '-') {
			switch(tolower(argv[i][1]))
			{
			case 's':
				bIsServer = (~0);
				break;
			}
		}
	}
}

int main(int argc, char **argv) {
	PANEL			*p1 = NULL,
					*p2 = NULL,
					*p3 = NULL;
	fd_set			readfds;
	struct timeval	tv;
	size_t			ps = sizeof (PANEL);
	char			obuf[TT_BUFSIZE], 
					ibuf[TT_BUFSIZE];
	HSPACKET		hsp;
	int				sockaddr_size,
					sockdest = INVALID_SOCKET,
					rc;

#ifdef _WIN32
    // Load Winsock
	WSADATA wsd;
    if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", rc);
        goto cleanup;
    }
#endif

	ProcessArgs(argc, argv);

	// Create the announce socket.
	p1 = CreateBoundPanel(UDP_PORT,AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (p1 == NULL) {
		fprintf(stderr, "Failed to create bound panel p1. %s\n", sock_error());
		goto cleanup;
	}
	rc = SetDestination(p1,UDP_ADDR,UDP_PORT);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to set destination for panel p1. %s\n", sock_error());
		goto cleanup;
	}
	rc = MakeMulticast(p1);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to promote panel p1 to multicast. %s\n", sock_error());
		goto cleanup;
	}
	rc = SetMulticastTTL(p1, UDP_TTL);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to set multicast TTL. %s\n", sock_error());
		goto cleanup;
	}
	rc = SetMulticastLoopback(p1, 1);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to set multicast loopback. %s\n", sock_error());
		goto cleanup;
	}

	if (bIsServer) {
		// Prepare to accept connections.
		p2 = CreateBoundPanel(TCP_PORT,AF_INET,SOCK_STREAM,IPPROTO_TCP);
		rc = listen(p2->sp_socket, 1);
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to begin listening for new connections. %s\n", sock_error());
			goto cleanup;
		}

		// Announce until we detect a connection
		while (sockdest == INVALID_SOCKET) {
			// Build outgoing datagram
			hsp.magic = htonl(HS_MAGICNUM);
			hsp.port = htons(atoi(TCP_PORT));
			memcpy(obuf, &hsp, sizeof (hsp));

			// Sending datagram
			rc = sendto(p1->sp_socket, obuf, sizeof (obuf), 0, &(p1->sp_dest), sizeof (p1->sp_dest));
			if (rc == SOCKET_ERROR) {
				fprintf(stderr, "Failed to send to multicast address. %s\n", sock_error());
				goto cleanup;
			}
			printf("Sent handshake packets to ");
			PrintAddr(stdout, &(p1->sp_dest));
			printf("\n");

			// Preparing to receive incoming TCP connection
			FD_ZERO(&readfds);
			FD_SET(p2->sp_socket, &readfds);

			tv.tv_sec = 0;
			tv.tv_usec = 500000;
			rc = select(p2->sp_socket + 1, &readfds, NULL, NULL, &tv);

			if (rc == -1) {
				fprintf(stderr, "Error in select(). %s\n", sock_error());
				goto cleanup;
			} else if (rc == 0) {
				// No triggered events. Nothing to do.
			} else {
				// Connect request from client
				sockaddr_size = sizeof (struct sockaddr);
				sockdest = accept(p2->sp_socket, &(p2->sp_dest), &sockaddr_size);
				printf("Connection established with client at ");
				PrintAddr(stdout, &(p1->sp_bind));
				printf("\n");
				break;
			}
		}

		// Connection established. Receive some data.
		rc = recv(sockdest, ibuf, TT_BUFSIZE, 0);
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to receive usable data. %s\n", sock_error());
			goto cleanup;
		}
		printf("Received %u bytes from ", TT_BUFSIZE);
		PrintAddr(stdout, &(p2->sp_dest));
		printf("\n");
	} else {
		p2 = CreateBoundPanel(INADDR_ANY,"0",AF_INET,SOCK_STREAM,IPPROTO_TCP);
		sockaddr_size = sizeof (struct sockaddr);
		// Wait for announcement
		rc = recvfrom(p1->sp_socket, ibuf, TT_BUFSIZE, 0, &(p2->sp_dest), &sockaddr_size);
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to receive multicast packet. %s\n", sock_error());
			goto cleanup;
		}
		// Extract handshake
		memcpy(&hsp, ibuf, sizeof (hsp));
		hsp.magic = ntohl(hsp.magic);;
		hsp.port = ntohs(hsp.port);
		
		if (hsp.magic == HS_MAGICNUM) {
			// Valid handshake received, connect to server
			printf("Handshake packet received from ");
			PrintAddr(stdout, &(p2->sp_dest));
 			printf(". Server listening on port %u.\n", hsp.port);
			// Alter address to match target.
			((struct sockaddr_in *) &(p2->sp_dest))->sin_port = htons(hsp.port);
			rc = connect(p2->sp_socket, &(p2->sp_dest), sockaddr_size);
			if (rc == SOCKET_ERROR) {
				fprintf(stderr, "Failed to connect to ");
				PrintAddr(stdout, &(p2->sp_dest));
				fprintf(stderr, "\n");
			} else {
				printf("Connection established with server at ");
				PrintAddr(stdout, &(p2->sp_dest));
				printf("\n");
			}
		} else {
			// Invalid handshake received. Just a test program so quit.
			printf("Packet received from ");
			PrintAddr(stdout, &(p2->sp_dest));
			printf(". Not recognized as handshake.\n");
			goto cleanup;
		}

		memset(obuf, '~', TT_BUFSIZE);
		rc = send(p2->sp_socket, obuf, TT_BUFSIZE, 0);
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to send %u bytes to ", TT_BUFSIZE);
			PrintAddr(stdout, &(p2->sp_dest));
			fprintf(stderr, ". %s\n", sock_error());
			goto cleanup;
		}
		printf("Sent %u bytes to ", TT_BUFSIZE);
		PrintAddr(stdout, &(p2->sp_dest));
		printf("\n");
	}

cleanup:
#ifdef _WIN32
	// Close Winsock
	WSACleanup();
#endif

	if (p1) {
		FreePanel(p1);
	}
	if (p2) {
		FreePanel(p2);
	}
	return 0;
}
