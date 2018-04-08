#ifndef IPK_H
#define IPK_H

// C++
#include <string>
// net
#include <sys/socket.h>
#include <sys/types.h>

#define ARG_ERR  1
#define SOCK_ERR 2
#define HW_ERR   3

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
#define BOOTP_BROADCAST 0x0080

#define DHCP_LEN              236
#define MTU_MAX               1500
#define OPTION_LEN            ( MTU_MAX - DHCP_LEN )
 
/** @brief DHCP header. */
struct DHCPPacket {

    u_int8_t  op {0};                   /* Message type (opcode) [0]*/
    u_int8_t  htype {HTYPE_ETHER};      /* HW address type (net/if_types.h) [1] */
    u_int8_t  hlen {ETH_ADDR_LEN};      /* HW address length [2] */
    u_int8_t  hops {0};                 /* Relay agent hops count [3] */
    u_int32_t xid {0};                  /* Transaction ID [4] */
    u_int16_t secs {0};                 /* Time, since client started looking [8] */
    u_int16_t flags {BOOTP_BROADCAST};  /* Flags [10] */
    in_addr_t ciaddr {0};               /* Client IP (if exists) [12] */
    in_addr_t yiaddr {0};               /* Client IP [16] */
    in_addr_t siaddr {0};               /* IP of next server to talk to [20] */
    in_addr_t giaddr {0};               /* IP of DHCP relay agent [24] */
    unsigned char chaddr [16] {0};      /* Client HW address [28] */
    char sname [64] {0};                /* Server name [42] */
    char file [128] {0};                /* Boot filename [106] */
    unsigned char opt [OPTION_LEN] = {0x63, 0x82, 0x53, 0x63, 53, 0x01, 0x01, 0xFF};
                                        /* Optional parameters (len depends on MTU) [234] */
};


/**
 * @brief Socket class to get RAII programming style.
 */
class Socket
{
        int msocket = -1; /**< Saved socket descriptor. */
    public: 
        /** @brief Constructor. */
        Socket() { msocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); }
        /** @brief Destructor. */
        ~Socket() { close(msocket); }
        /** @brief Int conversion (equivalent with BSD socket, aka int) */
        operator int() { return msocket; }

        /**
         * @brief Setter of the flag on the socket.
         * @param flag      Flag to set.
         * @returns Whether the setting was successful.
         */
        bool set(int flag) { int en = 1; return setsockopt(msocket, SOL_SOCKET, flag, &en, sizeof(en)) >= 0; }
        /** @brief Whether the socket was successfully created. */
        bool opened() { return msocket != -1; } 
};


#endif // IPK_H