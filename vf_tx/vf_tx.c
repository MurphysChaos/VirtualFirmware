/*
	vf_tx -- VirtualFirmware ThreadX component
	
	Creates a connection between the sockets API and a ThreadX-based simulation 
	of the firmware. Written to function with the tx.dll provided for a Win32
    environment.
*/

/*
	Copyright (C) 2011 Joel Murphy, All rights reserved

	The code in this code file is licensed under GPLv2. This license does not 
	extend to any other files bundlded with this file.
	
	ThreadX and ThreadX RTOS are trademarks of ExpressLogic and all ThreadX 
	headers and libraries are the property of ExpressLogic.
*/

#include "message.h"
#include "e1000.h"
#include "tx_api.h"

#pragma comment (lib, "tx.lib")
#define OPTRC_FILE "vf.optrc"

#define FW_MSG_SIZE         sizeof (struct e1000_aq_desc)   /* Size of a firmware message, in bytes. */
#define FW_DAT_SIZE         4096    /* The size of the data queue for receiving a data message. */
#define FW_THREADS          1       /* Number of threads needing stacks. */
#define FW_STACK_SIZE       1024    /* Stack size for a single thread. */ 
#define FW_BYTE_POOL_SIZE   (FW_MSG_SIZE + FW_THREADS * FW_STACK_SIZE + FW_DAT_SIZE)
#define AQ_DESC             struct e1000_aq_desc

/* Thread IDs */
#define TID_COMM    1
#define TID_FW      2

/* define the control blocks */
TX_THREAD       t_0;     /* Communication thread */
TX_BYTE_POOL    pl_byte_0;  /* Primary memory pool */

/* thread entry prototypes*/
void    t_0_entry(ULONG t_input);

/* Communications socket */
SOCKET  g_sock;

int main()
{
    int rc;
    WSADATA wsaData;
    
    /* Initialize Winsock */
    rc = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (rc != 0) {
        dbg(DBG_ERROR, "WSAStartup failed with error: %d\n", rc);
        goto err;
    }
    g_sock = announce(OPTRC_FILE);
    if (g_sock == INVALID_SOCKET) {
        rc = -1;
        goto err;
    }

	tx_kernel_enter();

    err:
	closesocket(g_sock);
	WSACleanup();
    return rc;
}

void tx_application_define(void *mem_base) {
    CHAR    *mem_ptr;
    
    /* Claim memory pool. */
    tx_byte_pool_create(&pl_byte_0, "byte pool 0", mem_base, FW_BYTE_POOL_SIZE);

    tx_byte_allocate(&pl_byte_0, (VOID **) &mem_ptr, FW_STACK_SIZE, TX_NO_WAIT);
    tx_thread_create(&t_0, "communication thread", t_0_entry, TID_COMM, mem_ptr, FW_STACK_SIZE + FW_DAT_SIZE, 16, 16, 4, TX_AUTO_START);
}

static void e1000_aq_get_version(struct e1000_aq_desc *desc)
{
#define FW_MAJ_TEMP 0x34
#define FW_MIN_TEMP 0x13
#define API_MAJ_TEMP 0xfa
#define API_MIN_TEMP 0xaf
    desc->param0 = (FW_MAJ_TEMP);
    desc->param0 |= ((FW_MIN_TEMP << 16));
    desc->param1 = (API_MAJ_TEMP);
    desc->param1 |= ((API_MIN_TEMP << 16));
}

static void e1000_aq_driver_heartbeat(struct e1000_aq_desc *desc) {
    /* Something should be done here. */
}

static void e1000_aq_echo(struct e1000_aq_desc *desc, CHAR *in_buff)
{
    /* Sample provided showed copying the data into an internal buffer. The received 
       data is already in an internal buffer. */

    /* Pretend we've done something interesting. */
}

void t_0_entry(ULONG t_input) {
    AQ_DESC desc;
    CHAR buffer[FW_DAT_SIZE];
    UINT msg_length;
    UINT dat_length;
    int rc;
    int echobuf = 0;

    dbg(DBG_ALL, "Entering thread t_0.\n");

    while (1) {
        msg_length = sizeof (desc);
        rc = recvdata(g_sock, &desc, (uint16_t *) &msg_length); 
        if (rc == SOCKET_ERROR) {
            dbg(DBG_ERROR, "RECVDATA (message): %s\n", sock_error());
        } else {
            dat_length = desc.datalen;

            if(dat_length) {
                rc = recvdata(g_sock, buffer, (uint16_t *) &dat_length);
                if (rc == SOCKET_ERROR) {
                    dbg(DBG_ERROR, "RECVDATA (buffer): %s\n", sock_error());
                }
            }

            switch(desc.opcode) {
            case e1000_aqc_get_version:
                dbg(DBG_ALL, "Message: e1000_aqc_get_version\n");
                e1000_aq_get_version(&desc);
                dbg(DBG_ALL, "  Set param0: %u\n      param1: %u\n", desc.param0, desc.param1);
                break;
            case e1000_aqc_driver_heartbeat:
                dbg(DBG_ALL, "Message: e1000_aqc_driver_heartbeat\n");
                e1000_aq_driver_heartbeat(&desc);
                break;
            case e1000_aqc_echo:
                dbg(DBG_ALL, "Message: e1000_aqc_echo\n");
                e1000_aq_echo(&desc, buffer);
                echobuf = 1;
                break;
            default:
                desc.retval = E1000_AQ_RC_ENOSYS;
                break;
            }

            desc.flags |= E1000_AQ_FLAG_DD;

            dbg(DBG_ALL, "Sending: desc\n");
            dbg(DBG_ALL, "\t.flags=0x%x (%d)\n", desc.flags, desc.flags);
            dbg(DBG_ALL, "\t.opcode=0x%x (%d)\n", desc.opcode, desc.opcode);
            dbg(DBG_ALL, "\t.datalen=0x%x (%d)\n", desc.datalen, desc.datalen);
            dbg(DBG_ALL, "\t.retval=0x%x (%d)\n", desc.retval, desc.retval);
            dbg(DBG_ALL, "\t.cookie_high=0x%x (%d)\n", desc.cookie_high, desc.cookie_high);
            dbg(DBG_ALL, "\t.cookie_low=0x%x (%d)\n", desc.cookie_low, desc.cookie_low);
            dbg(DBG_ALL, "\t.param0=0x%x (%d)\n", desc.param0, desc.param0);
            dbg(DBG_ALL, "\t.param1=0x%x (%d)\n", desc.param1, desc.param1);
            dbg(DBG_ALL, "\t.addr_high=0x%x (%d)\n", desc.addr_high, desc.addr_high);
            dbg(DBG_ALL, "\t.addr_low=0x%x (%d)\n", desc.addr_low, desc.addr_low);
            dbg(DBG_ALL, "msg_length=0x%x (%d)\n", msg_length, msg_length);
            rc = senddata(g_sock, &desc, msg_length);
            if (rc == SOCKET_ERROR) {
                dbg(DBG_ERROR, "SENDDATA (message): %s\n", sock_error());
                break;
            }
            if(echobuf && dat_length) {
                rc = senddata(g_sock, buffer, dat_length);
                if (rc == SOCKET_ERROR) {
                    dbg(DBG_ERROR, "SENDDATA (buffer): %s\n", sock_error());
                    break;
                }
                echobuf = 0;
            }
        }
    }
}
