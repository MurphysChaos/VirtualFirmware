/*
	vf_tx -- VirtualFirmware ThreadX component
	
	Creates a connection between the sockets API and a ThreadX-based simulation 
	of the firmware.
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

#define FW_MSG_SIZE	32
#define FW_QUE_SIZE 32 /* number of messages, queue is FW_QUE_SIZE * FW_MSG_SIZE in bytes */
#define FW_DAT_SIZE	4096

/* Define the control blocks */
TX_THREAD	thread_msg_in;
TX_THREAD	thread_msg_out;





int main()
{
	tx_kernel_enter();
}

