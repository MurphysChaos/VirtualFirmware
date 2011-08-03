/*
    Copyright (c) 2011 Kabiru Ahmed

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

#include "message.h"

int bIsServer = 0;

#define OPTRC_FILE "demo.optrc"
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

  if (bIsServer) {
    socket = announce(OPTRC_FILE);
    if (socket == INVALID_SOCKET) {
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
      goto err;
    }
    printf("Enter a message: ");
    fgets(msg,MAX_MSGSIZE, stdin);
    rc = senddata(socket, msg, strlen(msg));
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
