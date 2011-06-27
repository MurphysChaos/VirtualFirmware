/*
 * panel.h
 *
 * A panel is a structure containing information about a socket
 * The intent is to make a more useable interface for accessing
 * the socket, and displaying its current information.
 */

#ifndef _PANEL_H_
#define _PANEL_H_

// Socket headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

// Socket Error library
#include "sock_error.h"

// Other required headers
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/* Defined for compatability with Win32 */
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (~0)
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (~0)
#endif

/*
 * This structure is used to store the various
 * addresses that a panel will be using.
 */
typedef struct address {
  char host[NI_MAXHOST];
  char service[NI_MAXSERV];
} ADDRESS;

/*
 * This structure defines an address
 * that a panel is connected to.
 * 
 * NOTE:
 * socket represents the socket to send
 * on, in orer to reach the address.
 */
typedef struct destination {
  ADDRESS addr;
  struct addrinfo* r_addr;
  uint32_t socket;
  int isConnected;
  
  struct destination* next;
} DESTINATION;

typedef struct mgroup {
  char host[NI_MAXHOST];

  struct mgroup* next;
} MGROUP;

typedef struct panel {
  uint32_t socket;

  ADDRESS source;
  DESTINATION* destination;

  MGROUP* mgroups;

  uint32_t family;
  uint32_t type;
  uint32_t protocol;

  /* These are used for internal operations */
  struct addrinfo* r_source;
} PANEL;

/*
 * PanelCreate()
 *
 * This function will return a new blank panel ready to be filled in.
 */
PANEL* PanelCreate();

/*
 * PanelInit()
 * 
 * This function is used to initialize a panel to a beginning state.
 * It will set the source address, and the family, type and protocol.
 * 
 * It will create and bind the socket with the specified details
 */
int PanelInit(PANEL* p, char* addr, char* port, int family, int type, int protocol);

/*
 * PanelSetDestination()
 *
 * This function will initialize a single DESTINATION
 * structure with the proper details, and should be used
 * in a connectionless system to force set the destination.
 */
int PanelSetDestination(PANEL* p, char* addr, char* port);

/*
 * PanelJoinMulticast()
 * 
 * This function will join the panel to the multicast addr
 * that has been specified. It should be used in order
 * to receive messages from that multicast address.
 */
int PanelJoinMulticast(PANEL* p, char* addr);

/*
 * PanelConnectDestination()
 *
 * This function is used to connect to a remote server
 * in a connection based system. It will fill in
 * a single DESTINATION structure with the proper details.
 */
int PanelConnectDestination(PANEL* p, char* addr, char* port);

/*
 * PanelAcceptConnection()
 *
 * This function is used to accept a connection from a client
 * in a connection based system. It will fill in the next
 * slot of the DESTINATION linked list with the details
 * of this connection. The socket given here is the socket
 * returned by the accept() call.
 */
int PanelAcceptConnection(PANEL* p);

/*
 * PanelSendData()
 *
 * This function will send data in the data buffer
 * across the network. It assumes the data is raw
 * and does not provide its own header information.
 *
 * NOTE:
 * It uses the first destination in the linked list.
 * The user must handle other possible operations.
 */
int PanelSendData(PANEL* p, void* data, int length);

/*
 * PanelReceiveData()
 *
 * This function will receive data from the network
 * and put it into the buffer given. The maxlength
 * should initially store the maximum size of the buffer
 * 
 * Upon return, maxlength will have the total data
 * that has been put into the buffer
 *
 * NOTE:
 * It uses the first destination in the linked list,
 * and the user must handle other more complex operations. 
 */
int PanelReceiveData(PANEL* p, void* buffer, int maxLength, struct sockaddr* fromaddr, socklen_t* fromlen);

/*
 * PanelDelete()
 * 
 * This function simply frees all memory in use by the panel
 * and closes all connections, then deletes the panel itself.
 */
int PanelDelete();

#endif
