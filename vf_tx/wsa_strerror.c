/* 
    wsa_strerror -- Adds functionality similar to strerror to Windows 
    Sockets, allowing for human-readable errors.
*/

/*
    Copyright (C) 2011 Joel Murphy, All rights reserved
 
    The code in this file is licensed under GPL v2. This license does not 
    extend to any file bundled with this one.
  
    The error strings included in this file are copyright Microsoft. No 
    intellectual property claim is stated or implied by the author of the 
    remainder of the file.
*/

#ifdef _WIN32

#include "wsa_strerror.h"

/* Returns the Microsoft-specified description of a particular winsock
 * error.
 * * * */
const char *wsa_strerror(int e) {
	switch (e) {
	case WSA_INVALID_HANDLE: // 6
		return "Specified event object handle is invalid.";
		break;
	case WSA_NOT_ENOUGH_MEMORY: // 8
		return "Insufficient memory available.";
		break;
	case WSA_INVALID_PARAMETER: // 87
		return "One or more parameters are invalid.";
		break;
	case WSA_OPERATION_ABORTED: // 995
		return "Overlapped operation aborted.";
		break;
	case WSA_IO_INCOMPLETE: // 996
		return "Overlapped I/O event object not in signaled state.";
		break;
	case WSA_IO_PENDING: // 997
		return "Overlapped operations will complete later.";
		break;
	case WSAEINTR: // 10004
		return "Interrupted function call.";
		break;
	case WSAEBADF: // 10009
		return "File handle is not valid.";
		break;
	case WSAEACCES: // 10013
		return "Permission denied.";
		break;
	case WSAEFAULT: // 10014
		return "Bad address.";
		break;
	case WSAEINVAL: // 10022
		return "Invalid argument.";
		break;
	case WSAEMFILE: // 10024
		return "Too many open files.";
		break;
	case WSAEWOULDBLOCK: // 10035
		return "Resource temporarily unavailable.";
		break;
	case WSAEINPROGRESS: // 10036
		return "Operation now in progress.";
		break;
	case WSAEALREADY: // 10037
		return "Operation already in progress.";
		break;
	case WSAENOTSOCK: // 10038
		return "Socket operation on nonsocket.";
		break;
	case WSAEDESTADDRREQ: // 10039
		return "Destination address required.";
		break;
	case WSAEMSGSIZE: // 10040
		return "Message too long.";
		break;
	case WSAEPROTOTYPE: // 10041
		return "Protocol wrong type for socket.";
		break;
	case WSAENOPROTOOPT: // 10042
		return "Bad protocol option.";
		break;
	case WSAEPROTONOSUPPORT: // 10043
		return "Protocol not supported.";
		break;
	case WSAESOCKTNOSUPPORT: // 10044
		return "Socket type not supported.";
		break;
	case WSAEOPNOTSUPP: // 10045
		return "Operation not supported.";
		break;
	case WSAEPFNOSUPPORT: // 10046
		return "Protocol family not supported.";
		break;
	case WSAEAFNOSUPPORT: // 10047
		return "Address family not supported by protocol family.";
		break;
	case WSAEADDRINUSE: // 10048
		return "Address already in use.";
		break;
	case WSAEADDRNOTAVAIL: // 10049
		return "Cannot assign requested address.";
		break;
	case WSAENETDOWN: // 10050
		return "Network is down.";
		break;
	case WSAENETUNREACH: // 10051
		return "Network is unreachable.";
		break;
	case WSAENETRESET: // 10052
		return "Network dropped connection on reset.";
		break;
	case WSAECONNABORTED: // 10053
		return "Software caused connection abort.";
		break;
	case WSAECONNRESET: // 10054
		return "Connection reset by peer.";
		break;
	case WSAENOBUFS: // 10055
		return "No buffer space available.";
		break;
	case WSAEISCONN: // 10056
		return "Socket is already connected.";
		break;
	case WSAENOTCONN: // 10057
		return "Socket is not connected.";
		break;
	case WSAESHUTDOWN: // 10058
		return "Cannot send after socket shutdown.";
		break;
	case WSAETOOMANYREFS: // 10059
		return "Too many references.";
		break;
	case WSAETIMEDOUT: // 10060
		return "Connection timed out.";
		break;
	case WSAECONNREFUSED: // 10061
		return "Connection refused.";
		break;
	case WSAELOOP: // 10062
		return "Cannot translate name.";
		break;
	case WSAENAMETOOLONG: // 10063
		return "Name too long.";
		break;
	case WSAEHOSTDOWN: // 10064
		return "Host is down.";
		break;
	case WSAEHOSTUNREACH: // 10065
		return "No route to host.";
		break;
	case WSAENOTEMPTY: // 10066
		return "Directory not empty.";
		break;
	case WSAEPROCLIM: // 10067
		return "Too many processes.";
		break;
	case WSAEUSERS: // 10068
		return "User quota exceeded.";
		break;
	case WSAEDQUOT: // 10069
		return "Disk quota exceeded.";
		break;
	case WSAESTALE: // 10070
		return "Stale file handle reference.";
		break;
	case WSAEREMOTE: // 10071
		return "Item is remote.";
		break;
	case WSASYSNOTREADY: // 10091
		return "Network subsystem is unavailable.";
		break;
	case WSAVERNOTSUPPORTED: // 10092
		return "Winsock.dll version out of range.";
		break;
	case WSANOTINITIALISED: // 10093
		return "Successful WSAStartup not yet performed.";
		break;
	case WSAEDISCON: // 10101
		return "Graceful shutdown in progress.";
		break;
	case WSAENOMORE: // 10102
		return "No more results.";
		break;
	case WSAECANCELLED: // 10103
		return "Call has been canceled.";
		break;
	case WSAEINVALIDPROCTABLE: // 10104
		return "Procedure call table is invalid.";
		break;
	case WSAEINVALIDPROVIDER: // 10105
		return "Service provider is invalid.";
		break;
	case WSAEPROVIDERFAILEDINIT: // 10106
		return "Service provider failed to initialize.";
		break;
	case WSASYSCALLFAILURE: // 10107
		return "System call failure.";
		break;
	case WSASERVICE_NOT_FOUND: // 10108
		return "Service not found.";
		break;
	case WSATYPE_NOT_FOUND: // 10109
		return "Class type not found.";
		break;
	case WSA_E_NO_MORE: // 10110
		return "No more results.";
		break;
	case WSA_E_CANCELLED: // 10111
		return "Call was canceled.";
		break;
	case WSAEREFUSED: // 10112
		return "Database query was refused.";
		break;
	case WSAHOST_NOT_FOUND: // 11001
		return "Host not found.";
		break;
	case WSATRY_AGAIN: // 11002
		return "Nonauthoritative host not found.";
		break;
	case WSANO_RECOVERY: // 11003
		return "This is a nonrecoverable error.";
		break;
	case WSANO_DATA: // 11004
		return "Valid name, no data record of requested type.";
		break;
	case WSA_QOS_RECEIVERS: // 11005
		return "QoS receivers.";
		break;
	case WSA_QOS_SENDERS: // 11006
		return "QoS senders.";
		break;
	case WSA_QOS_NO_SENDERS: // 11007
		return "No QoS senders.";
		break;
	case WSA_QOS_NO_RECEIVERS: // 11008
		return "QoS no receivers.";
		break;
	case WSA_QOS_REQUEST_CONFIRMED: // 11009
		return "QoS request confirmed.";
		break;
	case WSA_QOS_ADMISSION_FAILURE: // 11010
		return "QoS admission error.";
		break;
	case WSA_QOS_POLICY_FAILURE: // 11011
		return "QoS policy failure.";
		break;
	case WSA_QOS_BAD_STYLE: // 11012
		return "QoS bad style.";
		break;
	case WSA_QOS_BAD_OBJECT: // 11013
		return "QoS bad object.";
		break;
	case WSA_QOS_TRAFFIC_CTRL_ERROR: // 11014
		return "QoS traffic control error.";
		break;
	case WSA_QOS_GENERIC_ERROR: // 11015
		return "QoS generic error.";
		break;
	case WSA_QOS_ESERVICETYPE: // 11016
		return "QoS service type error.";
		break;
	case WSA_QOS_EFLOWSPEC: // 11017
		return "QoS flowspec error.";
		break;
	case WSA_QOS_EPROVSPECBUF: // 11018
		return "Invalid QoS provider buffer.";
		break;
	case WSA_QOS_EFILTERSTYLE: // 11019
		return "Invalid QoS filter style.";
		break;
	case WSA_QOS_EFILTERTYPE: // 11020
		return "Invalid QoS filter type.";
		break;
	case WSA_QOS_EFILTERCOUNT: // 11021
		return "Incorrect QoS filter count.";
		break;
	case WSA_QOS_EOBJLENGTH: // 11022
		return "Invalid QoS object length.";
		break;
	case WSA_QOS_EFLOWCOUNT: // 11023
		return "Incorrect QoS flow count.";
		break;
	case WSA_QOS_EUNKOWNPSOBJ: // 11024
		return "Unrecognized QoS object.";
		break;
	case WSA_QOS_EPOLICYOBJ: // 11025
		return "Invalid QoS policy object.";
		break;
	case WSA_QOS_EFLOWDESC: // 11026
		return "Invalid QoS flow descriptor.";
		break;
	case WSA_QOS_EPSFLOWSPEC: // 11027
		return "Invalid QoS provider-specific flowspec.";
		break;
	case WSA_QOS_EPSFILTERSPEC: // 11028
		return "Invalid QoS provider-specific filterspec.";
		break;
	case WSA_QOS_ESDMODEOBJ: // 11029
		return "Invalid QoS shape discard mode object.";
		break;
	case WSA_QOS_ESHAPERATEOBJ: // 11030
		return "Invalid QoS shaping rate object.";
		break;
	case WSA_QOS_RESERVED_PETYPE: // 11031
		return "Reserved policy QoS element type.";
		break;
	}
	return NULL;
}

#endif
