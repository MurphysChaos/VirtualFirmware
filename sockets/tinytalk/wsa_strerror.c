/* wsa_strerror -- Adds functionality similar to strerror to Windows 
 *   Sockets, allowing for human-readable errors.
 *
 * Copyright (C) 2011 Joel Murphy, All rights reserved
 *
 * The code in this file is licensed under GPL v2. This license does not 
 * extend to any file bundled with this one.
 * 
 * The error strings included in this file are copyright Microsoft. No 
 * intellectual property claim is stated or implied by the author of the 
 * remainder of the file.
 */

#ifdef _WIN32

#include "wsa_strerror.h"

#if 0
int errnotowsa(int e) {
	switch (e) {
	case 0: // 6: "Specified event object handle is invalid."
		return WSA_INVALID_HANDLE;
		break;
	case 0: // 8: "Insufficient memory available."
		return WSA_NOT_ENOUGH_MEMORY;
		break;
	case 0: // 87: "One or more parameters are invalid."
		return WSA_INVALID_PARAMETER;
		break;
	case 0: // 995: "Overlapped operation aborted."
		return WSA_OPERATION_ABORTED;
		break;
	case 0: // 996: "Overlapped I/O event object not in signaled state."
		return WSA_IO_INCOMPLETE;
		break;
	case 0: // 997: "Overlapped operations will complete later."
		return WSA_IO_PENDING;
		break;
	case 0: // 10004: "Interrupted function call."
		return WSAEINTR;
		break;
	case 0: // 10009: "File handle is not valid."
		return WSAEBADF;
		break;
	case 0: // 10013: "Permission denied."
		return WSAEACCES;
		break;
	case 0: // 10014: "Bad address."
		return WSAEFAULT;
		break;
	case 0: // 10022: "Invalid argument."
		return WSAEINVAL;
		break;
	case 0: // 10024: "Too many open files."
		return WSAEMFILE;
		break;
	case 0: // 10035: "Resource temporarily unavailable."
		return WSAEWOULDBLOCK;
		break;
	case 0: // 10036: "Operation now in progress."
		return WSAEINPROGRESS;
		break;
	case 0: // 10037: "Operation already in progress."
		return WSAEALREADY;
		break;
	case 0: // 10038: "Socket operation on nonsocket."
		return WSAENOTSOCK;
		break;
	case 0: // 10039: "Destination address required."
		return WSAEDESTADDRREQ;
		break;
	case 0: // 10040: "Message too long."
		return WSAEMSGSIZE;
		break;
	case 0: // 10041: "Protocol wrong type for socket."
		return WSAEPROTOTYPE;
		break;
	case 0: // 10042: "Bad protocol option."
		return WSAENOPROTOOPT;
		break;
	case 0: // 10043: "Protocol not supported."
		return WSAEPROTONOSUPPORT;
		break;
	case 0: // 10044: "Socket type not supported."
		return WSAESOCKTNOSUPPORT;
		break;
	case 0: // 10045: "Operation not supported."
		return WSAEOPNOTSUPP;
		break;
	case 0: // 10046: "Protocol family not supported."
		return WSAEPFNOSUPPORT;
		break;
	case 0: // 10047: "Address family not supported by protocol family."
		return WSAEAFNOSUPPORT;
		break;
	case 0: // 10048: "Address already in use."
		return WSAEADDRINUSE;
		break;
	case 0: // 10049: "Cannot assign requested address."
		return WSAEADDRNOTAVAIL;
		break;
	case 0: // 10050: "Network is down."
		return WSAENETDOWN;
		break;
	case 0: // 10051: "Network is unreachable."
		return WSAENETUNREACH;
		break;
	case 0: // 10052: "Network dropped connection on reset."
		return WSAENETRESET;
		break;
	case 0: // 10053: "Software caused connection abort."
		return WSAECONNABORTED;
		break;
	case 0: // 10054: "Connection reset by peer."
		return WSAECONNRESET;
		break;
	case 0: // 10055: "No buffer space available."
		return WSAENOBUFS;
		break;
	case 0: // 10056: "Socket is already connected."
		return WSAEISCONN;
		break;
	case 0: // 10057: "Socket is not connected."
		return WSAENOTCONN;
		break;
	case 0: // 10058: "Cannot send after socket shutdown."
		return WSAESHUTDOWN;
		break;
	case 0: // 10059: "Too many references."
		return WSAETOOMANYREFS;
		break;
	case 0: // 10060: "Connection timed out."
		return WSAETIMEDOUT;
		break;
	case 0: // 10061: "Connection refused."
		return WSAECONNREFUSED;
		break;
	case 0: // 10062: "Cannot translate name."
		return WSAELOOP;
		break;
	case 0: // 10063: "Name too long."
		return WSAENAMETOOLONG;
		break;
	case 0: // 10064: "Host is down."
		return WSAEHOSTDOWN;
		break;
	case 0: // 10065: "No route to host."
		return WSAEHOSTUNREACH;
		break;
	case 0: // 10066: "Directory not empty."
		return WSAENOTEMPTY;
		break;
	case 0: // 10067: "Too many processes."
		return WSAEPROCLIM;
		break;
	case 0: // 10068: "User quota exceeded."
		return WSAEUSERS;
		break;
	case 0: // 10069: "Disk quota exceeded."
		return WSAEDQUOT;
		break;
	case 0: // 10070: "Stale file handle reference."
		return WSAESTALE;
		break;
	case 0: // 10071: "Item is remote."
		return WSAEREMOTE;
		break;
	case 0: // 10091: "Network subsystem is unavailable."
		return WSASYSNOTREADY;
		break;
	case 0: // 10092: "Winsock.dll version out of range."
		return WSAVERNOTSUPPORTED;
		break;
	case 0: // 10093: "Successful WSAStartup not yet performed."
		return WSANOTINITIALISED;
		break;
	case 0: // 10101: "Graceful shutdown in progress."
		return WSAEDISCON;
		break;
	case 0: // 10102: "No more results."
		return WSAENOMORE;
		break;
	case 0: // 10103: "Call has been canceled."
		return WSAECANCELLED;
		break;
	case 0: // 10104: "Procedure call table is invalid."
		return WSAEINVALIDPROCTABLE;
		break;
	case 0: // 10105: "Service provider is invalid."
		return WSAEINVALIDPROVIDER;
		break;
	case 0: // 10106: "Service provider failed to initialize."
		return WSAEPROVIDERFAILEDINIT;
		break;
	case 0: // 10107: "System call failure."
		return WSASYSCALLFAILURE;
		break;
	case 0: // 10108: "Service not found."
		return WSASERVICE_NOT_FOUND;
		break;
	case 0: // 10109: "Class type not found."
		return WSATYPE_NOT_FOUND;
		break;
	case 0: // 10110: "No more results."
		return WSA_E_NO_MORE;
		break;
	case 0: // 10111: "Call was canceled."
		return WSA_E_CANCELLED;
		break;
	case 0: // 10112: "Database query was refused."
		return WSAEREFUSED;
		break;
	case 0: // 11001: "Host not found."
		return WSAHOST_NOT_FOUND;
		break;
	case 0: // 11002: "Nonauthoritative host not found."
		return WSATRY_AGAIN;
		break;
	case 0: // 11003: "This is a nonrecoverable error."
		return WSANO_RECOVERY;
		break;
	case 0: // 11004: "Valid name, no data record of requested type."
		return WSANO_DATA;
		break;
	case 0: // 11005: "QoS receivers."
		return WSA_QOS_RECEIVERS;
		break;
	case 0: // 11006: "QoS senders."
		return WSA_QOS_SENDERS;
		break;
	case 0: // 11007: "No QoS senders."
		return WSA_QOS_NO_SENDERS;
		break;
	case 0: // 11008: "QoS no receivers."
		return WSA_QOS_NO_RECEIVERS;
		break;
	case 0: // 11009: "QoS request confirmed."
		return WSA_QOS_REQUEST_CONFIRMED;
		break;
	case 0: // 11010: "QoS admission error."
		return WSA_QOS_ADMISSION_FAILURE;
		break;
	case 0: // 11011: "QoS policy failure."
		return WSA_QOS_POLICY_FAILURE;
		break;
	case 0: // 11012: "QoS bad style."
		return WSA_QOS_BAD_STYLE;
		break;
	case 0: // 11013: "QoS bad object."
		return WSA_QOS_BAD_OBJECT;
		break;
	case 0: // 11014: "QoS traffic control error."
		return WSA_QOS_TRAFFIC_CTRL_ERROR;
		break;
	case 0: // 11015: "QoS generic error."
		return WSA_QOS_GENERIC_ERROR;
		break;
	case 0: // 11016: "QoS service type error."
		return WSA_QOS_ESERVICETYPE;
		break;
	case 0: // 11017: "QoS flowspec error."
		return WSA_QOS_EFLOWSPEC;
		break;
	case 0: // 11018: "Invalid QoS provider buffer."
		return WSA_QOS_EPROVSPECBUF;
		break;
	case 0: // 11019: "Invalid QoS filter style."
		return WSA_QOS_EFILTERSTYLE;
		break;
	case 0: // 11020: "Invalid QoS filter type."
		return WSA_QOS_EFILTERTYPE;
		break;
	case 0: // 11021: "Incorrect QoS filter count."
		return WSA_QOS_EFILTERCOUNT;
		break;
	case 0: // 11022: "Invalid QoS object length."
		return WSA_QOS_EOBJLENGTH;
		break;
	case 0: // 11023: "Incorrect QoS flow count."
		return WSA_QOS_EFLOWCOUNT;
		break;
	case 0: // 11024: "Unrecognized QoS object."
		return WSA_QOS_EUNKOWNPSOBJ;
		break;
	case 0: // 11025: "Invalid QoS policy object."
		return WSA_QOS_EPOLICYOBJ;
		break;
	case 0: // 11026: "Invalid QoS flow descriptor."
		return WSA_QOS_EFLOWDESC;
		break;
	case 0: // 11027: "Invalid QoS provider-specific flowspec."
		return WSA_QOS_EPSFLOWSPEC;
		break;
	case 0: // 11028: "Invalid QoS provider-specific filterspec."
		return WSA_QOS_EPSFILTERSPEC;
		break;
	case 0: // 11029: "Invalid QoS shape discard mode object."
		return WSA_QOS_ESDMODEOBJ;
		break;
	case 0: // 11030: "Invalid QoS shaping rate object."
		return WSA_QOS_ESHAPERATEOBJ;
		break;
	case 0: // 11031: "Reserved policy QoS element type."
		return WSA_QOS_RESERVED_PETYPE;
		break;
	}
	return NULL;
}
#endif

/* Returns the Microsoft-specified description of a particular winsock
 * error.
 */
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
