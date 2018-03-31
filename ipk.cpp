
// C
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <cstring>
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
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "defs.h"

#define ARG_ERR  1
#define SOCK_ERR 2
#define HW_ERR   3

/**
 * @brief SIGINT handler.
 */
void sigint(int);

/*
 * @brief Checksum calc function.
 */
unsigned short csum(unsigned short *ptr,int nbytes);



int sock = -1; // socket
extern size_t pckt_size; // size of packet

void createSocket();
void send_DHCP_DISCOVER(char * ifce);

using namespace std;
int main(int argc, char *argv[])
{
    // argument processing
    if(argc != 3 || string(argv[1]) != "-i")
    {
        cerr << "Usage: ./ipkdhcpstarve -i <interface>\n";
        exit(ARG_ERR);
    }
    char * ifce = argv[2];

    createSocket();
    send_DHCP_DISCOVER(ifce);

    return 0;

    
}

#include <errno.h>

void createSocket()
{
    //  create socket
    if( (sock = socket(PF_INET, SOCK_RAW, IPPROTO_UDP)) < 0 ) { cerr << strerror(errno) << "\n"; cerr << "Fail creating a socket.\n"; exit(SOCK_ERR); }

    // set socket options
    int en = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &en, sizeof(int)) < 0) { cerr << "Fail setting raw socket options.\n"; exit(SOCK_ERR); } // tells kernel, that headers are included
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(int)) < 0) { cerr << "Fail setting raw socket options.\n"; exit(SOCK_ERR); }
        
}
void send_DHCP_DISCOVER(char * ifce)
{
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, ifce, strlen(ifce)) < 0) { cerr << "Fail setting raw socket options.\n"; exit(SOCK_ERR); }
    unsigned char *pckt = new unsigned char [pckt_size];

    struct ether_header * eth  = (struct ether_header *) (pckt);
    struct iphdr        * ip   = (struct iphdr        *) (pckt + sizeof(struct ether_header));
    struct udphdr       * udp  = (struct udphdr       *) (pckt + sizeof(struct iphdr) + sizeof(struct ether_header));
    struct dhcpmessage  * dhcp = (struct dhcpmessage  *) (pckt + sizeof(struct udphdr) + sizeof(struct iphdr) + sizeof(struct ether_header));

    struct ifreq ifreq;
    char srcHwAddr[18];
    
    if(sizeof(ifce) >= IFNAMSIZ) { std::cerr << "Interface name too long.\n"; exit(HW_ERR); }
    strcpy(ifreq.ifr_name, ifce); // copy interface
 
    // get hw access
    if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0) { cerr << "Fail controlling " << ifce << " interface.\n"; exit(HW_ERR); } 

    // get hw address
    sprintf(srcHwAddr, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char) ifreq.ifr_hwaddr.sa_data[0], (unsigned char) ifreq.ifr_hwaddr.sa_data[1],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[2], (unsigned char) ifreq.ifr_hwaddr.sa_data[3], (unsigned char) ifreq.ifr_hwaddr.sa_data[4],
            (unsigned char) ifreq.ifr_hwaddr.sa_data[5]);
    std::cout << srcHwAddr << "\n";


    /* ------- ETHERNET ------- */
    memcpy(eth->ether_dhost, ether_aton("ff:ff:ff:ff:ff:ff"), ETH_ALEN); // dest ethernet addr
    memcpy(eth->ether_shost, ether_aton(srcHwAddr), ETH_ALEN); // src ethernet addr
    eth->ether_type = htons(ETH_P_IP); // ethernet type
    /* ----------------------- */


    /* --------- IP ---------- */
    ip->tos = 0; // type of service
    ip->version = 4; // ip version
    ip->ihl = sizeof(struct iphdr) >> 2; // ip header length
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr)  + sizeof(struct dhcpmessage)); // total length
    ip->id = htons((int) (rand() / (((double) RAND_MAX + 1) / 14095))); // id of the packet
    ip->frag_off = 0; // fragment offset
    ip->ttl = 128; // TTL (time to live)
    ip->protocol = IPPROTO_UDP; // UDP protocol
    ip->saddr = inet_addr("0.0.0.0"); // src ip
    ip->daddr = inet_addr("255.255.255.255"); // dst ip
    ip->check = csum ((unsigned short *) pckt, ip->tot_len); // ip sets checksum
    /* ----------------------- */


    /* --------- UDP --------- */
    udp->source = htons(67); // src port
    udp->dest = htons(68); // dst port
    udp->len = htons(sizeof(struct udphdr) + sizeof(struct dhcpmessage)); // UDP length (incl DHCP packet)
    /* ----------------------- */
 
    
    /* --------- DHCP -------- */
    dhcp->op = DHCPDISCOVER; // msg type
    dhcp->htype = HTYPE_ETHER; // hw addr type
    dhcp->hlen = ETH_ADDR_LEN; // hw addr len
    dhcp->hops = 0; // relay agent hops count
    dhcp->secs = 0; // time since client started looking
    dhcp->flags = htons(BOOTP_BROADCAST); // flags
    inet_aton("0.0.0.0", (struct in_addr *) &dhcp->ciaddr); // client ip
    inet_aton("0.0.0.0", (struct in_addr *) &dhcp->yiaddr); // client ip
    inet_aton("0.0.0.0", (struct in_addr *) &dhcp->siaddr); // ip of next server to talk to
    inet_aton("0.0.0.0", (struct in_addr *) &dhcp->giaddr); // ip of dhcp relay agent
    memcpy(dhcp->chaddr, &ifreq.ifr_addr, ETHER_ADDR_LEN); // copy mac addr for ifreq
    bzero(dhcp->sname, sizeof(dhcp->sname)); // server name must be null
    bzero(dhcp->file, sizeof(dhcp->file)); // file name must be null
    bzero(dhcp->opt, sizeof(dhcp->opt)); // must be filled in
    /* ----------------------- */
     

    // set data in addr
    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(struct sockaddr_ll));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifreq.ifr_ifindex;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifreq.ifr_ifindex;
    
    if (ioctl(sock, SIOCGIFINDEX, &ifreq) < 0) { cerr << "ioctl get index.\n"; exit(HW_ERR); }

    std::cout << "ifce index " << ifreq.ifr_ifindex << "\n";

    // bind the socket on a address
    int a;
    if( (a = bind(sock, (struct sockaddr *)&addr, sizeof(addr))) != 0) 
    {
        cerr << strerror(errno) << "\n";
        std::cerr << "Fail binding a socket.\n"; 
        exit(SOCK_ERR);
    }
 
    // write the packet to the socket
    int n = 0;
    if ((n = write(sock, pckt, pckt_size)) <= 0) { std::cerr << "Packet sending error!\n"; exit(SOCK_ERR); }
    std::cout << n << " B sent.\n";
 
    close(sock);
    delete [] pckt;
}

unsigned short csum(unsigned short *ptr,int nbytes) 
{
    register long sum;
    unsigned short oddbyte;
    register short answer;
 
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
 
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
     
    return(answer);
}

void sigint(int) { exit(0); }