#include "panel.h"
#include "opt.h"

int main(int argc, char **argv) {
    SOCKET s;
    int nNumInterfaces, i, nRet, rc = 0, pc = 0;
    INTERFACE_INFO interfaces[32];
    PANEL *p[32];
    char ipaddr[16];
    unsigned long nReturned = 0;

#ifdef _WIN32
    WSADATA wsaData;
    rc = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (rc != 0) {
        printf("WSAStartup failed with error: %d\n", rc);
        goto err;
    }	
#endif

    s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    nRet = WSAIoctl(s, // socket handle
        SIO_GET_INTERFACE_LIST, 
        0, 
        0, 
        &interfaces,
        sizeof(INTERFACE_INFO) * 32,
        &nReturned,
        0, 
        0);

    if (nRet == SOCKET_ERROR)
    {
        int nError = WSAGetLastError();
        printf("Error getting interface list: %i\n", nError);
        return;
    }

    nNumInterfaces = nReturned / sizeof(INTERFACE_INFO);

    for (i = 0; i < nNumInterfaces; i++)
    {
        INTERFACE_INFO *pIf = &interfaces[i];
        
        if ((pIf->iiFlags & IFF_LOOPBACK) != 0) {
            PrintAddr(stdout, &((pIf->iiAddress).Address));
            printf(" is a loopback device.\n");
            continue;
        }

        if ((pIf->iiFlags & IFF_UP) == 0) {
            PrintAddr(stdout, &((pIf->iiAddress).Address));
            printf(" is down.\n");
            continue;
        }

        FormatAddr(&((pIf->iiAddress).Address), ipaddr, 16);
        printf("Creating multicast panel for %s\n.", ipaddr);
        p[pc] = SocketToPanel(socket(AF_INET, SOCK_DGRAM, IPPROTO_IP), &(pIf->iiAddress.Address));
        rc = BindPanel(p[pc], ipaddr, OPT.mcastport, 1);
        rc = SetDestination(p[pc], OPT.mcastip, OPT.mcastport);
        rc = JoinMulticastGroup(p[pc]);
        rc = SetMulticastSendInterface(p[pc], &(pIf->iiAddress.Address));
        rc = SetMulticastTTL(p[pc], OPT.mcastttl);
        rc = SetMulticastLoopback(p[pc], 1);
    }

err:
#ifdef _WIN32
	WSACleanup();
#else
    close(socket);
#endif
}
