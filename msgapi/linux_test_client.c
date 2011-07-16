#include "message.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	char message[4096];
	uint16_t msg_length = 4096;
	int rc = 0;
	SOCKET s = INVALID_SOCKET;

	s = announce("9752", 0xe1000);
	
	while(1) {
		rc = recvmsg_withlength(s, message, &msg_length);
		if (rc < 0) {
			printf("recv failed\n");
			exit(-1);
		}

		printf("received msg of length: %d\n", msg_length);

		rc = sendmsg_withlength(s, message, msg_length);
		if (rc < 0) {
			printf("send failed\n");
			exit(-1);
		}
	}
}
