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
`	memset(p, 0, ps);
	p->sp_ibuf = (char *) malloc(SP_BUFSIZE);
	if (p->sp_ibuf == NULL) {
		fprintf(stderr, "Failed to allocate memory for input buffer.\n");
		return NULL;
	}
	p->sp_obuf = (char *) malloc(SP_BUFSIZE);
	if (p->sp_ibuf == NULL) {
		fprintf(stderr, "Failed to allocate memory for output buffer.\n");
		return NULL;
	}

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
	hints.ai_family = ((addr) ? af : AF_UNSPEC);
	hints.ai_socktype = type;
	hints.ai_protocol = proto;
	rc = getaddrinfo(addr, "0", &hints, &(p->sp_iface));
	if(rc != 0) {
		fprintf(stderr, "Failed to resolve remote address. %s\n", sock_error());
		FreePanel(p);
		return NULL;
	} else {
		PrintAddrinfo(p->sp_iface);
	}

	hints.ai_flags = ((addr) ? AI_PASSIVE : 0);
	hints.ai_family = ((addr) ? af : AF_UNSPEC);
	hints.ai_socktype = type;
	hints.ai_protocol = proto;
	rc = getaddrinfo(INADDR_ANY, port, &hints, &(p->sp_bind));
	if(rc != 0) {
		fprintf(stderr, "Failed to resolve host address. %s\n", sock_error());
		FreePanel(p);
		return NULL;
	} else {
		PrintAddrinfo(p->sp_bind);
	}

	// Save "intended" address family.
	p->sp_af = af;

	// Create the socket.
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
	p->sp_ibuf = (char *) malloc(SP_BUFSIZE);
	if (p->sp_ibuf == NULL) {
		fprintf(stderr, "Failed to allocate memory for input buffer.\n");
		return NULL;
	}
	p->sp_obuf = (char *) malloc(SP_BUFSIZE);
	if (p->sp_ibuf == NULL) {
		fprintf(stderr, "Failed to allocate memory for output buffer.\n");
		return NULL;
	}

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
	hints.ai_family = ((addr) ? af : AF_UNSPEC);
	hints.ai_socktype = type;
	hints.ai_protocol = proto;
	rc = getaddrinfo(addr, "0", &hints, &(p->sp_iface));
	if(rc != 0) {
		fprintf(stderr, "Failed to resolve remote address. %s\n", sock_error());
		FreePanel(p);
		return NULL;
	} else {
		PrintAddrinfo(p->sp_iface);
	}

	hints.ai_flags = ((addr) ? AI_PASSIVE : 0);
	hints.ai_family = ((addr) ? af : AF_UNSPEC);
	hints.ai_socktype = type;
	hints.ai_protocol = proto;
	rc = getaddrinfo(INADDR_ANY, port, &hints, &(p->sp_bind));
	if(rc != 0) {
		fprintf(stderr, "Failed to resolve host address. %s\n", sock_error());
		FreePanel(p);
		return NULL;
	} else {
		PrintAddrinfo(p->sp_bind);
	}

	// Save "intended" address family.
	p->sp_af = af;

	// Create the socket.
	p->sp_socket = socket(af, type, proto);
	if (p->sp_socket == INVALID_SOCKET) {
		fprintf(stderr, "Failed to create socket. %s\n", sock_error());
		FreePanel(p);
		return NULL;
	}
	// Set socket to reusable
	rc = setsockopt(p->sp_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof (optval));
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Unable to set socket to be reusable.  %s\n", sock_error());
		FreePanel(p);
		return NULL;
	}

	// Bind the socket
	rc = bind(p->sp_socket, (p->sp_bind->ai_addr), sizeof (p->sp_bind->ai_addr));
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Unable to bind socket to ");
		PrintSockaddr(p->sp_bind->ai_addr);
		fprintf(stderr, ". %s\n", sock_error());
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
		itoa(sin->sin_port, port, 6);
		rc = getaddrinfo(addr_s, port, &hints, &(p->sp_iface));
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to convert socket to IPv4 panel. %s\n", sock_error());
			return NULL;
		}
	} else if (addr->sa_family == AF_INET6) {
		// Create IPv6 addrinfo
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) addr;
		char addr_s[INET6_ADDRSTRLEN];
		inet_ntop(addr->sa_family, &(sin6->sin6_addr), addr_s, INET6_ADDRSTRLEN);
		itoa(sin6->sin6_port, port, 6);
		rc = getaddrinfo(addr_s, port, &hints, &(p->sp_iface));
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to convert socket to IPv6 panel. %s\n", sock_error());
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
		if(p->sp_ibuf) {
			free(p->sp_ibuf);
			p->sp_ibuf = NULL;
		}
		if(p->sp_obuf) {
			free(p->sp_obuf);
			p->sp_obuf = NULL;
		}
		free(p);
	}
	p = NULL;
}

/* Make an existing panel into a multicast panel by joining the multicast group
 * and setting the send interface.
 * * * */ 
int MakeMulticast(PANEL *p, int ttl) {
	struct ip_mreq		mreq;
	struct ipv6_mreq	mreq6;
	struct sockaddr		*sa = (p->sp_iface->ai_addr);
	struct sockaddr_in	*sin = (struct sockaddr_in *) sa;
	struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;
	int		level, 
			mc_opt, si_opt, 
			mc_len, si_len;
	char	*mc_val, *si_val;
	int		rc;

	if (p->sp_af == AF_INET) {
		level = IPPROTO_IP;
		// IPv4 multicast membership parameters
		mc_opt = IP_ADD_MEMBERSHIP;
		mc_val = (char *) &mreq;
		mc_len = sizeof (mreq);
		mreq.imr_multiaddr.s_addr = sin->sin_addr.s_addr;
		mreq.imr_interface.s_addr = INADDR_ANY;
		// IPv4 send interface parameters
		si_opt = IP_MULTICAST_IF;
		si_val = (char *) &(sin->sin_addr.s_addr);
		si_len = sizeof (sin->sin_addr.s_addr);
		// IPv4 multicast TTL
	} else if (p->sp_af == AF_INET6) {
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
		// IPv6 send interface parameters
		si_opt = IPV6_MULTICAST_IF;
		si_val = (char *) &(sin6->sin6_scope_id);
		si_len = sizeof (sin6->sin6_scope_id);
		// IPv6 multicast TTL
	} else {
		fprintf(stderr, "Invalid address family to join multicast group.\n");
		return SOCKET_ERROR;
	}

	rc = setsockopt(p->sp_socket, level, mc_opt, mc_val, mc_len);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to join multicast group. %s\n", sock_error());
		return SOCKET_ERROR;
	}
	rc = setsockopt(p->sp_socket, level, si_opt, si_val, si_len);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to set multicast send interface. %s\n", sock_error());
		return SOCKET_ERROR;
	}
	
	return SetMulticastTTL(p,ttl);
}

/* Set the time-to-live (TTL) value for a multicast panel.
 * * * */
int SetMulticastTTL(PANEL *p, int ttl) {
	int		level, ttl_opt, ttl_len;
	char	*ttl_val;
	int		rc;

	if (p->sp_af == AF_INET) {
		level = IPPROTO_IP;
		ttl_opt = IP_MULTICAST_TTL;
	} else if (p->sp_af == AF_INET6) {
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
}	p->sp_socket = socket(af, type, proto);
	if (p->sp_socket == INVALID_SOCKET) {
		fprintf(stderr, "Failed to create socket. %s\n", sock_error());
		FreePanel(p);
		return NULL;
	}
	// Set socket to reusable
	rc = setsockopt(p->sp_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof (optval));
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Unable to set socket to be reusable.  %s\n", sock_error());
		FreePanel(p);
		return NULL;
	}

	// Bind the socket
	rc = bind(p->sp_socket, (p->sp_bind->ai_addr), sizeof (p->sp_bind->ai_addr));
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Unable to bind socket to ");
		PrintSockaddr(p->sp_bind->ai_addr);
		fprintf(stderr, ". %s\n", sock_error());
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
		itoa(sin->sin_port, port, 6);
		rc = getaddrinfo(addr_s, port, &hints, &(p->sp_iface));
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to convert socket to IPv4 panel. %s\n", sock_error());
			return NULL;
		}
	} else if (addr->sa_family == AF_INET6) {
		// Create IPv6 addrinfo
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) addr;
		char addr_s[INET6_ADDRSTRLEN];
		inet_ntop(addr->sa_family, &(sin6->sin6_addr), addr_s, INET6_ADDRSTRLEN);
		itoa(sin6->sin6_port, port, 6);
		rc = getaddrinfo(addr_s, port, &hints, &(p->sp_iface));
		if (rc == SOCKET_ERROR) {
			fprintf(stderr, "Failed to convert socket to IPv6 panel. %s\n", sock_error());
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
		if(p->sp_ibuf) {
			free(p->sp_ibuf);
			p->sp_ibuf = NULL;
		}
		if(p->sp_obuf) {
			free(p->sp_obuf);
			p->sp_obuf = NULL;
		}
		free(p);
	}
	p = NULL;
}

/* Make an existing panel into a multicast panel by joining the multicast group
 * and setting the send interface.
 * * * */ 
int MakeMulticast(PANEL *p, int ttl) {
	struct ip_mreq		mreq;
	struct ipv6_mreq	mreq6;
	struct sockaddr		*sa = (p->sp_iface->ai_addr);
	struct sockaddr_in	*sin = (struct sockaddr_in *) sa;
	struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;
	int		level, 
			mc_opt, si_opt, 
			mc_len, si_len;
	char	*mc_val, *si_val;
	int		rc;

	if (p->sp_af == AF_INET) {
		level = IPPROTO_IP;
		// IPv4 multicast membership parameters
		mc_opt = IP_ADD_MEMBERSHIP;
		mc_val = (char *) &mreq;
		mc_len = sizeof (mreq);
		mreq.imr_multiaddr.s_addr = sin->sin_addr.s_addr;
		mreq.imr_interface.s_addr = INADDR_ANY;
		// IPv4 send interface parameters
		si_opt = IP_MULTICAST_IF;
		si_val = (char *) &(sin->sin_addr.s_addr);
		si_len = sizeof (sin->sin_addr.s_addr);
		// IPv4 multicast TTL
	} else if (p->sp_af == AF_INET6) {
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
		// IPv6 send interface parameters
		si_opt = IPV6_MULTICAST_IF;
		si_val = (char *) &(sin6->sin6_scope_id);
		si_len = sizeof (sin6->sin6_scope_id);
		// IPv6 multicast TTL
	} else {
		fprintf(stderr, "Invalid address family to join multicast group.\n");
		return SOCKET_ERROR;
	}

	rc = setsockopt(p->sp_socket, level, mc_opt, mc_val, mc_len);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to join multicast group. %s\n", sock_error());
		return SOCKET_ERROR;
	}
	rc = setsockopt(p->sp_socket, level, si_opt, si_val, si_len);
	if (rc == SOCKET_ERROR) {
		fprintf(stderr, "Failed to set multicast send interface. %s\n", sock_error());
		return SOCKET_ERROR;
	}
	
	return SetMulticastTTL(p,ttl);
}

/* Set the time-to-live (TTL) value for a multicast panel.
 * * * */
int SetMulticastTTL(PANEL *p, int ttl) {
	int		level, ttl_opt, ttl_len;
	char	*ttl_val;
	int		rc;

	if (p->sp_af == AF_INET) {
		level = IPPROTO_IP;
		ttl_opt = IP_MULTICAST_TTL;
	} else if (p->sp_af == AF_INET6) {
		level = IPPROTO_IPV6;
		ttl_opt = IPV6_MULTICAST_HOPS;
	} else {
		fprintf(stderr, "Invalid family to set multicast TTL.\n");
	}

	ttl_val = (char *) &ttl;
	ttl_len = sizeof (ttl);

	rc = setsockopt(p->sp_socket, level, ttl_opt, ttl_val, ttl_len);
	if (rc == SOCKET_ERROR) {
	        fprintf(stderr, "Failed to set multicast TTL. [%s]\n", sock_error());
		return SOCKET_ERROR;
	}

	return 0;
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
