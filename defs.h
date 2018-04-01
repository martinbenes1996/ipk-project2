#ifndef DEFS_H
#define DEFS_H

// C++
#include <string>
// net
#include <sys/socket.h>
#include <sys/types.h>

#define ARG_ERR  1
#define SOCK_ERR 2
#define HW_ERR   3
#define DHCP_ERR 4

// used
 // https://cboard.cprogramming.com/c-programming/124445-dhcp-discover.html, author  Xandux
 // https://stackoverflow.com/questions/1791616/implementing-dhcp-client, author avd


/* DHCP message types. */
#define DHCPDISCOVER 1
#define DHCPOFFER    2
#define DHCPREQUEST  3
#define DHCPDECLINE  4
#define DHCPACK      5
#define DHCPNAK      6
#define DHCPRELEASE  7
#define DHCPINFORM   8

/* Possible values for hardware type (htype) field... */
#define HTYPE_ETHER   1 /* Ethernet 10Mbps              */
#define HTYPE_IEEE802 6 /* IEEE 802.2 Token Ring... */
#define HTYPE_FDDI    8 /* FDDI...          */

#define ETH_ADDR_LEN 6
#define BOOTP_BROADCAST 0x8000

#define DHCP_LEN              236
#define MTU_MAX               1500
#define OPTION_LEN            ( MTU_MAX - DHCP_LEN )
 
struct DHCPPacket {
    u_int8_t  op {0};              /* Message type (opcode) [0]*/
    u_int8_t  htype {0};           /* HW address type (net/if_types.h) [1] */
    u_int8_t  hlen {0};            /* HW address length [2] */
    u_int8_t  hops {0};            /* Relay agent hops count [3] */
    u_int32_t xid {0};             /* Transaction ID [4] */
    u_int16_t secs {0};            /* Time, since client started looking [8] */
    u_int16_t flags {0};           /* Flags [10] */
    struct in_addr ciaddr {0};                   /* Client IP (if exists) [12] */
    struct in_addr yiaddr {0};                   /* Client IP [16] */
    struct in_addr siaddr {0};                   /* IP of next server to talk to [20] */
    struct in_addr giaddr {0};                   /* IP of DHCP relay agent [24] */
    unsigned char chaddr [16] {0};               /* Client HW address [28] */
    char sname [64] {0};                         /* Server name [42] */
    char file [128] {0};                         /* Boot filename [106] */
    unsigned char opt [OPTION_LEN] {0};  /* Optional parameters (len depends on MTU) [234] */
};




std::string printDHCP(DHCPPacket& p)
{
    std::string r;
    r += "op:\t"      + std::to_string(p.op)     + "\n";
    r += "htype:\t"   + std::to_string(p.htype)  + "\n";
    r += "hlen:\t"    + std::to_string(p.hlen)   + "\n";
    r += "hops:\t"    + std::to_string(p.hops)   + "\n";
    r += "xid:\t"     + std::to_string(p.xid)    + "\n";
    r += "secs:\t"    + std::to_string(p.secs)   + "\n";
    r += "flags:\t"   + std::to_string(p.flags)  + "\n";
    r += "ciaddr:\t"  + std::to_string(p.ciaddr.s_addr) + "\n";
    r += "yiaddr:\t"  + std::to_string(p.yiaddr.s_addr) + "\n";
    r += "siaddr:\t"  + std::to_string(p.siaddr.s_addr) + "\n";
    r += "giaddr:\t"  + std::to_string(p.giaddr.s_addr) + "\n";
    r += "chaddr:\t"  + std::string((char *)p.chaddr) + "\n";
    r += "sname:\t"   + std::string(p.sname)  + "\n";
    r += "file:\t"    + std::string(p.file)   + "\n";
    r += "opt:\t"     + std::string((char *)p.opt)    + "\n";

    return r;
}

class Socket
{
        int msocket = -1;
    public: 
        Socket() { msocket = socket(AF_INET, SOCK_DGRAM, 0); }
        ~Socket() { close(msocket); }
        bool set(int flag) { int en = 1; return setsockopt(msocket, SOL_SOCKET, flag, &en, sizeof(en)) >= 0; }
        operator int() { return msocket; }
        bool opened() { return msocket != -1; } 
};


#endif // DEFS_H