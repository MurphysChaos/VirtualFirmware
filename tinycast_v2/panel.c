#include "panel.h"

struct addrinfo* ResolveAddress(char* addr, char* port, int family, int type, int protocol) {
  struct addrinfo hints;
  struct addrinfo* res = NULL;
  int rc = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_NUMERICSERV | AI_NUMERICHOST | ((addr) ? 0 : AI_PASSIVE);
  hints.ai_family = family;
  hints.ai_socktype = type;
  hints.ai_protocol = protocol;
  
  rc = getaddrinfo( addr, ((port) ? port : "0"), &hints, &res );
  if (rc != 0)
  {
    fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(rc));
    return NULL;
  }
  
  return res;
}

void FreeDestination(DESTINATION* d) {
  freeaddrinfo(d->r_addr);
  free(d);
}

PANEL* PanelCreate() {
  PANEL* p = NULL;
  
  p = (PANEL*) malloc(sizeof(PANEL));
  if(!p) {
    fprintf(stderr, "Failed to allocate memory for panel.\n");
    return NULL;
  }

  memset(p, 0, sizeof(PANEL));
  p->socket = INVALID_SOCKET;

  return p;
}

int PanelInit(PANEL* p, char* addr, char* port, int family, int type, int protocol) {
  int rc = 0, reuseaddr = -1;

  strncpy(p->source.host, ((addr) ? addr : "0.0.0.0"), NI_MAXHOST);
  strncpy(p->source.service, ((port) ? port : "0"), NI_MAXSERV);
  
  p->family = family;
  p->type = type;
  p->protocol = protocol;

  p->r_source = ResolveAddress(addr, port, family, type, protocol);
  if(!p->r_source) {
    fprintf(stderr, "failed to resolve socket address. %s\n", sock_error());
    return rc;
  }
  
  // Create the socket itself
  p->socket = socket(family, type, protocol);
  if(p->socket == INVALID_SOCKET) {
    fprintf(stderr, "failed to create socket. %s\n", sock_error());
    return rc;
  }

  // Bind the socket to the source address
  rc = bind(p->socket, p->r_source->ai_addr, p->r_source->ai_addrlen);
  if(rc == SOCKET_ERROR) {
    fprintf(stderr, "Unable to bind the socket to its source address. %s\n", sock_error());
    close(p->socket);
    p->socket = INVALID_SOCKET;
    return rc;
  }

  // Set the socket so its address is reusable.
  rc = setsockopt(p->socket, SOL_SOCKET, SO_REUSEADDR, (char*) &reuseaddr, sizeof(reuseaddr));
  if(rc == SOCKET_ERROR) {
    fprintf(stderr, "Unable to set the socket's source address as reusable. %s\n", sock_error());
    close(p->socket);
    p->socket = INVALID_SOCKET;
    return rc;
  }

  return rc;
}

int PanelSetDestination(PANEL* p, char* addr, char* port) {
  int rc = 0;
  
  DESTINATION* d = malloc(sizeof(DESTINATION));
  if(!d) {
    fprintf(stderr, "Failed to allocate memory for destination.\n");
    return -1;
  }
  
  strncpy(d->addr.host, addr, NI_MAXHOST);
  strncpy(d->addr.service, port, NI_MAXSERV);

  // In a connectionless system,
  // We would use the same socket.
  d->socket = p->socket;

  d->r_addr = ResolveAddress(addr, port, p->family, p->type, p->protocol);
  if(!d->r_addr) {
    fprintf(stderr, "failed to resolve destination address. %s\n", sock_error());
    return rc;
  }

  /* Add this destination to the linked list */
  d->next = p->destination;
  p->destination = d;

  return rc;
}

int PanelJoinMulticast(PANEL* p, char* addr) {
  struct ip_mreq mreq;
  struct ipv6_mreq mreq6;
  MGROUP* mgroup;
  int level, rc = 0;

  mgroup = malloc(sizeof(MGROUP));
  if(!mgroup) {
    fprintf(stderr, "Failed to allocate memory for multicast group\n");
    return SOCKET_ERROR;
  }

  if(p->family == AF_INET) {
    // IPv4 Membership
    inet_pton(AF_INET, addr, &mreq.imr_multiaddr.s_addr);
    level = IPPROTO_IP;
    mreq.imr_interface.s_addr = INADDR_ANY;
    
    rc = setsockopt(p->socket, level, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if(rc == SOCKET_ERROR) {
      fprintf(stderr, "Failed to join multicast group. %s\n", sock_error());
      free(mgroup);
      return rc;
    }
  }
  else if(p->family == AF_INET6) {
    // IPv6 Membership
    fprintf(stderr, "IPv6 Multicast not currently supported\n");
    free(mgroup);
    return SOCKET_ERROR;
  }
  else {
    fprintf(stderr, "Invalid address family.\n");
    free(mgroup);
    return SOCKET_ERROR;
  }

  strncpy(mgroup->host, addr, NI_MAXHOST);
  mgroup->next = p->mgroups;
  p->mgroups = mgroup;

  return rc;
}

int PanelConnectDestination(PANEL* p, char* addr, char* port) {
  DESTINATION* d;
  int rc = 0;
  
  rc = PanelSetDestination(p, addr, port);
  if(rc == SOCKET_ERROR) {
    return rc;
  }

  d = p->destination;
  d->isConnected = 1;

  rc = connect(p->socket, d->r_addr->ai_addr, d->r_addr->ai_addrlen);
  if(rc == SOCKET_ERROR) {
    fprintf(stderr, "failed to connect to destination. %s\n", sock_error());
    p->destination = d->next;
    FreeDestination(d);

    return rc;
  }

  return rc;
}

int PanelAcceptConnection(PANEL* p) {
  DESTINATION* d;
  uint32_t n_socket = INVALID_SOCKET;
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);
  char host[NI_MAXHOST];
  char serv[NI_MAXSERV];
  int rc = 0;
  
  n_socket = accept(p->socket, &addr, &addrlen);
  if(!n_socket) {
    fprintf(stderr, "failed to accept incoming connection. %s\n", sock_error());
    return SOCKET_ERROR;
  }

  rc = getnameinfo(&addr, addrlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
  if(rc != 0) {
    fprintf(stderr, "getnameinfo failed: %s\n", gai_strerror(rc));
    return SOCKET_ERROR;
  }

  rc = PanelSetDestination(p, host, serv);
  if(rc == SOCKET_ERROR) {
    close(n_socket);
    return rc;
  }

  d = p->destination;
  d->socket = n_socket;
  d->isConnected = 1;

  return rc;
}

int PanelSendData(PANEL* p, void* data, int length) {
  DESTINATION* d = p->destination;
  if(!d) {
    fprintf(stderr, "Destination has not been set yet!\n");
    return SOCKET_ERROR;
  }

  return sendto(d->socket, data, length, 0, d->r_addr->ai_addr, d->r_addr->ai_addrlen);
}

int PanelReceiveData(PANEL* p, void* buffer, int maxlength, struct sockaddr* fromaddr, socklen_t* fromlen) {
  return recvfrom(p->socket, buffer, maxlength, 0, fromaddr, fromlen);
}

int PanelDelete(PANEL* p) {
  freeaddrinfo(p->r_source);
  close(p->socket);
  
  while(p->destination) {
    DESTINATION* prev = p->destination;
    p->destination = prev->next;
    close(prev->socket);

    FreeDestination(prev);
  }

  while(p->mgroups) {
    MGROUP* prev = p->mgroups;
    p->mgroups = prev->next;

    free(prev);
  }

  free(p);
}
