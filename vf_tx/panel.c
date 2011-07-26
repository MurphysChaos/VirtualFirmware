/* 
 * panel.c -- Panels are a conceptual extension of a socket. They are 
 * intended to assist in the creation of socket programs by reducing 
 * the amount of code the author must write and by reducing code differences
 * between Windows and Linux.
 *
 * Copyright (C) 2011 Joel Murphy, Jacob Keller, All rights reserved
 * The code in this file is licensed under GPL v2. This license does not 
 * extend to any file bundled with this one.
 */

#include "panel.h"

/* Used as a helper function to create the "pre-zeroed" panel.
 */
PANEL *CreateEmptyPanel(void) {
	PANEL *p = NULL;
	size_t ps = sizeof (PANEL);

	// Prepare necessary memory
	p = (PANEL *) malloc(ps);
	if (p == NULL) {
		return NULL;
	}
	memset(p, 0, ps);

	return p;
}

/* Creates a panel with only an active socket. Useful for most of your 
 * socket-panel needs.
 */
PANEL *CreatePanel(int af, int type, int proto) {
	PANEL *p = CreateEmptyPanel();

	// Create the socket.
	p->sp_socket = socket(af, type, proto);
	if (p->sp_socket == INVALID_SOCKET) {
		FreePanel(p);
		return NULL;
	}
	
	p->sp_family = af;
	p->sp_socktype = type;
	p->sp_protocol = proto;
	p->sp_flags = sp_setflag(p->sp_flags, SP_F_VALID);

	return p;
}

/* Creates a panel that is pre-bound to an outgoing port. The contained 
 * socket will listen on all active interfaces. Use CreatePanel() then 
 * BindPanel() to listen on a specific interface.
 */
PANEL *CreateBoundPanel(const char *svc, int af, int type, int proto) {
	PANEL *p = CreatePanel(af, type, proto);
	int rc;

	// Bind socket to address
	rc = BindPanel(p, INADDR_ANY, svc, 1);
	if (rc == SOCKET_ERROR) {
		FreePanel(p);
		return NULL;
	}

	return p;
}

/* To be called after an accept to insert a socket into a panel.
 */
PANEL *SocketToPanel(int s, struct sockaddr *addr) {
	PANEL *p = CreateEmptyPanel();

	// Copy received data
	p->sp_socket = s;
	memcpy(&(p->sp_bind), addr, sizeof (addr));
	p->sp_flags = sp_setflag(p->sp_flags, SP_F_VALID);
    
	return p;
}

/* Nicely cleans up after an unused panel.
 */
void FreePanel(PANEL *p) {
    int rc;

	if (p) {
#ifdef _WIN32
        shutdown(p->sp_socket, SD_BOTH);
		closesocket(p->sp_socket);
#else
		close(p->sp_socket);
#endif
		free(p);
	}
}

/* Nicely delets panel information but leaves socket intact*/
void DissociatePanel(PANEL* p) {
	if (p) {
		free(p);
	}
}

/* Resolves a general address and service specifier into a net-standard 
 * internet address and port. The service an be a generalized service 
 * specifier (e.g. "http", "ftp", "smtp", etc.) or a specific port.
 * Returns NULL on error. Use freeaddrinfo() on the return result after 
 * you are done with it.
 */
struct addrinfo *ResolveAddr(const char *addr, const char *svc, int af, int type, int proto) {
	struct addrinfo hints, *res = NULL;
	int rc;

	memset(&hints, 0, sizeof (hints));
	hints.ai_flags = ((addr) ? 0 : AI_PASSIVE);
	hints.ai_family = af;
	hints.ai_socktype = type;
	hints.ai_protocol = proto;

	rc = getaddrinfo(addr, svc, &hints, &res);
	if (rc == 0) {
		return res;
	} else {
		return NULL;
	}
}

/* Causes a panel to bind to the stored bind address. Must be used on a 
 * valid panel.
 */
int BindPanel(PANEL *p, const char *addr, const char *svc, int reuse) {
	struct addrinfo *ai = NULL;
	int rc;

	if (sp_getflag(p->sp_flags, SP_F_VALID)) {
		// Resolve bind address
		ai = ResolveAddr(addr, svc, p->sp_family, p->sp_socktype, p->sp_protocol);
		if (ai == NULL)
			return SOCKET_ERROR;
		memcpy(&(p->sp_bind), ai->ai_addr, ai->ai_addrlen);
		freeaddrinfo(ai);
	} else {
		set_error(EPERM); // Operation not permitted.
		return SOCKET_ERROR;
	}

	// Set socket to reusable
	rc = setsockopt(p->sp_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof (reuse));
	if (rc == SOCKET_ERROR) {
		return rc;
	}
	
	// Bind the socket
	sp_setflag(p->sp_flags,SP_F_LISTENER);
	rc = bind(p->sp_socket, &(p->sp_bind), sizeof(p->sp_bind));
	return rc;
}

/* Aligns outgoing traffic for the panel. Must be used on a valid panel.
 */
int SetDestination(PANEL *p, const char *addr, const char *svc) {
	struct addrinfo *ai = NULL;

	if (sp_getflag(p->sp_flags, SP_F_VALID)) {
		// Resolve destination address
		ai = ResolveAddr(addr, svc, p->sp_family, p->sp_socktype, p->sp_protocol);
		if (ai == NULL)
			return SOCKET_ERROR;
		memcpy(&(p->sp_dest), ai->ai_addr, ai->ai_addrlen);
		freeaddrinfo(ai);
	} else {
		set_error(EPERM); // Operation not permitted.
		return SOCKET_ERROR;
	}

	sp_setflag(p->sp_flags,SP_F_SENDER);
	return NO_ERROR;
}

/* Sets the socket option with the linger structure. This doesn't work in 
 * Windows.
 */
int SetOptionLinger(PANEL *p, uint16_t opt_yesno, uint16_t opt_delay) {
    int level, option, optlen;
    struct linger optval;
    int rc;

    optlen = sizeof(optval);
    memset((void *) &optval, 0, optlen);

    level = SOL_SOCKET;
    option = SO_LINGER;
    optval.l_onoff = opt_yesno;
    optval.l_linger = opt_delay;

    rc = setsockopt(p->sp_socket, level, option, &optval, optlen);
    return rc;
}

/* Used to set any true or false option
 */
int SetOption(PANEL *p, int level, int option, int optval) {
    int optlen = sizeof(optval);
    int rc;

    rc = setsockopt(p->sp_socket, level, option, (const char *) &optval, optlen);
    return rc;
}

/* Make an existing panel into a multicast panel by joining the multicast group
 * and setting the send interface. Uses p->sp_dest which must be set prior to 
 * calling.
 */
int JoinMulticastGroup(PANEL *p, const char *ifaddr) {
	int level, option, optlen;
	char *optval;
	int rc;

	if ((p->sp_dest.sa_family == AF_INET) || (p->sp_dest.sa_family == AF_UNSPEC)) {
		struct ip_mreq mreq;
		struct sockaddr_in *sin = (struct sockaddr_in *) &(p->sp_dest);
		struct sockaddr_in *sbind = (struct sockaddr_in *) &(p->sp_bind);

		// IPv4 multicast membership parameters
		level = IPPROTO_IP;
		option = IP_ADD_MEMBERSHIP;
		optval = (char *) &mreq;
		optlen = sizeof (mreq);
		mreq.imr_multiaddr.s_addr = sin->sin_addr.s_addr;
		if (ifaddr) {
			inet_pton(AF_INET,  ifaddr, &mreq.imr_interface.s_addr);
		} else {
			mreq.imr_interface.s_addr = sbind->sin_addr.s_addr;
		}
	} else if (p->sp_dest.sa_family == AF_INET6) {
		struct ipv6_mreq mreq6;
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &(p->sp_dest);

		// IPv6 multicast membership parameters
		level = IPPROTO_IPV6;
		option = IPV6_ADD_MEMBERSHIP;
		optval = (char *) &mreq6;
		optlen = sizeof (mreq6);
#ifdef _WIN32
		sin6->sin6_scope_id = ScopeLevelSubnet;
#else
		sin6->sin6_scope_id = 0;
#endif
		mreq6.ipv6mr_multiaddr = sin6->sin6_addr;
		mreq6.ipv6mr_interface = sin6->sin6_scope_id;
	} else {
		set_error(EAFNOSUPPORT); // Address family not supported
		return SOCKET_ERROR;
	}

	// Join multicast group
	rc = setsockopt(p->sp_socket, level, option, optval, optlen);
	if (rc == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	sp_setflag(p->sp_flags,SP_F_MULTICAST);
	return rc;
}

/* Gracefully leave the multicast group.
 */
int LeaveMulticastGroup(PANEL *p, const char *ifaddr) {
	int level, option, optlen;
	char *optval;
	int rc;

	if ((p->sp_dest.sa_family == AF_INET) || (p->sp_dest.sa_family == AF_UNSPEC)) {
		struct ip_mreq mreq;
		struct sockaddr_in *sin = (struct sockaddr_in *) &(p->sp_dest);
		struct sockaddr_in *sbind = (struct sockaddr_in *) &(p->sp_bind);

		// IPv4 multicast membership parameters
		level = IPPROTO_IP;
		option = IP_DROP_MEMBERSHIP;
		optval = (char *) &mreq;
		optlen = sizeof (mreq);
		mreq.imr_multiaddr.s_addr = sin->sin_addr.s_addr;
		if (ifaddr) {
			inet_pton(AF_INET,  ifaddr, &mreq.imr_interface.s_addr);
		} else {
			mreq.imr_interface.s_addr = sbind->sin_addr.s_addr;
		}
	} else if (p->sp_dest.sa_family == AF_INET6) {
		struct ipv6_mreq mreq6;
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &(p->sp_dest);

		// IPv6 multicast membership parameters
		level = IPPROTO_IPV6;
		option = IPV6_DROP_MEMBERSHIP;
		optval = (char *) &mreq6;
		optlen = sizeof (mreq6);
#ifdef _WIN32
		sin6->sin6_scope_id = ScopeLevelSubnet;
#else
		sin6->sin6_scope_id = 0;
#endif
		mreq6.ipv6mr_multiaddr = sin6->sin6_addr;
		mreq6.ipv6mr_interface = sin6->sin6_scope_id;
	} else {
		set_error(EAFNOSUPPORT); // Address family not supported
		return SOCKET_ERROR;
	}

	// Join multicast group
	rc = setsockopt(p->sp_socket, level, option, optval, optlen);
	if (rc == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	sp_clearflag(p->sp_flags,SP_F_MULTICAST);
	return rc;
}

/* Override the default interface to send multicast messages from. If this 
 * is not called, the system default is used.
 */
int SetMulticastSendInterface(PANEL *p, struct sockaddr *addr) {
	int optlvl, option, optlen;
	char *optval;
	int rc;

	// IPv4 send interface parameters
	if (addr->sa_family == AF_INET) {
		optlvl = IPPROTO_IP;
		option = IP_MULTICAST_IF;
		optval = (char *) &((struct sockaddr_in *) addr)->sin_addr.s_addr;
		optlen = sizeof (((struct sockaddr_in *) addr)->sin_addr.s_addr);
	} else if (addr->sa_family == AF_INET6) {
		// IPv6 send interface parameters
		optlvl = IPPROTO_IPV6;
		option = IPV6_MULTICAST_IF;
		optval = (char *) &(((struct sockaddr_in6 *) addr)->sin6_scope_id);
		optlen = sizeof (((struct sockaddr_in6 *) addr)->sin6_scope_id);
	} else {
		set_error(EAFNOSUPPORT); // Address family not supported
		return SOCKET_ERROR;
	}
	rc = setsockopt(p->sp_socket, optlvl, option, optval, optlen);
	if (rc == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	return rc;
}

/* Set the time-to-live (TTL) value for a multicast panel.
 */
int SetMulticastTTL(PANEL *p, int ttl) {
	int level, ttl_opt, ttl_len;
	char *ttl_val;
	int rc;

	if (p->sp_dest.sa_family == AF_INET) {
		level = IPPROTO_IP;
		ttl_opt = IP_MULTICAST_TTL;
	} else if (p->sp_dest.sa_family == AF_INET6) {
		level = IPPROTO_IPV6;
		ttl_opt = IPV6_MULTICAST_HOPS;
	} else {
		return SOCKET_ERROR;
	}

	ttl_val = (char *) &ttl;
	ttl_len = sizeof (ttl);

	rc = setsockopt(p->sp_socket, level, ttl_opt, ttl_val, ttl_len);
	if (rc == SOCKET_ERROR) {
		set_error(EAFNOSUPPORT); // Address family not supported
		return SOCKET_ERROR;
	}

	return 0;
}

/* Sets whether multicast packets, sent by a given interface, are processed by 
 * the originating host. Loopback is on by default.
 */
int SetMulticastLoopback(PANEL *p, int loopval) {
	char *optval = NULL;
	int optlevel = 0,
		option = 0,
		optlen = 0,
		rc;

	rc = NO_ERROR;
	if (p->sp_dest.sa_family == AF_INET) {
		// Set the v4 options
		optlevel = IPPROTO_IP;
		option = IP_MULTICAST_LOOP;
		optval = (char *) &loopval;
		optlen = sizeof (loopval);
	} else if (p->sp_dest.sa_family == AF_INET6) {
		// Set the v6 options
		optlevel = IPPROTO_IPV6;
		option = IPV6_MULTICAST_LOOP;
		optval = (char *) &loopval;
		optlen = sizeof (loopval);
	} else {
		set_error(EAFNOSUPPORT); // Address family not supported
		return SOCKET_ERROR;
	}

	// Set the multpoint loopback
	rc = setsockopt(p->sp_socket, optlevel, option, optval, optlen);
	if (rc == SOCKET_ERROR) {
		return SOCKET_ERROR;
	}

	return NO_ERROR;
}

/* Turns an address into presentation format. The return value is the pointer
 * to buf.
 */
char *FormatAddr(struct sockaddr *sa, char *buf, int buflen) {
	struct sockaddr_in *sin = (struct sockaddr_in *) sa;
	struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;

	if ((sa->sa_family == AF_UNSPEC) || (sa->sa_family == AF_INET)) {
		inet_ntop(sin->sin_family, &(sin->sin_addr), buf, buflen);
		sprintf(buf, "%s:%d", buf, ntohs(sin->sin_port));
	} else if (sa->sa_family == AF_INET6) {
		inet_ntop(sin6->sin6_family, &(sin6->sin6_addr), buf, buflen);
		sprintf(buf, "%s/%d", buf, ntohs(sin6->sin6_port));
	} else {
		sprintf(buf, "<unsupported address format>");
	}

	return buf;
}

/* Print information on an entire panel.
 */
void PrintPanel(FILE *f, PANEL *p) {
	char buf[NI_MAXHOST];
	fprintf(f, "PANEL:\n");
	fprintf(f, "  Socket ID: %d\n", p->sp_socket);
	fprintf(f, "  Destination: %s", FormatAddr(&(p->sp_dest),buf,NI_MAXHOST));
	PrintAddrFamily(f, p->sp_dest.sa_family);
	fprintf(f, "\n  Bind: %s", FormatAddr(&(p->sp_bind),buf,NI_MAXHOST));
	PrintAddrFamily(f, p->sp_bind.sa_family);
	fprintf(f, "\n");
}

/* Prints address family value */
void PrintAddrFamily(FILE *f, int af) {
	switch (af) {
        case AF_INET:
		fprintf(f, ", Family: AF_INET");
		break;
        case AF_INET6:
		fprintf(f, ", Family: AF_INET6");
		break;
        default:
		fprintf(f, ", Family: UNKNOWN");
		break;
	}
}

void PrintAddrProtocol(FILE *f, int proto) {
	switch (proto) {
        case IPPROTO_IP:
		fprintf(f, ", Protocol: IPPROTO_IP");
		break;
        case IPPROTO_IPV6:
		fprintf(f, ", Protocol: IPPROTO_IPV6");
		break;
        case IPPROTO_TCP:
		fprintf(f, ", Protocol: IPPROTO_TCP");
		break;
        case IPPROTO_UDP:
		fprintf(f, ", Protocol: IPPROTO_UDP");
		break;
        default:
		fprintf(f, ", Protocol: UNKNOWN");
		break;
	}
}

/* Print all addresses in an Addrinfo linked list.
 */
void PrintAddrinfo(FILE *f, struct addrinfo *ai) {
	struct addrinfo *p = ai;

	if (ai) {
		while (p != NULL) {
			fprintf(f, "  Address: ");
			PrintAddr(f, ai->ai_addr);
			PrintAddrFamily(f, ai->ai_family);
			PrintAddrProtocol(f, ai->ai_protocol);
			fprintf(f, "\n");
			p = p->ai_next;
		}
	} else {
		fprintf(f, "No address information (null pointer).");
	}
}

/* Print information on an address.
 */
void PrintAddr(FILE* f, struct sockaddr *sa) {
	char buf[NI_MAXHOST];

	fprintf(f, "%s", FormatAddr(sa, buf, NI_MAXHOST));
	fflush(f);
}
