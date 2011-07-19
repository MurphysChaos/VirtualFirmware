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
#include "tx_api.h"

#define FW_MSG_SIZE	32      /* Size of a firmware message, in bytes. */
#define FW_QUEUE_SIZE 32    /* number of messages, queue is FW_QUE_SIZE * FW_MSG_SIZE in bytes. */
#define FW_DAT_SIZE	4096    /* The size of the data queue for receiving a data message. */
#define FW_STACK_SIZE 1024  /* Stack size for a single thread. */ 
#define FW_BYTE_POOL_SIZE 32768

/* define the control blocks */
TX_THREAD       t_connect;  /* Listen or announce */
TX_THREAD	    t_msg_in;   /* Receive messages */
TX_THREAD	    t_msg_out;  /* Send messages */
TX_QUEUE        q_msg_in;   /* Receive queue */
TX_QUEUE        q_msg_out;  /* Send queue */
TX_QUEUE        q_proc;     /* Inter-process communication */
TX_BYTE_POOL    pl_byte_0;  /* Primary memory pool */

/* thread entry prototypes*/
void    t_connect_entry(ULONG t_input);
void    t_msg_in_entry(ULONG t_input);
void    t_msg_out_entry(ULONG t_input);

int main()
{
	tx_kernel_enter();
}

void tx_application_define(void *mem_base) {
    CHAR    *mem_ptr;
    CHAR    *data;
    
    /* Claim memory pool. */
    tx_byte_pool_create(&pl_byte_0, "byte pool 0", mem_base, FW_BYTE_POOL_SIZE);

    /* Interprocess communication queue */
    tx_byte_allocate(&pl_byte_0, (VOID **) &mem_ptr, FW_QUEUE_SIZE * sizeof(ULONG), TX_NO_WAIT);
    tx_queue_create(&q_proc, "interprocess queue", sizeof(ULONG), mem_ptr, FW_QUEUE_SIZE * sizeof(ULONG));

    /* Connect thread */
    tx_byte_allocate(&pl_byte_0, (VOID **) &mem_ptr, FW_STACK_SIZE, TX_NO_WAIT);
    tx_thread_create(&t_connect, "connect thread", t_connect_entry, 0, mem_ptr, FW_STACK_SIZE, 16, 16, 4, TX_AUTO_START);

    /* Receive thread and queue */
    tx_byte_allocate(&pl_byte_0, (VOID **) &mem_ptr, FW_STACK_SIZE, TX_NO_WAIT);
    tx_thread_create(&t_msg_in, "receive thread", t_msg_in_entry, 0, mem_ptr, FW_STACK_SIZE, 16, 16, 4, TX_AUTO_START);
    tx_byte_allocate(&pl_byte_0, (VOID **) &mem_ptr, FW_QUEUE_SIZE * FW_MSG_SIZE, TX_NO_WAIT);
    tx_queue_create(&q_msg_in, "receive queue", FW_MSG_SIZE / sizeof(ULONG), mem_ptr, FW_QUEUE_SIZE * FW_MSG_SIZE);

    /* Send thread and queue */
    tx_byte_allocate(&pl_byte_0, (VOID **) &mem_ptr, FW_STACK_SIZE, TX_NO_WAIT);
    tx_thread_create(&t_msg_in, "send thread", t_msg_out_entry, 0, mem_ptr, FW_STACK_SIZE, 16, 16, 4, TX_AUTO_START);
    tx_byte_allocate(&pl_byte_0, (VOID **) &mem_ptr, FW_QUEUE_SIZE * FW_MSG_SIZE, TX_NO_WAIT);
    tx_queue_create(&q_msg_out, "send queue", FW_MSG_SIZE / sizeof(ULONG), mem_ptr, FW_QUEUE_SIZE * FW_MSG_SIZE);
}
