#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <WinError.h>
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

/* Used as a helper function to create the "pre-zeroed" panel.
 * * * */
PANEL *GetEmptyPanel() {
	PANEL *p = NULL;
	size_t ps = sizeof (PANEL);

	// Prepare necessary memory
	p = (PANEL *) malloc(ps);
	if (p == NULL) {
		fprintf(stderr, "Failed to allocate memory for panel.\n");
		return NULL;
	}
	memset(p, 0, ps);

	return p;
}

/* The primary means of creating a panel.
 * * * */
PANEL *GetNewPanel(char *addr, char *port, int af, int type, int proto) {
	struct addrinfo hints;
	PANEL *p = GetEmptyPanel();
	size_t ps = sizeof (PANEL);
	int rc, optval = -1;

	// Prepare necessary memory
	memset(&hints, 0, sizeof(hints));

	// Resolve destination address
	hints.ai_flags = 0;
	hints.ai_family = af;
	hints.ai_socktype = type;
	hints.ai_protocol = proto;
	rc = getaddrinfo(addr, port, &hints, &(p->sp_iface));
	if(rc != 0) {
		fprintf(stderr, "Failed to resolve remote address. %s\n", wsa_strerror(WSAGetLastError()));
		FreePanel(p);
		return NULL;
	} else {
		PrintAddrinfo(p->sp_iface);
	}

	// Resolve binding address
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = af;
	hints.ai_socktype = type;
	hints.ai_protocol = proto;
	rc = getaddrinfo(NULL, port, &hints, &(p->sp_bind));
	if(rc != 0) {
		fprintf(stderr, "Failed to resolve host address. %s\n", wsa_strerror(WSAGetLastError()));
		FreePanel(p);
		return NULL;
	} else {
		PrintAddrinfo(p->sp_bind);
	}

	// Create the socket.
	p->sp_socket = socket(af, type, proto);
	if (p->sp_socket == INVALID_SOCKET) {
		fprintf(stderr, "Failed to create socket. %s\n", wsa_strerror(WSAGetLastError()));
		FreePanel(p);
		return NULL;
	}
	// Set socket to reusable
	rc = setsockopt(p->sp_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof (optval));
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Unable to set socket to be reusable.  %s\n", wsa_strerror(WSAGetLastError()));
		FreePanel(p);
		return NULL;
	}

	// Bind the socket
	rc = bind(p->sp_socket, (p->sp_bind->ai_addr), sizeof *(p->sp_bind->ai_addr));
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Unable to bind socket to ");
		PrintSockaddr(p->sp_bind->ai_addr);
		fprintf(stderr, ". %s\n", wsa_strerror(WSAGetLastError()));
		FreePanel(p);
		return NULL;
	}
	
	return p;
}

/* To be called after an accept to insert a socket into a panel.
 * * * */
PANEL *GetNewPanelWithSocket(int s, struct sockaddr *addr) {
	struct addrinfo hints;
	PANEL *p = GetEmptyPanel();
	size_t ps = sizeof (PANEL);
	char port[6];
	int rc;

	// Prepare necessary memory, determine comman hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = 0;
	hints.ai_family = addr->sa_family;

	if (addr->sa_family == AF_INET) {
		// Create IPv4 addrinfo
		struct sockaddr_in *sin = (struct sockaddr_in *) addr;
		char addr_s[INET_ADDRSTRLEN];
		inet_ntop(addr->sa_family, &(sin->sin_addr), addr_s, INET_ADDRSTRLEN);
		sprintf(port, "%u", sin->sin_port);
		rc = getaddrinfo(addr_s, port, &hints, &(p->sp_iface));
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to convert socket to IPv4 panel. %s\n", wsa_strerror(WSAGetLastError()));
			return NULL;
		}
	} else if (addr->sa_family == AF_INET6) {
		// Create IPv6 addrinfo
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) addr;
		char addr_s[INET6_ADDRSTRLEN];
		inet_ntop(addr->sa_family, &(sin6->sin6_addr), addr_s, INET6_ADDRSTRLEN);
		sprintf(port, "%u", sin6->sin6_port);
		rc = getaddrinfo(addr_s, port, &hints, &(p->sp_iface));
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to convert socket to IPv6 panel. %s\n", wsa_strerror(WSAGetLastError()));
			return NULL;
		}
	} else {
		fprintf(stderr, "Incoming connection of unsupported family.\n");
		FreePanel(p);
		return NULL;
	}

	return p;
}

/* Nicely cleans up after an unused panel.
 * * * */
void FreePanel(PANEL *p) {
	if(p) {
		if(p->sp_iface) {
			freeaddrinfo(p->sp_iface);
			p->sp_iface = NULL;
		}
		if(p->sp_bind) {
			freeaddrinfo(p->sp_bind);
			p->sp_bind = NULL;
		}
		free(p);
	}
	p = NULL;
}

/* Make an existing panel into a multicast panel by joining the multicast group
 * and setting the send interface.
 * * * */ 
int MakeMulticast(PANEL *p) {
	struct ip_mreq		mreq;
	struct ipv6_mreq	mreq6;
	struct sockaddr		*sa = (p->sp_iface->ai_addr);
	struct sockaddr_in	*sin = (struct sockaddr_in *) sa;
	struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;
	int		level, 
			mc_opt, 
			mc_len;
	char	*mc_val;
	int		rc;

	if (p->sp_iface->ai_family == AF_INET) {
		level = IPPROTO_IP;
		// IPv4 multicast membership parameters
		mc_opt = IP_ADD_MEMBERSHIP;
		mc_val = (char *) &mreq;
		mc_len = sizeof (mreq);
		mreq.imr_multiaddr.s_addr = sin->sin_addr.s_addr;
		mreq.imr_interface.s_addr = INADDR_ANY;
		// IPv4 multicast TTL
	} else if (p->sp_iface->ai_family == AF_INET6) {
		level = IPPROTO_IPV6;
		// IPv6 multicast membership parameters
		mc_opt = IPV6_ADD_MEMBERSHIP;
		mc_val = (char *) &mreq6;
		mc_len = sizeof (mreq6);
#ifdef _WIN32
		sin6->sin6_scope_id = ScopeLevelSubnet;
#endif
		mreq6.ipv6mr_multiaddr = sin6->sin6_addr;
		mreq6.ipv6mr_interface = sin6->sin6_scope_id;
		// IPv6 multicast TTL
	} else {
		fprintf(stderr, "Invalid address family to join multicast group.\n");
		return SOCKET_ERROR;
	}

	// Join multicast group
	rc = setsockopt(p->sp_socket, level, mc_opt, mc_val, mc_len);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to join multicast group. %s\n", wsa_strerror(WSAGetLastError()));
		return SOCKET_ERROR;
	}

	return rc;
}

int SetMulticastSendInterface(PANEL *p, struct sockaddr *addr) {
	int		level, 
			si_opt, 
			si_len;
	char	*si_val;
	int		rc;

	// IPv4 send interface parameters
	if (addr->sa_family == AF_INET) {
		si_opt = IP_MULTICAST_IF;
		si_val = (char *) &((struct sockaddr_in *) addr)->sin_addr.s_addr;
		si_len = sizeof (((struct sockaddr_in *) addr)->sin_addr.s_addr);
	} else if (addr->sa_family == AF_INET6) {
		// IPv6 send interface parameters
		si_opt = IPV6_MULTICAST_IF;
		si_val = (char *) ((struct sockaddr_in6 *) addr)->sin6_scope_id;
		si_len = sizeof (((struct sockaddr_in6 *) addr)->sin6_scope_id);
	} else {
		fprintf(stderr, "Invalid address family to set send interface.\n");
		return SOCKET_ERROR;
	}
	rc = setsockopt(p->sp_socket, level, si_opt, si_val, si_len);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to set multicast send interface. %s\n", wsa_strerror(WSAGetLastError()));
		return SOCKET_ERROR;
	}
}

/* Set the time-to-live (TTL) value for a multicast panel.
 * * * */
int SetMulticastTTL(PANEL *p, int ttl) {
	int		level, ttl_opt, ttl_len;
	char	*ttl_val;
	int		rc;

	if (p->sp_iface->ai_family == AF_INET) {
		level = IPPROTO_IP;
		ttl_opt = IP_MULTICAST_TTL;
	} else if (p->sp_iface->ai_family == AF_INET6) {
		level = IPPROTO_IPV6;
		ttl_opt = IPV6_MULTICAST_HOPS;
	} else {
		fprintf(stderr, "Invalid family to set multicast TTL.\n");
	}

	ttl_val = (char *) &ttl;
	ttl_len = sizeof (ttl);

	rc = setsockopt(p->sp_socket, level, ttl_opt, ttl_val, ttl_len);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to set multicast TTL. [%d]\n",WSAGetLastError());
		return SOCKET_ERROR;
	}

	return 0;
}

int SetMulticastLoopback(PANEL *p, int loopval)
{
    char *optval=NULL;
    int   optlevel = 0,
          option = 0,
          optlen = 0,
          rc;

    rc = NO_ERROR;
	if (p->sp_iface->ai_family == AF_INET)
    {
        // Set the v4 options
        optlevel = IPPROTO_IP;
        option   = IP_MULTICAST_LOOP;
        optval   = (char *) &loopval;
        optlen   = sizeof(loopval);
    }
    else if (p->sp_iface->ai_family == AF_INET6)
    {
        // Set the v6 options
        optlevel = IPPROTO_IPV6;
        option   = IPV6_MULTICAST_LOOP;
        optval   = (char *) &loopval;
        optlen   = sizeof(loopval);
    }
    else
    {
        fprintf(stderr, "Invalid address family for multicast loopback.\n");
        rc = SOCKET_ERROR;
    }
    if (rc != SOCKET_ERROR)
    {
        // Set the multpoint loopback
		rc = setsockopt(p->sp_socket,optlevel,option,optval,optlen);
        if (rc == SOCKET_ERROR)
        {
            fprintf(stderr, "SetMulticastLoopBack: setsockopt failed. %s\n", wsa_strerror(WSAGetLastError()));
        }
    }
    return rc;
}
/* Displays information on a panel to stdout.
 * * * */
void PrintPanel(PANEL *p) {
	printf("PANEL:\n  Socket ID: %d\n",p->sp_socket);
	if (p->sp_iface) {
		printf("Target:\n");
		PrintAddrinfo(p->sp_iface);
	}
	if (p->sp_bind) {
		printf("Bind:\n");
		PrintAddrinfo(p->sp_bind);
	}
}

void PrintAddrinfo(struct addrinfo *ai) {
	struct addrinfo *p = ai;

	if (ai) {
		while (p != NULL) {
			printf("  Address: ");
			PrintSockaddr(ai->ai_addr);
			switch (ai->ai_family) {
			case AF_INET:
				printf(", Family: AF_INET");
				break;
			case AF_INET6:
				printf(", Family: AF_INET6");
				break;
			default:
				printf(", Family: UNKNOWN");
				break;
			}
			switch (ai->ai_protocol) {
			case IPPROTO_IP:
				printf(", Protocol: IPPROTO_IP");
				break;
			case IPPROTO_IPV6:
				printf(", Protocol: IPPROTO_IPV6");
				break;
			case IPPROTO_TCP:
				printf(", Protocol: IPPROTO_TCP");
				break;
			case IPPROTO_UDP:
				printf(", Protocol: IPPROTO_UDP");
				break;
			default:
				printf(", Protocol: UNKNOWN");
				break;
			}
			printf("\n");
			p = p->ai_next;
		}
	} else {
		printf("No address information (null pointer).");
	}
}

void PrintSockaddr(struct sockaddr *sa) {
	char buf[NI_MAXHOST];
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;

	switch (sa->sa_family) {
	case AF_UNSPEC:
	case AF_INET:
		// struct sockaddr_in
		sin = (struct sockaddr_in *) sa;
		inet_ntop(sin->sin_family, &(sin->sin_addr), buf, NI_MAXHOST);
		printf("%s:%d", buf, ntohs(sin->sin_port));
		break;
	case AF_INET6:
		// struct sockaddr_in6
		sin6 = (struct sockaddr_in6 *) sa;
		inet_ntop(sin6->sin6_family, &(sin6->sin6_addr), buf, NI_MAXHOST);
		printf("%s/%d", buf, ntohs(sin6->sin6_port));
		break;
	default:
		printf("Unknown Address Format");
	}
}