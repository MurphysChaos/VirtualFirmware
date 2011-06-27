#include "panel.h"
#include <sys/time.h>

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
  PANEL* hs = NULL;
  PANEL* cs = NULL;
  HSPACKET hsp;
  fd_set socks;
  struct timeval timeout;
  int rc, i, ready;
  char addressName[NI_MAXHOST];
  char msg[] = "Hello, client. I am ready to Serve.";
  char msgBuf[100];

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
    hs = PanelCreate();
    rc = PanelInit(hs, NULL, NULL, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to initialize handshake socket.\n");
      goto cleanup;
    }

    rc = PanelSetDestination(hs, UDP_ADDR, UDP_PORT);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to set the multicast destination.\n");
      goto cleanup;
    }

    cs = PanelCreate();
    rc = PanelInit(cs, NULL, TCP_PORT, AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to initialize connection socket.\n");
      goto cleanup;
    }


    listen(cs->socket, 10);

    hsp.magic = htonl(HS_MAGICNUM);
    hsp.port = htons(atoi(TCP_PORT));

    while(1) {
      rc = PanelSendData(hs, &hsp, sizeof(hsp));
      if(rc < 0) {
	fprintf(stderr, "failed to send magicNumber packet\n");
	goto cleanup;
      } else {
	FormatSockaddr(hs->destination->r_addr->ai_addr, addressName, NI_MAXHOST);
	printf("sent handshake packet to %s\n", addressName);
      }

      // Setup the select loop
      FD_ZERO(&socks);
      FD_SET(cs->socket, &socks);

      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      ready = select((cs->socket)+1, &socks, NULL, NULL, &timeout);
      if(ready < 0) {
	fprintf(stderr, "select failed\n");
	exit(1);
      } else if(ready > 0) {
	rc = PanelAcceptConnection(cs);
	if(rc == SOCKET_ERROR) {
	  fprintf(stderr, "failed to accept connection.\n");
	  goto cleanup;
	}
	break;
      }
    }

    PanelSendData(cs, msg, strlen(msg));

  } else {
    // Client Code
    char serv[NI_MAXHOST];
    char port[NI_MAXSERV];
    
    struct sockaddr fromaddr;
    struct sockaddr_in* fromaddr_in = (struct sockaddr_in*)&fromaddr;
    int fromlen = sizeof(fromaddr);

    hs = PanelCreate();
    rc = PanelInit(hs, NULL, UDP_PORT, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to initialize handshake socket.\n");
      goto cleanup;
    }

    rc = PanelJoinMulticast(hs, UDP_ADDR);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to join the multicast group.\n");
      goto cleanup;
    }

    cs = PanelCreate();

    rc = PanelInit(cs, NULL, NULL, AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "failed to initialize the connection socket.\n");
      goto cleanup;
    }

    while(1) {
      memset(&fromaddr, 0, sizeof(fromaddr));
      fromlen = sizeof(fromaddr);

      rc = PanelReceiveData(hs, &hsp, sizeof(hsp), &fromaddr, &fromlen);
      if(rc > 0) {
	hsp.magic = ntohl(hsp.magic);
	hsp.port = ntohs(hsp.port);

	FormatSockaddr(&fromaddr, addressName, NI_MAXHOST);
	printf("READ %d bytes from %s\n", rc, addressName);

	if(hsp.magic == HS_MAGICNUM) {
	  printf("CONNECT to %s\n", addressName);
	  inet_ntop(AF_INET, &fromaddr_in->sin_addr, serv, NI_MAXHOST);
	  snprintf(port, NI_MAXSERV, "%d", hsp.port);
	  rc = PanelConnectDestination(cs, serv, port);

	  if(rc < 0) {
	    fprintf(stderr, "connect failed\n");
	    goto cleanup;
	  }

	  break;
	}
      } else if(rc < 0) {
	fprintf(stderr, "recvfrom failed\n");
	goto cleanup;
      }
    }

    PanelReceiveData(cs, msgBuf, 100, NULL, 0);
    printf("%s\n", msg);
  }


 cleanup:
#ifdef _WIN32
  // Close Winsock
  WSACleanup();
#endif

  if (hs) {
    PanelDelete(hs);
  }

  if (cs) {
    PanelDelete(cs);
  }

  return 0;
}
