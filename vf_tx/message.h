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
    Copyright (c) 2011 Jacob Keller

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in 
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
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

/* anounce()
 * 
 * the announce function will multicast a message
 * announcing this functions service via a unique
 * magic number.
 *
 * Returns a socket
 */
SOCKET announce(const char *optrc);

/* locate()
 * 
 * the locate function will listen for incoming announcement messages
 * and will connect to the first valid server
 * 
 * Returns a socket
 */
SOCKET locate(const char *optrc);

/* senddata()
 * 
 * this function will send a message of raw bytes of a certain length
 * across the socket, by first sending the length in a fixed value
 * "header", then sending the buffer.
 */
int senddata(SOCKET socket, void *data, uint16_t length);

/* recvdata()
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
