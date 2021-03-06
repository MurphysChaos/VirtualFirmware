#include "message.h"

int bIsServer = 0;

#define OPTRC_FILE "tinymsg.optrc"
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

#ifdef _WIN32
    WSADATA wsaData;
    rc = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (rc != 0) {
        printf("WSAStartup failed with error: %d\n", rc);
        goto err;
    }	
#endif

    ProcessArgs(argc, argv);
    memset(buffer, 0, MAX_MSGSIZE + 1);
    memset(msg, 0, MAX_MSGSIZE + 1);

    strcpy(msg, "Hello, this is the client!");

    if (bIsServer) {
        socket = announce(OPTRC_FILE);
        if (socket == INVALID_SOCKET) {
            rc = -1;
            goto err;
        }

        rc = recvdata(socket, buffer, &msg_size);
        if (rc == SOCKET_ERROR) {
            goto err;
        }

        printf("%s\n", buffer);
    } else {
        socket = locate(OPTRC_FILE);
        if (socket == INVALID_SOCKET) {
            rc = -1;
            goto err;
        }

        rc = senddata(socket, msg, strlen(msg));
        if (rc == SOCKET_ERROR) {
            goto err;
        }
    }

err:
#ifdef _WIN32
	closesocket(socket);
	WSACleanup();
#else
    close(socket);
#endif
    return rc;
}
