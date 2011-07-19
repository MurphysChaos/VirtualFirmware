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

SOCKET announce(const char *optrc) {
  PANEL* hs = NULL;
  PANEL* cs = NULL;

  fd_set readfds;
  struct timeval tv;
  int num_packets = 0;
  int count = 0;

  char addr[NI_MAXHOST];

  struct announce_msg m;
  
  socklen_t acceptlen;

  SOCKET socket = INVALID_SOCKET;
  int rc = 0;

  ReadOptions(optrc);

  hs = CreateBoundPanel(OPT.mcastport, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(!hs) {
    dbg(DBG_ERROR, "CreateBoundPanel(hs): %s\n", sock_error()); 
    goto err;
  }

  rc = SetDestination(hs, OPT.mcastip, OPT.mcastport);
  if(rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "SetDesination(hs): %s\n", sock_error());
    goto err;
  }

  rc = MakeMulticast(hs);
  if (rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "MakeMulticast(hs): %s\n", sock_error());
    goto err;
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

  cs = CreateBoundPanel(OPT.tcpport, AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(!cs) {
    dbg(DBG_ERROR, "CreateBoundPanel(cs): %s\n", sock_error());
    goto err;
  }

  rc = listen(cs->sp_socket, 1);
  if(rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "listen(cs): %s\n", sock_error());
    goto err;
  }

  num_packets = OPT.timeout * PACKETS_PER_SEC;

  /* the funny condition causes us to loop forever
   * if we have a timeout of 0 */
  for(count = 0; (num_packets == 0 || count < num_packets); count++) {
    m.magic = htonl(OPT.magicnum);
    m.port = htons(atoi(OPT.tcpport));
    m.flags = 0;

    rc = sendto(hs->sp_socket, &m, sizeof(m), 0, &(hs->sp_dest), sizeof(hs->sp_dest));
    if (rc == SOCKET_ERROR) {
      dbg(DBG_ERROR, "sendto(hs): %s\n", sock_error());
      goto err;
    }

    dbg(DBG_ALL, "Sent announce msg %d to %s\n", count, FormatAddr(&(hs->sp_dest), addr, NI_MAXHOST));

    FD_ZERO(&readfds);
    FD_SET(cs->sp_socket, &readfds);
    
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    rc = select(cs->sp_socket + 1, &readfds, NULL, NULL, &tv);
    if (rc < 0) {
      dbg(DBG_ERROR, "select(cs): %s\n", sock_error());
      goto err;
    } else if (rc == 0) {
      continue;
    } else {
      acceptlen = sizeof(struct sockaddr);
      socket = accept(cs->sp_socket, &(cs->sp_dest), &acceptlen);
      dbg(DBG_STATUS, "connected to %s\n", FormatAddr(&(cs->sp_dest), addr, NI_MAXHOST));
      break;
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

SOCKET locate(const char *optrc) {
  PANEL* hs = NULL;
  PANEL* cs = NULL;

  fd_set readfds;
  struct timeval tv;

  struct announce_msg m;
  
  char addr[NI_MAXHOST];

  socklen_t recvlen;

  SOCKET socket = INVALID_SOCKET;
  int rc = 0;

  ReadOptions(optrc);

  hs = CreateBoundPanel(OPT.mcastport, AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(!hs) {
    dbg(DBG_ERROR, "CreateBoundPanel(hs): %s\n", sock_error());
    goto err;
  }

  rc = SetDestination(hs, OPT.mcastip, OPT.mcastport);
  if(rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "SetDestination(hs): %s\n", sock_error());
    goto err;
  }

  rc = MakeMulticast(hs);
  if (rc == SOCKET_ERROR) {
    dbg(DBG_ERROR, "MakeMulticast(hs): %s\n", sock_error());
    goto err;
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

int sendmsg_withlength(SOCKET socket, void* data, uint16_t length)
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

int recvmsg_withlength(SOCKET socket, void* data, uint16_t* length)
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
