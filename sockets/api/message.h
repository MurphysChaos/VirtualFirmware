#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include "panel.h"
#include "sockerr.h"

/*
 * anounce()
 * the announce function will multicast a message
 * announcing this functions service via a unique
 * magic number.
 *
 * Returns a socket
 */
int announce(char* d_port, int magic);

/*
 * locate()
 * the locate function will listen for incoming announcement messages
 * and will connect to the first valid server
 * 
 * Returns a socket
 */
int locate(int magic);
