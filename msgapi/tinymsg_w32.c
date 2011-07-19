/* Win32 Client side */
/* Since this has a main() to, make sure to change in tinymsg.c in order for this to work */

#include "message.h"


int bIsServer = 0;

#define PORT "9752"
#define MAGIC 0x6A38937
#define MAX_MSGSIZE 1024

/* E1000 TYPES ==================================================== */
#define E1000_AQ_FLAG_DD_OFF_SHIFT	0
#define E1000_AQ_FLAG_DD		(1 << E1000_AQ_FLAG_DD_OFF_SHIFT)

enum e1000_admin_queue_err {
	E1000_AQ_RC_OK		= 0,
	E1000_AQ_RC_ENOSYS	= 1,
};

enum e1000_admin_queue_cmd {
	e1000_aqc_get_version		= 0x0001,
	e1000_aqc_driver_heartbeat	= 0x0002,
	e1000_aqc_echo			= 0x0003,
};

struct e1000_aq_desc {
        uint16_t flags;
        uint16_t opcode;
        uint16_t datalen;
        uint16_t retval;
        uint32_t cookie_high;
        uint32_t cookie_low;
        uint32_t param0;
        uint32_t param1;
        uint32_t addr_high;
        uint32_t addr_low;
};
static void e1000_aq_get_version(struct e1000_aq_desc *desc);
static void e1000_aq_echo(struct e1000_aq_desc *desc);
/* E1000 TYPES ==================================================== */

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
	struct e1000_aq_desc eMsg;
	struct e1000_aq_desc eBuf;


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
    //socket = announce(PORT, MAGIC);
	socket = announce(NULL);
    if (socket == INVALID_SOCKET) {
      goto err;
    }

	rc = recvmsg_withlength(socket, &eBuf, &msg_len);
    if (rc == SOCKET_ERROR) {
      goto err;
    }
	else
	{
		//Now process the message
		switch (eBuf.opcode) {
		case e1000_aqc_get_version:
			e1000_aq_get_version(&eBuf);
			break;
		case e1000_aqc_echo:
			//e1000_aq_echo(&desc);
			break;
		default:
			eBuf.retval = E1000_AQ_RC_ENOSYS;
			break;
		}

		// writeback descriptor
		eBuf.flags |= E1000_AQ_FLAG_DD;		

		//try to send it back
		if(sendmsg_withlength(socket, &eBuf, msg_len) == SOCKET_ERROR)
			goto err;
	}

    
  } else {
	//initialize the message
	eMsg.flags = 1;
    eMsg.opcode = 2;
    eMsg.datalen = 3;
    eMsg.retval = 4;
    eMsg.cookie_high = 5;
    eMsg.cookie_low = 6;
    eMsg.param0 = 7;
    eMsg.param1 = 8;
    eMsg.addr_high = 9;
    eMsg.addr_low = 10;

	socket = locate(NULL);
    if (socket == INVALID_SOCKET) {
      goto err;
    }
    
	
	rc = sendmsg_withlength(socket, &eMsg, sizeof(struct e1000_aq_desc));
	printf("Sent %d bytes\n", sizeof(struct e1000_aq_desc));
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

static void e1000_aq_get_version(struct e1000_aq_desc *desc)
{
	//fprintf(e1000_log, "in aq_get_version!\n");
#define FW_MAJ_TEMP 0x34
#define FW_MIN_TEMP 0x13
#define API_MAJ_TEMP 0xfa
#define API_MIN_TEMP 0xaf
	desc->param0 = (FW_MAJ_TEMP);
	desc->param0 |= ((FW_MIN_TEMP << 16));
	desc->param1 = (API_MAJ_TEMP);
	desc->param1 |= ((API_MIN_TEMP << 16));
}

static void e1000_aq_echo(struct e1000_aq_desc *desc)
{
/* This part has too many definitions, it will work when we merge this with the real QEMU */
//	target_phys_addr_t in_buf;
//	uint8_t *my_buf;
//
//	//fprintf(e1000_log, "%s\n", __func__);
//
//	if (desc->datalen != 0) {
//		/* create my local buffer */
//		my_buf = qemu_malloc(desc->datalen);
//		
//		/* copyin the indirect buffer */
//		in_buf = ((uint64_t)(desc->addr_high) << 32) | desc->addr_low;
//		my_buf = qemu_malloc(desc->datalen);
//		if (!my_buf) {
//			desc->flags |= E1000_AQ_FLAG_ERR;
//			desc->retval = ENOSPC;
//			goto echo_done;
//		}
//
//		/* copy into a local buffer */
//		cpu_physical_memory_read(in_buf, (void *)my_buf, desc->datalen);
//
//		/* pretend we've done something interesting */
//
//		/* copy back to the user's buffer */
//		cpu_physical_memory_write(in_buf, (void *)my_buf, desc->datalen);
//
//		/* goodbye local buffer */
//		qemu_free(my_buf);
//	}
//echo_done:
	return;
}