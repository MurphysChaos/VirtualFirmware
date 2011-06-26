#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include "panel.h"
#include "wsa_strerror.h"

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
	PANEL *p1 = NULL, *p2 = NULL;
	size_t ps = sizeof (PANEL);
	HSPACKET hsp;
	int rc, i;

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
		MakeMulticast(p1, UDP_TTL);
		PrintPanel(p1);
	} else {
		fprintf(stderr, "Multicast socket creation failed. Exiting.");
		goto cleanup;
	}

	if (bIsServer) {
		hsp.magic = htonl(HS_MAGICNUM);
		hsp.port = htons(atoi(TCP_PORT));

		for (i=1;i<20;i++) {
			rc = sendto(p1->sp_socket, (char *) &hsp, sizeof (hsp), 0, p1->sp_iface->ai_addr, p1->sp_iface->ai_addrlen);
			if (rc == SOCKET_ERROR) {
				fprintf(stderr, "Failed to send to multicast address. %s\n", wsa_strerror(WSAGetLastError()));
				goto cleanup;
			}
			printf("Sent handshake packet to ");
			PrintSockaddr(p1->sp_iface->ai_addr);
			printf("\n");
		}

		Sleep(1000);
	} else {
		struct sockaddr fromaddr;
		int				fromlen;

		for (i=1;i<20;i++) {
			fromlen = sizeof (fromaddr);
			rc = recvfrom(p1->sp_socket, (char *) p1->sp_iface, SP_BUFSIZE, 0, &fromaddr, &fromlen);
			if (rc == SOCKET_ERROR) {
				fprintf(stderr, "Failed to receive multicast packet. %s\n", wsa_strerror(WSAGetLastError()));
				goto cleanup;
			}
			hsp.magic = ntohl(((HSPACKET *) (p1->sp_ibuf))->magic);
			hsp.port = ntohs(((HSPACKET *) (p1->sp_ibuf))->port);

			if (hsp.magic == HS_MAGICNUM) {
				printf("Handshake packet received from ");
				PrintSockaddr(&fromaddr);
				printf(". Server listening on port %u.\n", hsp.port);
			} else {
				printf("Packet received from ");
				PrintSockaddr(&fromaddr);
				printf(". Not recognized as handshake.\n");
			}
		}
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