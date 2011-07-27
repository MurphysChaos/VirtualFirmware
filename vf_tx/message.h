/*
    message -- Messaging API
    
    Implements a method of connecting two interfaces using a brute force 
    multicast announce on all valid sockets and then sending messages of 
    variable length between the two interfaces.

    The brute force connection method means the two machines don't have to 
    know each others IP addresses ahead of time. The limitation is they must 
    be within the same subnet.
*/

/*
    Copyright (C) 2011 Jacob Keller, All rights reserved

    The code in this file is licensed under GPL v2. This license does not 
    extend to any file bundled with this one.
*/

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "stdint.h"
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <net/if.h>
typedef int SOCKET;
#define FALSE 0
#define TRUE 1
#endif
#include <stdio.h>
#include <stdarg.h>
#include "panel.h"
#include "sockerr.h"
#include "opt.h"
#include "dbg.h"

/*
 * anounce()
 * 
 * the announce function will multicast a message
 * announcing this functions service via a unique
 * magic number.
 *
 * Returns a socket
 */
SOCKET announce(const char *optrc);

/*
 * locate()
 * 
 * the locate function will listen for incoming announcement messages
 * and will connect to the first valid server
 * 
 * Returns a socket
 */
SOCKET locate(const char *optrc);

/*
 * senddata()
 * 
 * this function will send a message of raw bytes of a certain length
 * across the socket, by first sending the length in a fixed value
 * "header", then sending the buffer.
 */
int senddata(SOCKET socket, void *data, uint16_t length);

/*
 * recvdata()
 *
 * this function will receive a message from the socket
 * and put its value into the buffer. It will return an
 * error code if the given buffer is not large enough to hold
 * all of the data
 *
 * it will modify the length value to be the actual
 * length of the message returned.
 */
int recvdata(SOCKET socket, void *data, uint16_t * length);
