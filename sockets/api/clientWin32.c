/* Win32 Client side */
/* Since this has a main() to, make sure to change in tinymsg.c in order for this to work */

#include "message.h"


int bIsServer = 0;

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
	SOCKET socket;

	int rc;
	uint16_t msg_len; //store message length for recvmsg_withlength()

	char buffer[MAX_MSGSIZE + 1];
	char msg[MAX_MSGSIZE + 1] = "Hello, this is the client!";

	//Win32 socket initialization==========
	WSADATA wsaData;
    rc = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (rc != 0) {
        printf("WSAStartup failed with error: %d\n", rc);
        goto err;
    }	
	//=====================================

	ProcessArgs(argc, argv);
	memset(buffer, 0, sizeof(MAX_MSGSIZE) + 1);



  if (bIsServer) {
    socket = announce(PORT, MAGIC);
    if (socket == INVALID_SOCKET) {
      goto err;
    }

    //rc = recv(socket, buffer, MAX_MSGSIZE, 0);
	rc = recvmsg_withlength(socket, buffer, &msg_len);
    if (rc == SOCKET_ERROR) {
      goto err;
    }
	else
	{
		printf("Got the message\n");
		printf("Received %d bytes\n", (int)msg_len);
		printf("%s\n", buffer);
		printf("Now trying to send it back...\n");	
		if(sendmsg_withlength(socket, buffer, msg_len) == SOCKET_ERROR)
			goto err;
	}

    
  } else {
    socket = locate(MAGIC);
    if (socket == INVALID_SOCKET) {
      goto err;
    }
    
	
    //rc = send(socket, msg, strlen(msg), 0);
	rc = sendmsg_withlength(socket, msg, strlen(msg));
	printf("Sent %d bytes\n", strlen(msg));
    if (rc == SOCKET_ERROR) {
      goto err;
    }
  }



	closesocket(socket);
	WSACleanup();
	return 0;

	err:
	closesocket(socket);
	WSACleanup();
	return -1;
}
