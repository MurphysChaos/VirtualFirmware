#include <sys/types.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

#define INVALID_SOCKET -1
#define MCAST_ADDR "234.5.6.7"
#define MCAST_PORT "25000"
#define TCP_PORT "25001"
#define DEFAULT_TTL 0

struct announce {
  int magic;
  unsigned short port;
  unsigned short reserved;
};

int getPort(struct addrinfo* a) {
  char serv[NI_MAXSERV];
  int servlen = NI_MAXSERV;
  int ret;


  ret = getnameinfo(a->ai_addr, a->ai_addrlen, NULL, 0, serv, servlen, 0);
  if(ret != 0) {
    return 0;
  } else {
    return atoi(serv);
  }
}

int PrintAddress(struct sockaddr* sa, int salen)
{
  char	host[NI_MAXHOST],
        serv[NI_MAXSERV];
  int	hostlen = NI_MAXHOST,
        servlen = NI_MAXSERV,
        rc;
        
  // Validate argument
  if (sa == NULL)
    return -1;
    
  rc = getnameinfo(sa, salen, host, hostlen, serv, servlen, NI_NUMERICHOST | NI_NUMERICSERV);
  if (rc != 0)
  {
    fprintf(stderr, "%s: getnameinfo failed: %d\n", __FILE__, rc);
    return rc;
  }
  
  // If the port is zero then don't print it
  if (strncmp(serv, "0", 1) != 0)
  {
    if (sa->sa_family == AF_INET6)
      printf("[%s]:%s", host, serv);
    else
      printf("%s:%s", host, serv);
  }
  else
    printf("%s", host);
    
  return 0;
}

struct addrinfo* ResolveAddress(char* addr, char* port, int family, int sockType, int protocol) {
  struct addrinfo hints;
  struct addrinfo* res = NULL;

  int ret = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = ((addr) ? 0 : AI_PASSIVE);
  hints.ai_family = family;
  hints.ai_socktype = sockType;
  hints.ai_protocol = protocol;
  
  ret = getaddrinfo( addr, port, &hints, &res );
  if (ret != 0)
  {
    fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret));
    exit(1);
  }
  
  return res;
}

/*
  JoinMulticastGroup -- Joins the multicast socket, on the specified multicast
  group. The interface is the IP address for IPv4 and the scope ID for IPv6.
*/
int JoinMulticastGroup(int s, struct addrinfo *group, struct addrinfo *iface)
{
  struct ip_mreq	mreqv4;
  struct ipv6_mreq	mreqv6;
  char			*optval = NULL;
  int			optlevel = 0,
                        option = 0,
                        optlen = 0,
                        ret = 0,
                        errno;
  
  if (group->ai_family == AF_INET)
  {
    // Setup the v4 option values andip_mreq structure
    optlevel = IPPROTO_IP;
    option   = IP_ADD_MEMBERSHIP;
    optval   = (char *)& mreqv4;
    optlen   = sizeof(mreqv4);
    
    mreqv4.imr_multiaddr.s_addr = ((struct sockaddr_in *)group->ai_addr)->sin_addr.s_addr;
    mreqv4.imr_interface.s_addr = ((struct sockaddr_in *)iface->ai_addr)->sin_addr.s_addr;
  }
  else if (group->ai_family == AF_INET6)
  {
    // Setup the v6 option values and ipv6_mreq structure
    optlevel = IPPROTO_IPV6;
    option   = IPV6_ADD_MEMBERSHIP;
    optval   = (char *)& mreqv6;
    optlen   = sizeof(mreqv6);
    
    mreqv6.ipv6mr_multiaddr = ((struct sockaddr_in6 *)group->ai_addr)->sin6_addr;
    mreqv6.ipv6mr_interface = ((struct sockaddr_in6 *)iface->ai_addr)->sin6_scope_id;
  }
  else
  {
    fprintf(stderr, "ERROR: Invalid address family for joining multicast group.\n");
    ret = -1;
  }
  
  if (ret != -1)
  {
    // Join the group
    ret = setsockopt(s, optlevel, option, optval, optlen);
    
    if (ret == -1)
    {
      fprintf(stderr, "ERROR #%d: setsockopt failed in JoinMulticastGroup.\n", errno);
    }
    else {
      printf("Joined group: ");
      PrintAddress(group->ai_addr, (int) group->ai_addrlen);
      printf("\n");
    }
  }
  
  return ret;
}

/*
  SetSendInterface -- Sets the outgoing interface of the socket. The 
  interface is the IP address for IPv4 and the scope ID for IPv6.
*/
int SetSendInterface(int s, struct addrinfo *iface)
{
  char	*optval = NULL;
  int	optlevel = 0,
        option = 0,
        optlen = 0,
        ret = 0,
        errno;
  
  if (iface->ai_family == AF_INET)
  {
    // Setup the v4 option values
    optlevel = IPPROTO_IP;
    option   = IP_MULTICAST_IF;
    optval   = (char *) &((struct sockaddr_in *)iface->ai_addr)->sin_addr.s_addr;
    optlen   = sizeof(((struct sockaddr_in *)iface->ai_addr)->sin_addr.s_addr);
  }
  else if (iface->ai_family == AF_INET6)
  {
    // Setup the v6 option values
    optlevel = IPPROTO_IPV6;
    option   = IPV6_MULTICAST_IF;
    optval   = (char *) &((struct sockaddr_in6 *)iface->ai_addr)->sin6_scope_id;
    optlen   = sizeof(((struct sockaddr_in6 *)iface->ai_addr)->sin6_scope_id);
  }
  else
  {
    fprintf(stderr, "ERROR: Invalid address family for sending interface.\n");
    ret = -1;
  }
  
  // Set send IF
  if (ret != -1)
  {
    // Set the send interface
    ret = setsockopt(s, optlevel, option, optval, optlen);
    if (ret == -1)
    {
      fprintf(stderr, "ERROR #%d: setsockopt failed in SetSendInterface.\n", errno);
    }
  }
  
  return ret;
}

/*
  SetMulticastTtl -- Sets the multicast TTL value for the socket.
*/
int SetMulticastTtl(int s, int af, int ttl)
{
  char	*optval = NULL;
  int	optlevel = 0,
        option = 0,
        optlen = 0,
        ret = 0,
        errno;
  
  if (af == AF_INET)
  {
    // Set the options for v4
    optlevel = IPPROTO_IP;
    option   = IP_MULTICAST_TTL;
    optval   = (char *) &ttl;
    optlen   = sizeof(ttl);
  }
  else if (af == AF_INET6)
  {
    // Set the options for v6
    optlevel = IPPROTO_IPV6;
    option   = IPV6_MULTICAST_HOPS;
    optval   = (char *) &ttl;
    optlen   = sizeof(ttl);
  }
  else
  {
    fprintf(stderr, "ERROR: Invalid address family for setting TTL.\n");
    ret = -1;
  }

  if (ret != -1)
  {
    // Set the TTL value
    ret = setsockopt(s, optlevel, option, optval, optlen);
    if (ret == -1)
    {
      fprintf(stderr, "ERROR #%d: setsockopt failed in SetMulticastTtl.", errno);
    }
    else
    {
      printf("Set multicast TTL to: %d\n", ttl);
    }
  }
  
  return ret;
}

/*
  SetMulticastLoopBack -- Enables or disables multicast loopback. Data 
  is placed in the receive queue, if enabled (and is a member of a 
  multicast group).
*/
int SetMulticastLoopBack(int s, int af, int loopval)
{
  char	*optval = NULL;
  int	optlevel = 0,
        option = 0,
        optlen = 0,
        ret = 0,
        errno;
  
  if (af == AF_INET)
  {
    // Set the v4 options
    optlevel = IPPROTO_IP;
    option   = IP_MULTICAST_LOOP;
    optval   = (char *) &loopval;
    optlen   = sizeof(loopval);
  }
  else if (af == AF_INET6)
  {
    // Set the v6 options
    optlevel = IPPROTO_IPV6;
    option   = IPV6_MULTICAST_LOOP;
    optval   = (char *) &loopval;
    optlen   = sizeof(loopval);
  }
  else
  {
    fprintf(stderr, "ERROR: Invalid address family for setting multicast loopback.\n");
    ret = -1;
  }
  
  if (ret != -1)
  {
    // Set the multicast loopback
    ret = setsockopt(s, optlevel, option, optval, optlen);
    if (ret == -1)
    {
      fprintf(stderr, "ERROR #%d: setsockopt failed in SetMulticastLoopBack.\n", errno);
    }
    else
    {
      printf("Setting multicast loopback to: %d\n", loopval);
    }
  }
  
  return ret;
}

void serverLoop(struct addrinfo* rm, unsigned short port, int hs, int cs) {
  fd_set socks;
  struct timeval timeout;
  
  char msg[] = "Hello, client!";

  int newSock = 0;
  struct announce announceMsg = { htonl(21608), htons(port), 0 };

  int ret;
  int ready = 0;

  listen(cs, 10);

  printf("begin server loop\n");

  while(1) {
    ret = sendto(hs, &announceMsg, sizeof(announceMsg), 0, rm->ai_addr, rm->ai_addrlen);
    if(ret < 0) {
      fprintf(stderr, "failed to send magicNumber packet\n");
      exit(1);
    }

    printf("SENT %d bytes to ", ret);
    PrintAddress(rm->ai_addr, rm->ai_addrlen);
    printf("\n");

    printf("[port=%d]:MagicNumber=%d\n", ntohs(announceMsg.port), ntohl(announceMsg.magic));

    // Setup the select loop
    FD_ZERO(&socks);
    FD_SET(cs, &socks);

    timeout.tv_sec = 1;
    timeout.tv_usec = 250000;

    ready = select(cs+1, &socks, NULL, NULL, &timeout);
    if(ready < 0) {
      fprintf(stderr, "select failed\n");
      exit(1);
    } else if(ready > 0) {
      newSock = accept(cs, NULL, NULL);
      close(hs);
      break;
    }
  }

  send(newSock, msg, strlen(msg), 0);
  close(newSock);
  close(cs);

  exit(0);
}

void clientLoop(int hs, int cs) {
  struct sockaddr serv;
  struct sockaddr_in* serv_in = (struct sockaddr_in*)&serv;
  int length;
  int connectRet;
  int magicNumber = 21608;
  struct announce announceMsg;
  char msg[100];

  int ret = 0;
 
  printf("begin client loop\n");

  while(1) {
    ret = recvfrom(hs, &announceMsg, sizeof(announceMsg), 0, &serv, &length);
    if(ret > 0) {
      announceMsg.magic = ntohl(announceMsg.magic);
      announceMsg.port = ntohs(announceMsg.port);

      printf("READ %d bytes from ", ret);
      PrintAddress(&serv, sizeof(serv));
      printf("\n");

      printf("[port=%d]:MagicNumber=%d\n", announceMsg.port, announceMsg.magic);

      if(magicNumber == announceMsg.magic) {
	serv_in->sin_port = htons(announceMsg.port);

	printf("CONNECT to ");
	PrintAddress(&serv, sizeof(serv));
	printf("\n");

	connectRet = connect(cs, &serv, length);
	if(connectRet < 0) {
	  fprintf(stderr, "connect failed\n");
	  exit(1);
	}
	close(hs);
	break;
      }
    } else if(ret < 0) {
      fprintf(stderr, "recvfrom failed\n");
      exit(1);
    }
  }

  recv(cs, msg, 100, 0);
  printf("%s\n", msg);
  close(cs);

  exit(0);
}

int main(int argc, char** argv) {
  int hs = INVALID_SOCKET;
  int cs = INVALID_SOCKET;
  int ret = 0;
  struct addrinfo* hs_resmulti = NULL;
  struct addrinfo* hs_resbind = NULL;
  struct addrinfo* hs_resif = NULL;
  struct addrinfo* cs_resbind = NULL;
  int reuseaddr = 1;

  hs_resmulti = ResolveAddress(MCAST_ADDR, MCAST_PORT, AF_UNSPEC, SOCK_DGRAM, IPPROTO_UDP);
  if(!hs_resmulti) {
    fprintf(stderr, "failed to resolve multicast address\n");
    exit(1);
  }

  hs_resbind = ResolveAddress(NULL, 
			   MCAST_PORT,
			   hs_resmulti->ai_family,
			   hs_resmulti->ai_socktype,
			   hs_resmulti->ai_protocol);
  if(!hs_resbind) {
    fprintf(stderr, "failed to resolve bind address\n");
    exit(1);
  }

  hs_resif = ResolveAddress(NULL,
			 "0",
			 hs_resmulti->ai_family,
			 hs_resmulti->ai_socktype,
			 hs_resmulti->ai_protocol);
  if(!hs_resif) {
    fprintf(stderr, "failed to resolve interface address\n");
    exit(1);
  }

  cs_resbind = ResolveAddress(NULL, (argc > 1) ? TCP_PORT : "0", AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);
  if(!cs_resbind) {
    fprintf(stderr, "failed to resolve tcp interface address\n");
    exit(1);
  }
  
  hs = socket(hs_resmulti->ai_family, hs_resmulti->ai_socktype, hs_resmulti->ai_protocol);
  if(hs == INVALID_SOCKET) {
    fprintf(stderr, "failed to initialize the socket\n");
    exit(1);
  }
  
  ret = setsockopt(hs, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
  if(ret != 0) {
    fprintf(stderr, "setting socket to reuse addr failed\n");
    exit(1);
  }
 
  ret = bind(hs, hs_resbind->ai_addr, hs_resbind->ai_addrlen);
  if(ret != 0) {
    fprintf(stderr, "failed to bind hs socket to addr\n");
    exit(1);
  }

  printf("Binding hs to ");
  PrintAddress(hs_resbind->ai_addr, hs_resbind->ai_addrlen);
  printf("\n");

  // Join the multicast group, if specified
  ret = JoinMulticastGroup(hs, hs_resmulti, hs_resif);
  if (ret == -1) {
    fprintf(stderr, "ERROR: Unable to join multicast group.\n");
    exit(1);
  }
  
  // Set the outgoing interface
  ret = SetSendInterface(hs, hs_resif);
  if (ret != 0) {
    fprintf(stderr, "unable to set multicast send interface.\n");
    exit(1);
  }
  
  // Set the TTL.
  ret = SetMulticastTtl(hs, hs_resmulti->ai_family, DEFAULT_TTL);
  if (ret != 0) {
    fprintf(stderr, "unable to set multicast TTL.\n");
    exit(1);
  }
  
  // Disable the loopback if selected
  ret = SetMulticastLoopBack(hs, hs_resmulti->ai_family, 1);
  if (ret == -1) {
    fprintf(stderr, "unable to set multicast loopback flag.\n");
    exit(1);
  }

  cs = socket(cs_resbind->ai_family, cs_resbind->ai_socktype, cs_resbind->ai_protocol);
  if(cs == INVALID_SOCKET) {
    fprintf(stderr, "failed to initialize the socket\n");
    exit(1);
  }
  
  ret = setsockopt(cs, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
  if(ret != 0) {
    fprintf(stderr, "setting socket to reuse addr failed\n");
    exit(1);
  }
 
  ret = bind(cs, cs_resbind->ai_addr, cs_resbind->ai_addrlen);
  if(ret != 0) {
    fprintf(stderr, "failed to bind cs socket to addr\n");
    exit(1);
  }

  printf("Binding cs to ");
  PrintAddress(cs_resbind->ai_addr, cs_resbind->ai_addrlen);
  printf("\n");


  // hack: any argument to function will make it do server
  if(argc > 1) {
    serverLoop(hs_resmulti, getPort(cs_resbind), hs, cs);
  } else {
    clientLoop(hs, cs);
  }

  exit(0);
}
