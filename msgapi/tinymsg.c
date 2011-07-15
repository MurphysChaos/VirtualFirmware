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
  uint16_t msg_size = MAX_MSGSIZE;

  char buffer[MAX_MSGSIZE + 1];
  char msg[MAX_MSGSIZE + 1];
  
  ProcessArgs(argc, argv);
  memset(buffer, 0, MAX_MSGSIZE + 1);
  memset(msg, 0, MAX_MSGSIZE + 1);

  strcpy(msg, "Hello, this is the client!");

  if (bIsServer) {
    socket = announce(PORT, MAGIC);
    if (socket == INVALID_SOCKET) {
      goto err;
    }

    rc = recvmsg_withlength(socket, buffer, &msg_size);
    if (rc == SOCKET_ERROR) {
      goto err;
    }

    printf("%s\n", buffer);
  } else {
    socket = locate(MAGIC);
    if (socket == INVALID_SOCKET) {
      goto err;
    }
    
    rc = sendmsg_withlength(socket, msg, strlen(msg));
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
