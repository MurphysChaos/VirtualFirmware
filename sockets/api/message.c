#include "message.h"

struct announce_msg {
  uint32_t magic;
  uint16_t port;
  uint16_t flags; // reserved
};

#define MULTICAST_ADDR "234.1.1.10"
#define MULTICAST_PORT "9751"
#define UDP_TTL 1

int announce(char* d_port, int magic) {
  PANEL* hs;
  PANEL* cs;

  fd_set readfds;
  struct timeval tv;
  struct announce_msg m;
  
  int acceptlen;

  int socket = INVALID_SOCKET;
  int rc = 0;

  hs = CreateBoundPanel(MULTICAST_PORT, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(!hs) {
    goto err;
  }

  rc = SetDestination(hs, MULTICAST_ADDR, MULTICAST_PORT);
  if(rc == SOCKET_ERROR) {
    goto err;
  }

  rc = MakeMulticast(hs);
  if (rc == SOCKET_ERROR) {
    goto err;
  }

  rc = SetMulticastTTL(hs, UDP_TTL);
  if (rc == SOCKET_ERROR) {
    goto err;
  }

  rc = SetMulticastLoopback(hs, 1);
  if (rc == SOCKET_ERROR) {
    goto err;
  }

  cs = CreateBoundPanel(d_port, AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(!cs) {
    goto err;
  }

  rc = listen(cs->sp_socket, 1);
  if(rc == SOCKET_ERROR) {
    goto err;
  }
  
  /* Begin the main loop */
  while(socket == INVALID_SOCKET) {
    m.magic = htonl(magic);
    m.port = htons(atoi(d_port));
    
    rc = sendto(hs->sp_socket, &m, sizeof(m), 0, &(hs->sp_dest), sizeof(hs->sp_dest));
    if (rc == SOCKET_ERROR) {
      goto err;
    }

    FD_ZERO(&readfds);
    FD_SET(cs->sp_socket, &readfds);
    
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    rc = select(cs->sp_socket + 1, &readfds, NULL, NULL, &tv);
    if (rc < 0) {
      goto err;
    } else if (rc == 0) {
      continue;
    } else {
      acceptlen = sizeof(struct sockaddr);
      socket = accept(cs->sp_socket, &(cs->sp_dest), &acceptlen);
     }
  }

  FreePanel(hs);
  DissociatePanel(cs);
 
  return socket;

 err:
  if (hs) {
    FreePanel(hs);
  }

  if (cs) {
    FreePanel(cs);
  }

  return INVALID_SOCKET;
}

int locate(int magic) {
  PANEL* hs;
  PANEL* cs;

  fd_set readfds;
  struct timeval tv;
  struct announce_msg m;
  
  int recvlen;

  int socket = INVALID_SOCKET;
  int rc = 0;

  hs = CreateBoundPanel(MULTICAST_PORT, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(!hs) {
    goto err;
  }

  rc = SetDestination(hs, MULTICAST_ADDR, MULTICAST_PORT);
  if(rc == SOCKET_ERROR) {
    goto err;
  }

  rc = MakeMulticast(hs);
  if (rc == SOCKET_ERROR) {
    goto err;
  }

  rc = SetMulticastTTL(hs, UDP_TTL);
  if (rc == SOCKET_ERROR) {
    goto err;
  }

  rc = SetMulticastLoopback(hs, 1);
  if (rc == SOCKET_ERROR) {
    goto err;
  }

  cs = CreateBoundPanel("0", AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(!cs) {
    goto err;
  }

  while(socket == INVALID_SOCKET) {
    recvlen = sizeof(struct sockaddr);
    rc = recvfrom(hs->sp_socket, &m, sizeof(m), 0, &(cs->sp_dest), &recvlen);
    if (rc == SOCKET_ERROR) {
      goto err;
    }

    m.magic = ntohl(m.magic);

    if (m.magic == magic) {
      ((struct sockaddr_in *) &(cs->sp_dest))->sin_port = m.port;
      rc = connect(cs->sp_socket, &(cs->sp_dest), sizeof(struct sockaddr));
      if (rc == SOCKET_ERROR) {
	goto err;
      }
      else {
	socket = cs->sp_socket;
      }
    }
  }

  FreePanel(hs);
  DissociatePanel(cs);

  return socket;

 err:
  if (hs) {
    FreePanel(hs);
  }

  if (cs) {
    FreePanel(cs);
  }

  return INVALID_SOCKET;
}

