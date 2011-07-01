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
  int socket;
  
  int rc;
  
  char buffer[MAX_MSGSIZE + 1];
  char msg[MAX_MSGSIZE + 1] = "Hello, this is the client!";
  
  ProcessArgs(argc, argv);
  memset(buffer, 0, sizeof(MAX_MSGSIZE) + 1);

  if (bIsServer) {
    socket = announce(PORT, MAGIC);
    if (socket == INVALID_SOCKET) {
      goto err;
    }

    rc = recv(socket, buffer, MAX_MSGSIZE, 0);
    if (rc == SOCKET_ERROR) {
      goto err;
    }

    printf("%s\n", buffer);
  } else {
    socket = locate(MAGIC);
    if (socket == INVALID_SOCKET) {
      goto err;
    }
    
    rc = send(socket, msg, strlen(msg), 0);
    if (rc == SOCKET_ERROR) {
      goto err;
    }
  }
 
  close(socket);
  return 0;

 err:
  close(socket);
  return -1;
}
