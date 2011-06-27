#include "panel.h"

#define UDP_ADDR "239.1.1.10"
#define UDP_PORT "9751"
#define TCP_PORT "9752"
#define HS_MAGICNUM 0x1e36

#ifndef _WIN32
#define Sleep(x) usleep(1000*x)
#endif

int bIsServer = 0;

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
  for (i=1;i<argc;i++) {
    if (argv[i][0] == '-') {
      switch(tolower(argv[i][1])) {
      case 's':
	bIsServer = (~0);
	break;
      }
    }
  }
}

int main(int argc, char **argv) {
  PANEL* p = NULL;
  HSPACKET hsp;
  int rc, i;
  char addressName[NI_MAXHOST];
  
#ifdef _WIN32
  // Load Winsock
  WSADATA wsd;
  if ((rc = WSAStartup(MAKEWORD(1, 1), &wsd)) != 0) {
    fprintf(stderr, "WSAStartup failed: %d\n", rc);
    goto cleanup;
  }
#endif

  ProcessArgs(argc, argv);

  if(bIsServer) {
    // Server Code
    p = PanelCreate();
    rc = PanelInit(p, NULL, NULL, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to initialize handshake socket.\n");
      goto cleanup;
    }

    rc = PanelSetDestination(p, UDP_ADDR, UDP_PORT);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to set the multicast destination.\n");
      goto cleanup;
    }

    hsp.magic = htonl(HS_MAGICNUM);
    hsp.port = htons(atoi(TCP_PORT));

    for(i = 0; i < 20; i++) {
      rc = PanelSendData(p, &hsp, sizeof(hsp));
      if(rc < 0) {
	fprintf(stderr, "Failed to send requested data.\n");
	goto cleanup;
      } else {
	FormatSockaddr(p->destination->r_addr->ai_addr, addressName, NI_MAXHOST);
	printf("sent handshake packet #%d to %s\n", i, addressName);
      }
      
      Sleep(1000);
    }
  } else {
    // Client Code
    struct sockaddr fromaddr;
    int fromlen;

    p = PanelCreate();
    rc = PanelInit(p, NULL, UDP_PORT, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to initialize handshake socket.\n");
      goto cleanup;
    }

    rc = PanelJoinMulticast(p, UDP_ADDR);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to join the multicast group.\n");
      goto cleanup;
    }

    for(i = 0; i < 20; i++) {
      fromlen = sizeof(fromaddr);
      
      rc = PanelReceiveData(p, &hsp, sizeof(HSPACKET), &fromaddr, &fromlen);
      if(rc < 0) {
	fprintf(stderr, "Failed to receive data from handshake socket.\n");
	goto cleanup;
      }
      
      hsp.magic = ntohl(hsp.magic);
      hsp.port = ntohs(hsp.port);

      if(hsp.magic == HS_MAGICNUM) {
	FormatSockaddr(&fromaddr, addressName, NI_MAXHOST);
	printf("Handshake packet received from %s\n", addressName);
	printf("Server listening on port %d\n", hsp.port);
      } else {
	printf("Packet recieved\n");
	printf("Not recognized as a handshake packet\n");
      }      
    }
  }

 cleanup:
#ifdef _WIN32
  // Close Winsock
  WSACleanup();
#endif

  if (p) {
    PanelDelete(p);
  }

  return 0;
}
