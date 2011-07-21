#include "message.h"

struct announce_msg {
  uint32_t magic;
  uint16_t port;
  uint16_t flags; // reserved
};

#define PACKETS_PER_SEC 10

#define DBG_STATUS 1
#define DBG_ERROR  2
#define DBG_ALL    3

void dbg(int level, const char *msg, ...) {
  if (OPT.dbglvl >= level) {
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
  }
}

typedef struct if_panel {
  PANEL *hs;
  PANEL *cs;
} IF_PANEL;

typedef struct if_data {
  char if_addr[NI_MAXHOST];
  char if_name[NI_MAXHOST];
  struct sockaddr sa;
} IF_DATA;

int buildIfPanel(IF_PANEL *p, IF_DATA *i) {
  int rc = 0;

  dbg(DBG_ALL, "Using interface '%s' (%s)\n", i->if_addr, i->if_name);

  p->hs = CreatePanel(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (p->hs == NULL) {
    return SOCKET_ERROR;
  }

  rc = BindPanel(p->hs, i->if_addr, OPT.mcastport, 1);
  if (rc == SOCKET_ERROR) {
    return rc;
  }

  rc = SetDestination(p->hs, OPT.mcastip, OPT.mcastport);
  if (rc == SOCKET_ERROR) {
    return rc;
  }

  rc = JoinMulticastGroup(p->hs, NULL);
  if (rc == SOCKET_ERROR) {
    return rc;
  }

  rc = SetMulticastSendInterface(p->hs, &i->sa);
  if (rc == SOCKET_ERROR) {
    return rc;
  }

  rc = SetMulticastTTL(p->hs, OPT.mcastttl);
  if (rc == SOCKET_ERROR) {
    return rc;
  }

  rc = SetMulticastLoopback(p->hs, 1);
  if (rc == SOCKET_ERROR) {
    return rc;
  }

  p->cs = CreatePanel(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (p->cs == NULL) {
    return SOCKET_ERROR;
  }

  rc = BindPanel(p->cs, i->if_addr, OPT.tcpport, 1);
  if (rc == SOCKET_ERROR) {
    return rc;
  }

  return 0;
}

int populateInterfaceData(IF_DATA *if_d, int *numIfs) {
#ifdef _WIN32
  SOCKET s;
  int numFoundIfs = 0, i, rc = 0, pc = 0;
  INTERFACE_INFO interfaces[32];
  unsigned long nReturned = 0;
  
  s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s == INVALID_SOCKET) {
    dbg(DBG_ERROR, "socket(): '%s'\n", sock_error());
    rc = SOCKET_ERROR;
    goto err;
  }

  rc = WSAIoctl(s,
		SIO_GET_INTERFACE_LIST,
		0,
		0,
		&interfaces,
		sizeof(INTERFACE_INFO) * 32,
		&nReturned,
		0,
		0);

  if (rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "SIO_GET_INTERFACE_LIST: '%s'\n", sock_error());
    goto err;
  }

  nReturned /= sizeof(INTERFACE_INFO);
  for (i = 0; i < nReturned; i++) {
    INTERFACE_INFO *pIf = &interfaces[i];
    if ((pIf->iiFlags & IFF_UP) &&
	(pIf->iiFlags & IFF_MULTICAST) &&
	!(pIf->iiFlags & IFF_LOOPBACK)) {

      /* We check to make sure we can hold the interface */
      if ((numFoundIfs + 1) > *numIfs) {
	numFoundIfs++;
	continue;
      }

      /* We found a useful interface */
      rc = getnameinfo(&(pIf->iiAddress.Address),
		       sizeof(struct sockaddr_in),
		       if_d[numFoundIfs].if_addr,
		       NI_MAXHOST,
		       0,
		       0,
		       NI_NUMERICHOST);
      if (rc != 0) {
	rc = SOCKET_ERROR;
	return rc;
      }
      
      /* Name the adapter */
      sprintf(if_d[numFoundIfs].if_name,
	       "ifwin%d",
	       i);

      /* Copy the struct sockaddr */
      memcpy(&if_d[numFoundIfs].sa, &pIf->iiAddress.Address, sizeof(struct sockaddr));

      /* Increment the total number of adapters */
      numFoundIfs++;
    }
  }

  /* return an error if we didn't have enough storage */
  if (numFoundIfs > *numIfs) {
    set_error(ENOMEM);
    dbg(DBG_ERROR, "populateInterfaceData(): not enough memory\n");
    rc = SOCKET_ERROR;
  }

  *numIfs = numFoundIfs;
  return rc;
  
 err:
  return rc;
#else
  struct ifaddrs *ifaddrs = NULL;
  struct ifaddrs *ifa = NULL;

  int numFoundIfs = 0;
  int rc = 0;

  rc = getifaddrs(&ifaddrs);
  if (rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "getifaddrs(): %s\n", sock_error());
    return rc;
  }

  for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    if (ifa->ifa_addr->sa_family != AF_INET)
      continue;

    if ((ifa->ifa_flags & IFF_UP) &&
	(ifa->ifa_flags & IFF_MULTICAST) &&
	!(ifa->ifa_flags & IFF_LOOPBACK)) {
      /* If we get here, then we want to use this interface
       * NOTE: first check that we have space. */
      if ((numFoundIfs + 1) > *numIfs) {
	numFoundIfs++;
	continue;
      }

      rc = getnameinfo(ifa->ifa_addr,
		       sizeof(struct sockaddr_in),
		       if_d[numFoundIfs].if_addr,
		       NI_MAXHOST,
		       0,
		       0,
		       NI_NUMERICHOST);
      if (rc != 0) {
	rc = SOCKET_ERROR;
	return rc;
      }

      /* Copy relevant data into the IF_DATA buffer */
      strncpy(if_d[numFoundIfs].if_name, ifa->ifa_name, NI_MAXHOST);
      memcpy(&if_d[numFoundIfs].sa, ifa->ifa_addr, sizeof(struct sockaddr));

      /* Increment the total number of interfaces */
      numFoundIfs++;
    }
  }
  
  /* Didn't have enough memory */
  if (numFoundIfs > *numIfs) {
    set_error(ENOMEM);
    dbg(DBG_ERROR, "populateInterfaceData(): not enough memory\n");
    rc = SOCKET_ERROR;
  }

  *numIfs = numFoundIfs;
  freeifaddrs(ifaddrs);
  return rc;
  
 err:
  freeifaddrs(ifaddrs);
  return rc;
#endif
}

#define MAX_IF_LENGTH 32
SOCKET announce(const char *optrc) {
  IF_PANEL ifp[MAX_IF_LENGTH];
  IF_DATA ifd[MAX_IF_LENGTH];
  int ifd_length = MAX_IF_LENGTH;

  fd_set readfds;
  struct timeval tv;
  int num_packets = 0;
  int count = 0;

  int maxSocket = 0;

  char addr[NI_MAXHOST];

  struct announce_msg m;
  
  socklen_t acceptlen;
  int i;

  SOCKET socket = INVALID_SOCKET;
  int rc = 0;

  ReadOptions(optrc);

  memset(ifp, 0, sizeof(IF_PANEL) * 32);

  rc = populateInterfaceData(ifd, &ifd_length);
  if (rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "populateInterfaceData(): '%s'\n", sock_error());
    goto err;
  }

  /* We have the interface data in useable form
   * Now we can build the interface panels */
  for (i = 0; i < ifd_length; i++) {
    rc = buildIfPanel(&ifp[i], &ifd[i]);
    if (rc == SOCKET_ERROR) {
      dbg(DBG_ERROR, "buildIfPanel(%d): '%s'\n", i, sock_error());
      goto err;
    }
  }

  /* Listen on each interface */
  for (i = 0; i < ifd_length; i++) {
    rc = listen(ifp[i].cs->sp_socket, 1);
    if (rc == SOCKET_ERROR) {
      dbg(DBG_ERROR, "listen(cs): '%s'\n", sock_error());
      goto err;
    }
  }

  num_packets = OPT.timeout * PACKETS_PER_SEC;

  /* the funny condition causes us to loop forever
   * if we have a timeout of 0 */
  for(count = 0; (num_packets == 0 || count < num_packets); count++) {
    m.magic = htonl(OPT.magicnum);
    m.port = htons(atoi(OPT.tcpport));
    m.flags = 0;

    for (i = 0; i < ifd_length; i++) {
      rc = sendto(ifp[i].hs->sp_socket, &m, sizeof(m), 0, &(ifp[i].hs->sp_dest), sizeof(ifp[i].hs->sp_dest));
      if (rc == SOCKET_ERROR) {
	dbg(DBG_ERROR, "sendto(hs): %s\n", sock_error());
	goto err;
      }
    }

    dbg(DBG_ALL, "Sent announce msg %d to %s\n", count, FormatAddr(&(ifp[0].hs->sp_dest), addr, NI_MAXHOST));

    FD_ZERO(&readfds);
    
    for (i = 0; i < ifd_length; i++) {
      if (ifp[i].cs->sp_socket > maxSocket) {
	maxSocket = ifp[i].cs->sp_socket;
      }
      FD_SET(ifp[i].cs->sp_socket, &readfds);
    }
    
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    rc = select(maxSocket + 1, &readfds, NULL, NULL, &tv);
    if (rc < 0) {
      dbg(DBG_ERROR, "select(cs): %s\n", sock_error());
      goto err;
    } else if (rc == 0) {
      continue;
    } else {
      for (i = 0; i < ifd_length; i++) {
	if (FD_ISSET(ifp[i].cs->sp_socket, &readfds)) {
	  acceptlen = sizeof(struct sockaddr);
	  socket = accept(ifp[i].cs->sp_socket, &(ifp[i].cs->sp_dest), &acceptlen);
	  dbg(DBG_STATUS, "connected to %s\n", FormatAddr(&(ifp[i].cs->sp_dest), addr, NI_MAXHOST));
	  break;
	}
      }
      
      /* we have a connection */
      if (socket != INVALID_SOCKET) {
	break;
      }
    }
  }

  for (i = 0; i < ifd_length; i++) {
    FreePanel(ifp[i].hs);
    if (socket == ifp[i].cs->sp_socket) {
      DissociatePanel(ifp[i].cs);
    } else {
      FreePanel(ifp[i].cs);
    }
  } 

  return socket;

 err:
  for (i = 0; i < ifd_length || i < MAX_IF_LENGTH; i++) {
    FreePanel(ifp[i].hs);
    FreePanel(ifp[i].cs);
  }

  return INVALID_SOCKET;
}

SOCKET locate(const char *optrc) {
  PANEL* hs = NULL;
  PANEL* cs = NULL;

  IF_DATA ifd[MAX_IF_LENGTH];
  int ifd_length = MAX_IF_LENGTH;
  int i = 0;

  fd_set readfds;
  struct timeval tv;

  struct announce_msg m;
  
  char addr[NI_MAXHOST];

  socklen_t recvlen;

  SOCKET socket = INVALID_SOCKET;
  int rc = 0;

  ReadOptions(optrc);

  rc = populateInterfaceData(ifd, &ifd_length);
  if (rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "populateInterfaceData(): '%s'\n", sock_error());
    goto err;
  }

  hs = CreatePanel(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(!hs) {
    dbg(DBG_ERROR, "CreateBoundPanel(hs): %s\n", sock_error());
    goto err;
  }

  rc = BindPanel(hs, NULL, OPT.mcastport, 1);
  if(rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "BindPanel(hs), '%s'\n", sock_error());
    goto err;
  }

  rc = SetDestination(hs, OPT.mcastip, OPT.mcastport);
  if(rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "SetDestination(hs): %s\n", sock_error());
    goto err;
  }

  for (i = 0; i < ifd_length; i++) {
    rc = JoinMulticastGroup(hs, ifd[i].if_addr);
    if (rc == SOCKET_ERROR) {
      dbg(DBG_ERROR, "MakeMulticast(hs): %s\n", sock_error());
      goto err;
    }
  }
  rc = SetMulticastTTL(hs, OPT.mcastttl);
  if (rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "SetMulticastTTL(hs): %s\n", sock_error());
    goto err;
  }

  rc = SetMulticastLoopback(hs, 1);
  if (rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "SetMulticastLoopback(hs): %s\n", sock_error());
    goto err;
  }

  cs = CreateBoundPanel("0", AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(!cs) {
    dbg(DBG_ERROR, "CreateBoundPanel(cs): %s\n", sock_error());
    goto err;
  }

  tv.tv_sec = OPT.timeout;
  tv.tv_usec = 0;

  while(socket == INVALID_SOCKET) {
    FD_ZERO(&readfds);
    FD_SET(hs->sp_socket, &readfds);

    rc = select(hs->sp_socket + 1, &readfds, NULL, NULL, (OPT.timeout > 0 ? &tv : NULL));
    if (rc < 0) {
      dbg(DBG_ERROR, "select(hs): %s\n", sock_error());
      goto err;
    } else if (rc == 0) {
      socket = INVALID_SOCKET;
      break;
    }

    /* if we get here, select woke up with a valid packet */

    recvlen = sizeof(struct sockaddr);
    rc = recvfrom(hs->sp_socket, &m, sizeof(m), 0, &(cs->sp_dest), &recvlen);
    if (rc == SOCKET_ERROR) {
      dbg(DBG_ERROR, "recvfrom(hs): %s\n", sock_error());
      goto err;
    }

    m.magic = ntohl(m.magic);

    dbg(DBG_ALL, "received announce (magic=%d) from %s\n",
	m.magic, FormatAddr(&(cs->sp_dest), addr, NI_MAXHOST)); 

    if (m.magic == OPT.magicnum) {
      ((struct sockaddr_in *) &(cs->sp_dest))->sin_port = m.port;
      rc = connect(cs->sp_socket, &(cs->sp_dest), sizeof(struct sockaddr));
      if (rc == SOCKET_ERROR) {
	dbg(DBG_ERROR, "connect(cs): %s\n", sock_error());
	goto err;
      }
      else {
	dbg(DBG_STATUS, "connected to %s\n", FormatAddr(&(cs->sp_dest), addr, NI_MAXHOST));
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

int senddata(SOCKET socket, void* data, uint16_t length)
{
	int rc = 0;
	uint16_t net_length = htons(length);

	if (length <= 0) {
		set_error(EINVAL);
		return -1;
	}

	rc = send(socket, &net_length, sizeof(uint16_t), 0);
	if (rc == 0) {
		set_error(ECANCELED);
		return -1;
	} else if (rc < 0) {
		return -1;
	}

	rc = send(socket, data, length, 0);
	if (rc == 0) {
		set_error(ECANCELED);
		return -1;
	} else if (rc < 0) {
		return rc;
	}

	return rc;
}

int recvdata(SOCKET socket, void* data, uint16_t* length)
{
	int rc = 0;
	int recv_length = sizeof(uint16_t);
	uint16_t msg_length;
	uint16_t buffer;
	

	/* PEEK at the message length 
	 * necessary to verify that buffer is large enough
	 * and still allow receive of message after check
	 */
	rc = recv(socket, &msg_length, recv_length, MSG_PEEK);
	if (rc == 0) {
		set_error(ECANCELED);
		return -1;
	} else if (rc < 0) {
		return -1;
	} else if (rc != sizeof(uint16_t)) {
		set_error(ECANCELED);
		return -1;
	}

	msg_length = ntohs(msg_length);
	if (msg_length > *length) {
		set_error(ENOMEM);
		return -1;
	}
	
	/* remove the header from the socket stream
	 * necessary because we peeked at the message
	 * length before hand.
	 * NOTE: this header will not be removed
	 * if the buffer was not long enough
	 */
	recv_length = sizeof(uint16_t);
	rc = recv(socket, &buffer, recv_length, 0);
	if (rc == 0) {
		set_error(ECANCELED);
		return -1;
	} else if (rc < 0) {
		return -1;
	}

	/* receive the raw data
	 * now we actually retrieve the data
	 * and error out if we don't receive
	 * the same amount as specified
	 * in the msg_length
	 */
	rc = recv(socket, data, msg_length, 0);
	if (rc == 0) {
		set_error(ECANCELED);
		return -1;
	} else if (rc < 0) {
		return -1;
	} else if (rc != msg_length) {
		set_error(ECANCELED);
		return -1;
	}

	/* set the length to the actual received length */
	*length = msg_length;
	return 0;
}
