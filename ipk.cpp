
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
Socket sock;           /**< Socket for communication. */
u_int32_t xid;         /**< Transaction ID. */
struct in_addr yiaddr; /**< Assigned address. */
struct ifreq ifr;      /**< Network device configuration. */
/* ---------------- */

/**
 * @brief Creates socket.
 * @param ifce      Interface name.
 */
void createSocket(char * ifce);
/**
 * @brief Sends broadcast DHCP Discover packet.
 */
void send_DHCP_DISCOVER();
/**
 * @brief Expects receiving of DHCP Offer packet.
 */
void receive_DHCP_OFFER();
/**
 * @brief Sends broadcast DHCP Request packet.
 */
void send_DHCP_REQUEST();
/**
 * @brief Expects receiving of DHCP Ack packet.
 */
void receive_DHCP_ACKNOWLEDGE();


/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
    // argument processing
    if(argc != 3 || string(argv[1]) != "-i")
    { cerr << "Usage: ./ipkdhcpstarve -i <interface>\n"; exit(ARG_ERR); }
    char * ifce = argv[2];

    srand(time(NULL)); // sets rand()

    createSocket(ifce); // create socket, connect to the interface

    // DHCP communication
    send_DHCP_DISCOVER();       // send DHCP Discover
    receive_DHCP_OFFER();       // receive DHCP Offer
    send_DHCP_REQUEST();        // send DHCP Request 
    receive_DHCP_ACKNOWLEDGE(); // receive DHCP Acknowledge

    std::cout << "Assigned address: " << yiaddr.s_addr << "\n";
    return 0;
}

void createSocket(char * ifce)
{
    //  create socket
    if(!sock.opened()) { cerr << "socket() failed.\n"; exit(SOCK_ERR); } 
    //else { cerr << "socket() succeeded.\n"; }

    // sets debug
    if(!sock.set(SO_DEBUG)) 
    { cerr << "setsockopt(SO_DEBUG) failed.\n"; exit(SOCK_ERR); } 
    //else { cerr << "setsockopt(SOL_DEBUG) succeeded.\n"; }

    // sets reuse address
    if(!sock.set(SO_REUSEADDR)) 
    { cerr << "setsockopt(SO_REUSEADDR) failed.\n"; exit(SOCK_ERR); } 
    //else { cerr << "setsockopt(SOL_REUSEADDR) succeeded.\n"; }

    // sets broadcast
    if(!sock.set(SO_BROADCAST)) 
    { cerr << "setsockopt(SO_BROADCAST) failed.\n"; exit(SOCK_ERR); } 
    //else { cerr << "setsockopt(SOL_BROADCAST) succeeded.\n"; }

    // get hw access
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifce, IFNAMSIZ);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
    { cerr << "ioctl(" << ifce << ") failed.\n"; exit(HW_ERR); } 
    //else { cerr << "ioctl(" << ifce << ") succeeded.\n"; }

    // get interface mac address
    //char srcHwAddr[18];
    //sprintf(srcHwAddr, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char) ifr.ifr_hwaddr.sa_data[0], (unsigned char) ifr.ifr_hwaddr.sa_data[1], (unsigned char) ifr.ifr_hwaddr.sa_data[2], (unsigned char) ifr.ifr_hwaddr.sa_data[3], (unsigned char) ifr.ifr_hwaddr.sa_data[4], (unsigned char) ifr.ifr_hwaddr.sa_data[5]);
    //std::cout << srcHwAddr << "\n";

    // sets socket to device
    if(setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (void*)&ifr, sizeof(ifr)) < 0)
    { cerr << "setsockopt(SO_BINDTODEVICE) failed.\n"; exit(SOCK_ERR); } 
    //else { cerr << "setsockopt(SO_BINDTODEVICE) succeeded.\n"; }

    // gets interface index
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0)
    { cerr << "ioctl(SIOCGIFINDEX) failed.\n"; exit(HW_ERR); } 
    //else { cerr << "ioctl(SIOCGIFINDEX) succeeded.\n"; }

}
void send_DHCP_DISCOVER()
{
    DHCPPacket dhcp;
    
    /* --------- DHCP -------- */
    dhcp.op = DHCPDISCOVER; // msg type
    dhcp.htype = HTYPE_ETHER; // hw addr type
    dhcp.hlen = ETH_ADDR_LEN; // hw addr len
    dhcp.hops = 0; // relay agent hops count
    dhcp.xid = xid = rand(); // transaction id
    dhcp.secs = 0; // time since client started looking
    dhcp.flags = htons(BOOTP_BROADCAST); // flags
    inet_aton("0.0.0.0", (struct in_addr *) &dhcp.ciaddr); // client ip
    inet_aton("0.0.0.0", (struct in_addr *) &dhcp.yiaddr); // client ip
    inet_aton("0.0.0.0", (struct in_addr *) &dhcp.siaddr); // ip of next server to talk to
    inet_aton("0.0.0.0", (struct in_addr *) &dhcp.giaddr); // ip of dhcp relay agent
    memcpy(dhcp.chaddr, &ifr.ifr_addr, ETHER_ADDR_LEN); // copy mac addr for ifreq
    /* ----------------------- */
    
    /* ----- address ----- */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(/*"192.168.0.255"*/"0.0.0.0");
    addr.sin_port = htons(67);
    /* ------------------ */

    int b;
    if((b = sendto(sock, &dhcp, sizeof(dhcp), 0, (struct sockaddr *)&addr, sizeof(addr))) < 0)
    { cerr << "sendto() failed.\n"; exit(SOCK_ERR); }
    else { cerr << "sendto() succeeded.\n"; }
}

void receive_DHCP_OFFER()
{
    DHCPPacket dhcp;
    /* ----- address ----- */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(68);
    socklen_t addrlen = sizeof(addr);
    /* ------------------ */

    int b;
    if((b = recvfrom(sock, &dhcp, sizeof(dhcp), 0, (struct sockaddr *)&addr, &addrlen)) < 0)
    { cerr << "recvfrom() failed.\n"; exit(SOCK_ERR); }
    else { cerr << "recvfrom() succeeded.\n"; }

    if(dhcp.op == DHCPOFFER && dhcp.xid == xid) { yiaddr = dhcp.yiaddr; }
    else { cerr << "DHCP OFFER expected.\n"; exit(DHCP_ERR); }
}

void send_DHCP_REQUEST()
{

}

void receive_DHCP_ACKNOWLEDGE()
{

}

void sigint(int) { exit(0); }