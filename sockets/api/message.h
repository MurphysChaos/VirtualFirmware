#ifdef _WIN32
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
typedef int SOCKET;
#define FALSE 0
#define TRUE 1
#endif
#include <stdio.h>
#include "panel.h"
#include "sockerr.h"

/*
 * anounce()
 * 
 * the announce function will multicast a message
 * announcing this functions service via a unique
 * magic number.
 *
 * Returns a socket
 */
int announce(char* d_port, int magic);

/*
 * locate()
 * 
 * the locate function will listen for incoming announcement messages
 * and will connect to the first valid server
 * 
 * Returns a socket
 */
int locate(int magic);

/*
 * sendmsg_withlength()
 * 
 * this function will send a message of raw bytes of a certain length
 * across the socket, by first sending the length in a fixed value
 * "header", then sending the buffer.
 */
int sendmsg_withlength(SOCKET socket, char* buffer, uint16_t length);

/*
 * recvmsg_withlength()
 *
 * this function will receive a message from the socket
 * and put its value into the buffer. It will return an
 * error code if the given buffer is not large enough to hold
 * all of the data
 *
 * it will modify the length value to be the actual
 * length of the message returned.
 */
int recvmsg_withlength(SOCKET socket, char* buffer, uint16_t* length);
