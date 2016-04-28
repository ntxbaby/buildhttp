#ifndef _PACKET_H_
#define _PACKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <netinet/in.h>
#include <sys/time.h>
#include "PacketParser.h"
     
#define CLIENT_TO_WEB 0
#define WEB_TO_CLIENT 1

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#if !defined HAVE_PACKET
// these are bits in th_flags:
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_ECE  0x40
#define TH_CWR  0x80

typedef struct _EtherHdr
{
    uint8_t ether_dst[6];
    uint8_t ether_src[6];
    uint16_t ether_type;

} EtherHdr;

/* PPPoEHdr Header; EtherHdr plus the PPPoE Header */
typedef struct _PPPoEHdr
{
    unsigned char ver_type;     /* pppoe version/type */
    unsigned char code;         /* pppoe code CODE_* */
    unsigned short session;     /* session id */
    unsigned short length;      /* payload length */
                                /* payload follows */
} PPPoEHdr;

/* tcpdump shows us the way to cross platform compatibility */
#define IP_VER(iph)    (((iph)->ip_verhl & 0xf0) >> 4)
#define IP_HLEN(iph)   ((iph)->ip_verhl & 0x0f)

typedef struct _IPHdr
{
    uint8_t ip_verhl;      /* version & header length */
    uint8_t ip_tos;        /* type of service */
    uint16_t ip_len;       /* datagram length */
    uint16_t ip_id;        /* identification  */
    uint16_t ip_off;       /* fragment offset */
    uint8_t ip_ttl;        /* time to live field */
    uint8_t ip_proto;      /* datagram protocol */
    uint16_t ip_csum;      /* checksum */
    struct in_addr ip_src;  /* source IP */
    struct in_addr ip_dst;  /* dest IP */
} IPHdr;

#define TCP_HLEN(tcph)  (((tcph)->th_offx2 & 0xf0) >> 4)
typedef struct _TCPHdr
{
    uint16_t th_sport;     /* source port */
    uint16_t th_dport;     /* destination port */
    uint32_t th_seq;       /* sequence number */
    uint32_t th_ack;       /* acknowledgement number */
    uint8_t th_offx2;      /* offset and reserved */
    uint8_t th_flags;
    uint16_t th_win;       /* window */
    uint16_t th_sum;       /* checksum */
    uint16_t th_urp;       /* urgent pointer */

} TCPHdr;

typedef struct _UDPHdr
{
    uint16_t uh_sport;
    uint16_t uh_dport;
    uint16_t uh_len;
    uint16_t uh_chk;
} UDPHdr;

typedef struct _daq_pkthdr
{
    struct timeval ts;      /* Timestamp */
    uint32_t caplen;        /* Length of the portion present */
    uint32_t pktlen;        /* Length of this packet (off wire) */
    int32_t ingress_index;  /* Index of the inbound interface. */
    int32_t egress_index;   /* Index of the outbound interface. */
    int32_t ingress_group;  /* Index of the inbound group. */
    int32_t egress_group;   /* Index of the outbound group. */
    uint32_t flags;         /* Flags for the packet (DAQ_PKT_FLAG_*) */
    uint32_t opaque;        /* Opaque context value from the DAQ module or underlying hardware.
                               Directly related to the opaque value in FlowStats. */
    void *priv_ptr;         /* Private data pointer */
    uint16_t address_space_id; /* Unique ID of the address space */
} DAQ_PktHdr_t;

// #define IsPPPoE(p) (p && p->pppoeh) // FIXME: use p->proto_bits
// #define IPH_IS_VALID(p) (p && p->iph) // FIXME: just for test, don't put this code in release version
// #define IsIP(p) (IPH_IS_VALID(p)) // FIXME: use p->proto_bits
// #define IsTCP(p) (IsIP(p) && p->tcph) // FIXME: use p->proto_bits
// #define IsUDP(p) (IsIP(p) && p->udph) // FIXME: use p->proto_bits

#define PROTO_BIT__NONE     0x0000
#define PROTO_BIT__IP       0x0001
#define PROTO_BIT__ARP      0x0002
#define PROTO_BIT__TCP      0x0004
#define PROTO_BIT__UDP      0x0008
#define PROTO_BIT__ICMP     0x0010
#define PROTO_BIT__TEREDO   0x0020
#define PROTO_BIT__GTP      0x0040
#define PROTO_BIT__PPPOE    0x0080 // TODO: should add vpn decoder
#define PROTO_BIT__OTHER    0x8000
#define PROTO_BIT__ALL      0xffff

#define IsPPPoE(p) (p && (p->proto_bits & PROTO_BIT__PPPOE) != 0)
#define IPH_IS_VALID(p) (p && (p->proto_bits & PROTO_BIT__IP) != 0)
#define IsIP(p) (IPH_IS_VALID(p))
#define IsTCP(p) (IsIP(p) && (p->proto_bits & PROTO_BIT__TCP) != 0)
#define IsUDP(p) (IsIP(p) && (p->proto_bits & PROTO_BIT__UDP) != 0)
#endif // #if !defined HAVE_PACKET

typedef struct _SimpPacket
{
    uint16_t proto_bits;
    uint16_t dsize;             /* packet payload size */

    //vvv-----------------------------
    uint16_t sp;                /* source port (TCP/UDP) */
    uint16_t dp;                /* dest port (TCP/UDP) */

    int direction;

    //vvv-----------------------------
    const EtherHdr *eh;         /* standard TCP/IP/Ethernet/ARP headers */
    //const PPPoEHdr *pppoeh;     /* Encapsulated PPP of Ether header */
    const IPHdr *iph;
    //const UDPHdr *udph;
    const TCPHdr *tcph;
    DAQ_PktHdr_t *pkth;
    const uint8_t *data;        /* packet payload pointer */
    const uint8_t *pkt;         /* raw packet data */
} SimpPacket;
#define Packet SimpPacket

//char* ParseMac(const u_char* packet, char* mac);



#ifdef __cplusplus
}
#endif
#endif /* _PACKET_H_ */
