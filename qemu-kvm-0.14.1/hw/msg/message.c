/*
    message.c -- Messaging API
    
    Implements a method of connecting two interfaces using a brute force 
    multicast announce on all valid sockets and then sending messages of 
    variable length between the two interfaces.

    The brute force connection method means the two machines don't have to 
    know each others IP addresses ahead of time. The limitation is they must 
    be within the same subnet.
*/

/*
    Copyright (c) 2011 Jacob Keller

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

struct announce_msg {
    uint32_t magic;
    uint32_t addr;
    uint16_t port;
    uint16_t flags;             // reserved
};

struct msg_header {
    uint16_t length;
    uint32_t sum;
};

#define PACKETS_PER_SEC 10

typedef struct if_panel {
    PANEL *hs;
    PANEL *cs;
} IF_PANEL;

typedef struct if_data {
    char if_addr[NI_MAXHOST];
    char if_name[NI_MAXHOST];
    struct sockaddr sa;
} IF_DATA;

int buildIfPanel(IF_PANEL * p, IF_DATA * i);
int populateInterfaceData(IF_DATA * if_d, int *numIfs);
uint32_t chksum(uint8_t * data, uint16_t length);


/* Fill the IF_PANEL structure
 */
int buildIfPanel(IF_PANEL * p, IF_DATA * i)
{
    int rc = 0;

    dbg(DBG_WARN, "Using interface '%s' (%s)\n", i->if_addr, i->if_name);

    p->hs = CreatePanel(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (p->hs == NULL) {
        return SOCKET_ERROR;
    }

    rc = BindPanel(p->hs, i->if_addr, OPT.mcastport, 1);
    if (rc == SOCKET_ERROR) {
        return rc;
    }

    rc = SetDestination(p->hs, OPT.mcastip, OPT.mcastport);
    if (rc == SOCKET_ERROR) {
        return rc;
    }

    rc = JoinMulticastGroup(p->hs, NULL);
    if (rc == SOCKET_ERROR) {
        return rc;
    }

    rc = SetMulticastSendInterface(p->hs, &i->sa);
    if (rc == SOCKET_ERROR) {
        return rc;
    }

    rc = SetMulticastLoopback(p->hs, 1);
    if (rc == SOCKET_ERROR) {
        return rc;
    }

    p->cs = CreatePanel(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (p->cs == NULL) {
        return SOCKET_ERROR;
    }

    rc = BindPanel(p->cs, i->if_addr, OPT.tcpport, 1);
    if (rc == SOCKET_ERROR) {
        return rc;
    }

    return 0;
}

/* Fill the IF_DATA structure
 */
int populateInterfaceData(IF_DATA * if_d, int *numIfs)
{
#ifdef _WIN32
    SOCKET s;
    int numFoundIfs = 0, i, rc = 0, pc = 0;
    INTERFACE_INFO interfaces[32];
    unsigned long nReturned = 0;

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) {
        dbg(DBG_ERROR, "socket(): '%s'\n", sock_error());
        rc = SOCKET_ERROR;
        goto err;
    }

    rc = WSAIoctl(s,
                  SIO_GET_INTERFACE_LIST,
                  0,
                  0,
                  &interfaces, sizeof(INTERFACE_INFO) * 32, &nReturned, 0, 0);

    if (rc == SOCKET_ERROR) {
        dbg(DBG_ERROR, "SIO_GET_INTERFACE_LIST: '%s'\n", sock_error());
        goto err;
    }

    shutdown(s, SD_BOTH);
    closesocket(s);

    nReturned /= sizeof(INTERFACE_INFO);
    for (i = 0; i < nReturned; i++) {
        INTERFACE_INFO *pIf = &interfaces[i];
        if ((pIf->iiFlags & IFF_UP) &&
            (pIf->iiFlags & IFF_MULTICAST) && !(pIf->iiFlags & IFF_LOOPBACK)) {

            /* We check to make sure we can hold the interface */
            if ((numFoundIfs + 1) > *numIfs) {
                numFoundIfs++;
                continue;
            }

            /* We found a useful interface */
            rc = getnameinfo(&(pIf->iiAddress.Address),
                             sizeof(struct sockaddr_in),
                             if_d[numFoundIfs].if_addr,
                             NI_MAXHOST, 0, 0, NI_NUMERICHOST);
            if (rc != 0) {
                rc = SOCKET_ERROR;
                return rc;
            }

            /* Name the adapter */
            sprintf(if_d[numFoundIfs].if_name, "ifwin%d", i);

            /* Copy the struct sockaddr */
            memcpy(&if_d[numFoundIfs].sa, &pIf->iiAddress.Address,
                   sizeof(struct sockaddr));

            /* Increment the total number of adapters */
            numFoundIfs++;
        }
    }

    /* return an error if we didn't have enough storage */
    if (numFoundIfs > *numIfs) {
        set_error(ENOMEM);
        dbg(DBG_ERROR, "populateInterfaceData(): not enough memory\n");
        rc = SOCKET_ERROR;
    }

    *numIfs = numFoundIfs;
    return rc;

 err:
    return rc;
#else
    struct ifaddrs *ifaddrs = NULL;
    struct ifaddrs *ifa = NULL;

    int numFoundIfs = 0;
    int rc = 0;

    rc = getifaddrs(&ifaddrs);
    if (rc == SOCKET_ERROR) {
        dbg(DBG_ERROR, "getifaddrs(): %s\n", sock_error());
        return rc;
    }

    for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;

        if ((ifa->ifa_flags & IFF_UP) &&
            (ifa->ifa_flags & IFF_MULTICAST) &&
            !(ifa->ifa_flags & IFF_LOOPBACK)) {
            /* If we get here, then we want to use this interface
             * NOTE: first check that we have space. */
            if ((numFoundIfs + 1) > *numIfs) {
                numFoundIfs++;
                continue;
            }

            rc = getnameinfo(ifa->ifa_addr,
                             sizeof(struct sockaddr_in),
                             if_d[numFoundIfs].if_addr,
                             NI_MAXHOST, 0, 0, NI_NUMERICHOST);
            if (rc != 0) {
                rc = SOCKET_ERROR;
                return rc;
            }

            /* Copy relevant data into the IF_DATA buffer */
            strncpy(if_d[numFoundIfs].if_name, ifa->ifa_name, NI_MAXHOST);
            memcpy(&if_d[numFoundIfs].sa, ifa->ifa_addr,
                   sizeof(struct sockaddr));

            /* Increment the total number of interfaces */
            numFoundIfs++;
        }
    }

    /* Didn't have enough memory */
    if (numFoundIfs > *numIfs) {
        set_error(ENOMEM);
        dbg(DBG_ERROR, "populateInterfaceData(): not enough memory\n");
        rc = SOCKET_ERROR;
    }

    *numIfs = numFoundIfs;
    freeifaddrs(ifaddrs);
    return rc;
#endif
}

/* anounce()
 * 
 * the announce function will multicast a message
 * announcing this functions service via a unique
 * magic number.
 *
 * Returns a socket
 */
#define MAX_IF_LENGTH 32
SOCKET announce(const char *optrc)
{
    IF_PANEL ifp[MAX_IF_LENGTH];
    IF_DATA ifd[MAX_IF_LENGTH];
    int ifd_length = MAX_IF_LENGTH;
    fd_set readfds;
    struct timeval tv;
    int num_packets = 0;
    int count = 0;
    int maxSocket = 0;
    char addr[NI_MAXHOST];
    struct announce_msg m;
    socklen_t acceptlen;
    int accepted = 0;
    int i;
    SOCKET socket = INVALID_SOCKET;
    int rc = 0;

    ReadOptions(optrc);

    memset(ifp, 0, sizeof(IF_PANEL) * 32);

    rc = populateInterfaceData(ifd, &ifd_length);
    if (rc == SOCKET_ERROR) {
        dbg(DBG_ERROR, "populateInterfaceData(): '%s'\n", sock_error());
        goto err;
    }

    /* We have the interface data in useable form
     * Now we can build the interface panels */
    for (i = 0; i < ifd_length; i++) {
        rc = buildIfPanel(&ifp[i], &ifd[i]);
        if (rc == SOCKET_ERROR) {
            dbg(DBG_ERROR, "buildIfPanel(%d): '%s'\n", i, sock_error());
            goto err;
        }
    }

    /* Listen on each interface */
    for (i = 0; i < ifd_length; i++) {
        rc = listen(ifp[i].cs->sp_socket, 1);
        if (rc == SOCKET_ERROR) {
            dbg(DBG_ERROR, "listen(cs): '%s'\n", sock_error());
            goto err;
        }
    }

    num_packets = OPT.timeout * PACKETS_PER_SEC;

    /* the funny condition causes us to loop forever
       if we have a timeout of 0 */
    for (count = 0; (num_packets == 0 || count < num_packets); count++) {
        m.magic = htonl(OPT.magicnum);
        m.port = htons(atoi(OPT.tcpport));
        m.flags = 0;

        for (i = 0; i < ifd_length; i++) {
            m.addr =
                ((struct sockaddr_in *)&(ifp[i].cs->sp_bind))->sin_addr.s_addr;
            rc = sendto(ifp[i].hs->sp_socket, (const char *)&m, sizeof(m), 0,
                        &(ifp[i].hs->sp_dest), sizeof(ifp[i].hs->sp_dest));
            if (rc == SOCKET_ERROR) {
                dbg(DBG_ERROR, "sendto(hs): %s\n", sock_error());
                goto err;
            }
        }

        dbg(DBG_VERB, "Sent announce msg %d to %s\n", count,
            FormatAddr(&(ifp[0].hs->sp_dest), addr, NI_MAXHOST));

        FD_ZERO(&readfds);

        for (i = 0; i < ifd_length; i++) {
            if (ifp[i].cs->sp_socket > maxSocket) {
                maxSocket = ifp[i].cs->sp_socket;
            }
            FD_SET(ifp[i].cs->sp_socket, &readfds);
        }

        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        rc = select(maxSocket + 1, &readfds, NULL, NULL, &tv);
        if (rc < 0) {
            dbg(DBG_ERROR, "select(cs): %s\n", sock_error());
            goto err;
        } else if (rc == 0) {
            continue;
        } else {
            for (i = 0; i < ifd_length; i++) {
                if (FD_ISSET(ifp[i].cs->sp_socket, &readfds)) {
                    acceptlen = sizeof(struct sockaddr);
                    socket =
                        accept(ifp[i].cs->sp_socket, &(ifp[i].cs->sp_dest),
                               &acceptlen);
                    accepted = i;
                    dbg(DBG_STATUS, "connected to %s\n",
                        FormatAddr(&(ifp[i].cs->sp_dest), addr, NI_MAXHOST));
                    break;
                }
            }

            /* we have a connection */
            if (socket != INVALID_SOCKET) {
                break;
            }
        }
    }

    for (i = 0; i < ifd_length; i++) {
        if (socket == ifp[i].cs->sp_socket) {
            DissociatePanel(ifp[i].cs);
        } else {
            FreePanel(ifp[i].cs);
        }
        rc = LeaveMulticastGroup(ifp[i].hs, NULL);
        if (rc == SOCKET_ERROR) {
            goto err;
        }
        FreePanel(ifp[i].hs);
    }

    return socket;

 err:
    for (i = 0; i < ifd_length || i < MAX_IF_LENGTH; i++) {
        FreePanel(ifp[i].hs);
        FreePanel(ifp[i].cs);
    }

    return INVALID_SOCKET;
}

/* locate()
 * 
 * the locate function will listen for incoming announcement messages
 * and will connect to the first valid server
 * 
 * Returns a socket
 */
SOCKET locate(const char *optrc)
{
    PANEL *hs = NULL;
    PANEL *cs = NULL;

    IF_DATA ifd[MAX_IF_LENGTH];
    int ifd_length = MAX_IF_LENGTH;
    int i = 0;

    fd_set readfds;
    struct timeval tv;

    struct announce_msg m;

    char addr[NI_MAXHOST];

    socklen_t recvlen;

    SOCKET socket = INVALID_SOCKET;
    int rc = 0;

    ReadOptions(optrc);

    rc = populateInterfaceData(ifd, &ifd_length);
    if (rc == SOCKET_ERROR) {
        dbg(DBG_ERROR, "populateInterfaceData(): '%s'\n", sock_error());
        goto err;
    }

    hs = CreatePanel(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (!hs) {
        dbg(DBG_ERROR, "CreateBoundPanel(hs): %s\n", sock_error());
        goto err;
    }

    rc = BindPanel(hs, NULL, OPT.mcastport, 1);
    if (rc == SOCKET_ERROR) {
        dbg(DBG_ERROR, "BindPanel(hs), '%s'\n", sock_error());
        goto err;
    }

    rc = SetDestination(hs, OPT.mcastip, OPT.mcastport);
    if (rc == SOCKET_ERROR) {
        dbg(DBG_ERROR, "SetDestination(hs): %s\n", sock_error());
        goto err;
    }

    for (i = 0; i < ifd_length; i++) {
        rc = JoinMulticastGroup(hs, ifd[i].if_addr);
        if (rc == SOCKET_ERROR) {
            dbg(DBG_ERROR, "MakeMulticast(hs): %s\n", sock_error());
            goto err;
        }
    }

    rc = SetMulticastLoopback(hs, 1);
    if (rc == SOCKET_ERROR) {
        dbg(DBG_ERROR, "SetMulticastLoopback(hs): %s\n", sock_error());
        goto err;
    }

    cs = CreateBoundPanel("0", AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (!cs) {
        dbg(DBG_ERROR, "CreateBoundPanel(cs): %s\n", sock_error());
        goto err;
    }

    tv.tv_sec = OPT.timeout;
    tv.tv_usec = 0;

    while (socket == INVALID_SOCKET) {
        FD_ZERO(&readfds);
        FD_SET(hs->sp_socket, &readfds);

        rc = select(hs->sp_socket + 1, &readfds, NULL, NULL,
                    (OPT.timeout > 0 ? &tv : NULL));
        if (rc < 0) {
            dbg(DBG_ERROR, "select(hs): %s\n", sock_error());
            goto err;
        } else if (rc == 0) {
            socket = INVALID_SOCKET;
            break;
        }

        /* if we get here, select woke up with a valid packet */

        recvlen = sizeof(struct sockaddr);
        rc = recvfrom(hs->sp_socket, &m, sizeof(m), 0, &(cs->sp_dest),
                      &recvlen);
        if (rc == SOCKET_ERROR) {
            dbg(DBG_ERROR, "recvfrom(hs): %s\n", sock_error());
            goto err;
        }

        m.magic = ntohl(m.magic);

        dbg(DBG_WARN, "received announce (magic=%d) from %s\n",
            m.magic, FormatAddr(&(cs->sp_dest), addr, NI_MAXHOST));

        if (m.magic == OPT.magicnum) {
            ((struct sockaddr_in *)&(cs->sp_dest))->sin_addr.s_addr = m.addr;
            ((struct sockaddr_in *)&(cs->sp_dest))->sin_port = m.port;
            rc = connect(cs->sp_socket, &(cs->sp_dest),
                         sizeof(struct sockaddr));
            if (rc == SOCKET_ERROR) {
                dbg(DBG_ERROR, "connect(cs): %s\n", sock_error());
                goto err;
            } else {
                dbg(DBG_STATUS, "connected to %s\n",
                    FormatAddr(&(cs->sp_dest), addr, NI_MAXHOST));
                socket = cs->sp_socket;
            }
        }
    }

    FreePanel(hs);
    DissociatePanel(cs);

    return socket;

 err:
    if (hs) {
        FreePanel(hs);
    }

    if (cs) {
        FreePanel(cs);
    }

    return INVALID_SOCKET;
}

uint32_t chksum(uint8_t * data, uint16_t length)
{
    uint32_t sum = 0;
    uint32_t value = 0;
    int extra = length % 4;
    int i = 0;

    for (i = 0; i < (length - extra); i += 4) {
        value = data[i];
        value |= data[i + 1] << 8;
        value |= data[i + 2] << 16;
        value |= data[i + 3] << 24;

        sum ^= value;
    }

    if (extra > 0) {
        value = (extra >= 1 ? data[i] : 0);
        value |= (extra >= 2 ? data[i + 1] : 0) << 8;
        value |= (extra >= 3 ? data[i + 2] : 0) << 16;
        sum ^= value;
    }

    return sum;
}

/* senddata()
 * 
 * this function will send a message of raw bytes of a certain length
 * across the socket, by first sending the length in a fixed value
 * "header", then sending the buffer.
 */
int senddata(SOCKET socket, void *data, uint16_t length)
{
    int rc = 0;
    struct msg_header m;

    if (length <= 0) {
        set_error(EINVAL);
        return -1;
    }

    m.length = htons(length);
    m.sum = htonl(chksum(data, length));

    rc = send(socket, &m, sizeof(struct msg_header), 0);
    if (rc == 0) {
        set_error(ECANCELED);
        return -1;
    } else if (rc < 0) {
        return -1;
    }

    dbg(DBG_VERB, "senddata: send length=%u\n", length);

    rc = send(socket, data, length, 0);
    if (rc == 0) {
        set_error(ECANCELED);
        return -1;
    } else if (rc < 0) {
        return rc;
    }

    return rc;
}

/* recvdata()
 *
 * this function will receive a message from the socket
 * and put its value into the buffer. It will return an
 * error code if the given buffer is not large enough to hold
 * all of the data
 *
 * it will modify the length value to be the actual
 * length of the message returned.
 */
int recvdata(SOCKET socket, void *data, uint16_t * length)
{
    int rc = 0;
    int recv_length = sizeof(struct msg_header);
    int recv_sum = 0;
    struct msg_header m;
    struct msg_header b;

    /* PEEK at the message length 
     * necessary to verify that buffer is large enough
     * and still allow receive of message after check
     */
    rc = recv(socket, &m, recv_length, MSG_PEEK);
    if (rc == 0) {
        set_error(ECANCELED);
        return -1;
    } else if (rc < 0) {
        return -1;
    } else if (rc != sizeof(struct msg_header)) {
        set_error(ECANCELED);
        return -1;
    }

    m.length = ntohs(m.length);
    m.sum = ntohl(m.sum);
    if (m.length > *length) {
        set_error(ENOMEM);
        return -1;
    }

    /* remove the header from the socket stream
     * necessary because we peeked at the message
     * length before hand.
     * NOTE: this header will not be removed
     * if the buffer was not long enough
     */
    recv_length = sizeof(struct msg_header);
    rc = recv(socket, &b, recv_length, 0);
    if (rc == 0) {
        set_error(ECANCELED);
        return -1;
    } else if (rc < 0) {
        return -1;
    }

    dbg(DBG_VERB, "recvdata: recv length %u\n", m.length);

    /* receive the raw data
     * now we actually retrieve the data
     * and error out if we don't receive
     * the same amount as specified
     * in the msg_length
     */
    rc = recv(socket, data, m.length, 0);
    if (rc == 0) {
        set_error(ECANCELED);
        return -1;
    } else if (rc < 0) {
        return -1;
    } else if (rc != m.length) {
        set_error(ECANCELED);
        return -1;
    }

    recv_sum = chksum(data, m.length);
    if (m.sum ^ recv_sum) {
        dbg(DBG_WARN, "chksum expected %u\n", m.sum);
        dbg(DBG_WARN, "chksum computed %u\n", recv_sum);
    }

    /* set the length to the actual received length */
    *length = m.length;
    return 0;
}
