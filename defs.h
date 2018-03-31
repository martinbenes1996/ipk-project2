#ifndef DEFS_H
#define DEFS_H

// used
 // https://cboard.cprogramming.com/c-programming/124445-dhcp-discover.html, author  Xandux
 // https://stackoverflow.com/questions/1791616/implementing-dhcp-client, author avd

struct dhcpmessage
{
    uint8_t op;       // message type
    uint8_t htype;    // hw addr type
    uint8_t hlen;     // hw addr len
    uint8_t hops;     // relay agent hops count
    uint32_t xid;     // transaction id
    uint16_t secs;    // time, since client started looking
    uint16_t flags;   // flags
    uint32_t ciaddr;  // client ip (if exists)
    uint32_t yiaddr;  // client ip
    uint32_t siaddr;  // ip of next server to talk to
    uint32_t giaddr;  // ip of dhcp relay agent
    char chaddr[16];  // client hw address
    char sname[64];   // server name
    char file[128];   // boot file
    char magic[4];    // magic number
    char opt[3];      // optional parameters
};


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

// size of packet
size_t pckt_size = sizeof(struct ether_header) // size of ethernet header
                 + sizeof(struct iphdr)        // size of ip header
                 + sizeof(struct udphdr)       // size of udp header
                 + sizeof(struct dhcpmessage); // size of dhcp message


//#define HEADER_OVERHEAD (  /*Ethernet*/14  +  /*IP*/20  +  /*UDP*/8  ) 
//
//#define SERVERNAME_LEN        64
//#define FILE_LEN              128
//#define DHCP_LEN              ( HEADER_OVERHEAD  +  /*fixed non udp*/236  )
//#define MTU_MAX               1500
//#define OPTION_LEN            ( MTU_MAX - DHCP_LEN )
//#define MIN_LEN               548
// 
//struct DHCPPacket {
//    u_int8_t  op;              /* Message type (opcode) [0]*/
//    u_int8_t  htype;           /* HW address type (net/if_types.h) [1] */
//    u_int8_t  hlen;            /* HW address length [2] */
//    u_int8_t  hops;            /* Relay agent hops count [3] */
//    u_int32_t xid;             /* Transaction ID [4] */
//    u_int16_t secs;            /* Time, since client started looking [8] */
//    u_int16_t flags;           /* Flags [10] */
//    struct in_addr ciaddr;                    /* Client IP (if exists) [12] */
//    struct in_addr yiaddr;                    /* Client IP [16] */
//    struct in_addr siaddr;                    /* IP of next server to talk to [18] */
//    struct in_addr giaddr;                    /* IP of DHCP relay agent [20] */
//    unsigned char chaddr [16];                /* Client HW address [24] */
//    char sname [SERVERNAME_LEN];              /* Server name [40] */
//    char file [FILE_LEN];                     /* Boot filename [104] */
//    unsigned char options [OPTION_LEN];  /* Optional parameters (len depends on MTU) [212] */
//};


#endif // DEFS_H