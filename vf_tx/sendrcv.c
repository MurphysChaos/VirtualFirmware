#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

typedef unsigned short uint16_t;

#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

//typedef int SOCKET;
#define FALSE 0
#define TRUE 1
#endif

#include "message.h"

int bIsServer = 0;

#define OPTRC_FILE "tinymsg.optrc"

#define PORT "9752"
#define MAGIC 0x6A38937
#define MAX_MSGSIZE 1024

void ProcessArgs(int argc, char **argv)
{
	int i;
	for (i=1;i<argc;i++)
	{
		if (argv[i][0] == '-') {
			switch(tolower(argv[i][1]))
			{
			case 's':
				bIsServer = (~0);
				break;
			}
		}
	}
}


int main(int argc, char** argv) {
  int socket;
  
  int rc;
  uint16_t msg_size = MAX_MSGSIZE;

  char buffer[MAX_MSGSIZE + 1];
  char msg[MAX_MSGSIZE + 1];
  
  // This is added for testing
  int buffSize = 0;
  int flags = 0;
  char *filename = argv[2];
  FILE *file = fopen(filename, "r");
  //End
  
  ProcessArgs(argc, argv);
  memset(buffer, 0, MAX_MSGSIZE + 1);
  memset(msg, 0, MAX_MSGSIZE + 1);
  
  

  
  if (bIsServer) {
  
    socket = announce(OPTRC_FILE);
    if (socket == INVALID_SOCKET) {
      goto err;
    }

	rc = recvdata(socket, buffer, &msg_size);
    if (rc == SOCKET_ERROR) {
      goto err;
    }
	printf("**** RECEIVED ****\n");
	printf("Content: %s \n", buffer);
	printf("Size = %d \n", msg_size);
  } else {
  
  // This is mainly added for testing
	// first check if the file is specified, exit if there is no file specified
	if(argc < 3){
		fprintf(stderr, "Please Specify the file to read the data from \n");
		printf("size of command line = %d \n", argc);
		exit(1);
	}
	
	
	
	if( file != NULL){
		while(fgets(msg, MAX_MSGSIZE, file)){
			// Set the buffer size
			if(argc == 3){
				// the size of the file is not given, we have to calculate the size ourself
				buffSize = strlen(msg);
				// gonna send this msg
            	// This if just for confirmation test
			}
			else{
				// buffer size is given as part of the command line argument
				//buffSize = (int) argv[2]; // Check this again to make sure I'm getting the right argument
				char *fileSize = argv[3];
				char s[5];
				FILE *fileS = fopen(fileSize, "r");
				if(fileS != NULL){
					while(fgets(s, 5, fileS)){
						buffSize = atoi(s);
					}
				}
			}
			
		}
			
	}
	else{
		fprintf(stderr, "Input file is NULL");
		return 3;
	}
	
	
	// End for testing
    socket = locate(OPTRC_FILE);
    if (socket == INVALID_SOCKET) {
      goto err;
    }
			
	rc = senddata(socket, msg, buffSize);
    if (rc == SOCKET_ERROR) {
      goto err;
    }
    printf("**** SENT ****\n");
    printf("Content: %s \n", msg);
	printf("Size = %d \n", buffSize);
  }
 
  close(socket);
  return 0;

 err:
  close(socket);
  return -1;
}
