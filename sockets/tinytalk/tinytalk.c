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

#define UDP_ADDR "239.1.1.10"	// Locally-scoped IPv4 multicast (same subnet)
#define UDP_PORT "9751"			// IANA unassigned port
#define UDP_TTL 1				// Time to live -- gives a max lifespan of 1 second
#define TCP_PORT "9752"			// IANA unassigned port
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
	struct sockaddr	fromaddr;
	int				fromlen,
					fromsock = INVALID_SOCKET;
	fd_set			readfds;
	struct timeval	tv;
	size_t			ps = sizeof (PANEL);
	char			obuf[SP_BUFSIZE], 
					ibuf[SP_BUFSIZE];
	HSPACKET		hsp;
	int				rc, 
					i;

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

	p1 = GetNewPanel(UDP_ADDR,UDP_PORT,AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (p1) {
		MakeMulticast(p1);
		SetMulticastTTL(p1, UDP_TTL);
		SetMulticastLoopback(p1, 1);
		printf("Socket promoted to multicast.\n");
	} else {
		fprintf(stderr, "Multicast socket creation failed. Exiting.");
		goto cleanup;
	}

	if (bIsServer) {
		// Prepare to accept connections.
		p2 = GetNewPanel(INADDR_ANY,TCP_PORT,AF_INET,SOCK_STREAM,IPPROTO_TCP);
		rc = listen(p2->sp_socket, 1);
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to begin listening for new connections. %s\n", sock_error());
			goto cleanup;
		}
		// Announce until we detect a connection
		while (fromsock == INVALID_SOCKET) {
			// Build outgoing datagram
			hsp.magic = htonl(HS_MAGICNUM);
			hsp.port = htons(atoi(TCP_PORT));
			memcpy(obuf, &hsp, sizeof (hsp));

			// Sending datagram
			rc = sendto(p1->sp_socket, obuf, sizeof (obuf), 0, p1->sp_iface->ai_addr, p1->sp_iface->ai_addrlen);
			if (rc == SOCKET_ERROR) {
				fprintf(stderr, "Failed to send to multicast address. %s\n", sock_error());
				goto cleanup;
			}
			printf("Sent handshake packets to ");
			PrintSockaddr(stdout, p1->sp_iface->ai_addr);
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
				fromlen = sizeof fromaddr;
				fromsock = accept(p2->sp_socket, &fromaddr, &fromlen);
				printf("Connection established with client at ");
				PrintSockaddr(stdout, &fromaddr);
				printf("\n");
				break;
			}
		}

		// Connection established. Receive some data.
		rc = recv(fromsock, ibuf, SP_BUFSIZE, 0);
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to receive usable data. %s\n", sock_error());
			goto cleanup;
		}
		printf("Received %u bytes from ", SP_BUFSIZE);
		PrintSockaddr(stdout, &fromaddr);
		printf("\n");
	} else {
		p2 = GetNewPanel(INADDR_ANY,"0",AF_INET,SOCK_STREAM,IPPROTO_TCP);
		fromlen = sizeof (fromaddr);
		// Wait for announcement
		rc = recvfrom(p1->sp_socket, ibuf, SP_BUFSIZE, 0, &fromaddr, &fromlen);
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
			PrintSockaddr(stdout, &fromaddr);
 			printf(". Server listening on port %u.\n", hsp.port);
			// Alter address to match target.
			((struct sockaddr_in *) &fromaddr)->sin_port = htons(hsp.port);
			rc = connect(p2->sp_socket, &fromaddr, fromlen);
			if (rc == SOCKET_ERROR) {
				fprintf(stderr, "Failed to connect to ");
				PrintSockaddr(stdout, &fromaddr);
				fprintf(stderr, "\n");
			} else {
				printf("Connection established with server at ");
				PrintSockaddr(stdout, &fromaddr);
				printf("\n");
			}
		} else {
			// Invalid handshake received. Just a test program so quit.
			printf("Packet received from ");
			PrintSockaddr(stdout, &fromaddr);
			printf(". Not recognized as handshake.\n");
			goto cleanup;
		}

		memset(obuf, '~', SP_BUFSIZE);
		rc = send(p2->sp_socket, obuf, SP_BUFSIZE, 0);
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to send %u bytes to ", SP_BUFSIZE);
			PrintSockaddr(stdout, &fromaddr);
			fprintf(stderr, ". %s\n", sock_error());
			goto cleanup;
		}
		printf("Sent %u bytes to ", SP_BUFSIZE);
		PrintSockaddr(stdout, &fromaddr);
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
