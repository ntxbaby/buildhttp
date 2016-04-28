//------------------------------------------------------------------------
// Nodes Monitor System is a sniffer to get evidences on Internet.
//
// Copyright (C) 2008 BAIHONG Information Security Techology CO.,
//
//------------------------------------------------------------------------
//
// Module Name      :PacketParser.h
//
//------------------------------------------------------------------------
// Notes:
//      This file declares the class PacketParser which is used to 
//      parse the packets from IP layer.
//------------------------------------------------------------------------
// Change Activities:
// tag  Reason   Ver  Rev Date   Origin      Description
// ---- -------- ---- --- ------ ----------- -----------------------------
// $d0= -------- 1.0  001 081212 wuzhonghua Initial
//
//------------------------------------------------------------------------

#ifndef PACKET_PARSER
#define PACKET_PARSER

#define LIBCAP_INTFACE   100
#define PFRING_INTERFACE 101
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
//#include <pfring.h>

// Enumerate the type of each kinds of packet.
enum PacketType
{
	ETH = 0,
	IP,
	TCP,
	UDP,
	NUL
};

// Define a structure to store the information we need from each layer.
struct PacketInfo
{
	unsigned  char srcMac[6];
	unsigned  char destMac[6];
	unsigned long int   srcIpv4;
	unsigned long int   destIpv4;

	unsigned short int srcPort;
	unsigned short int destPort;
	unsigned short int bodyLen;
	struct iphdr  *ip;
	struct tcphdr *tcp;
	enum PacketType pktType;
	char* body;
	char* packet;
	//struct pfring_pkthdr *pkt;
	unsigned long tv_sec;
	int  Flag;
	
	// The packet body of TCP or UDP
};
extern PacketInfo g_packetinfo;
extern	unsigned long int seq4;
extern	unsigned long int ack_seq4;
//-----------------------------------------------------------------------
// Class Name  : PacketParser
// Interface   : Parse, GetPktInfo
// Description : Parse the network packet from data link layer to 
//               transport layer, and collect the necessary information
//               into a structure "PacketInfo".
//-----------------------------------------------------------------------
class PacketParser
{
	public:
		PacketParser();
		virtual ~PacketParser();
		bool Parse(const char* packet);
		void  PacketFilter();
		//PacketInfo* GetPktInfo(const char *packet, struct pfring_pkthdr *pkt,int type=PFRING_INTERFACE);
	private:
		bool ParseEth();
		bool ParseIp();
		void ParseTcp();
		void ParseUdp();
		void ParsePPPOE();
		void CleanParser();
	private:
   	// The original packet from network.
		char* packet_;
		struct ethhdr *ethHeader;
		struct pppoe_hdr *pppoeHeader;
		struct iphdr* ipHeader_;
		char* ipBody_;
		struct tcphdr* tcpHeader_;
		struct udphdr* udpHeader_;
		PacketInfo* pktInfo_;
		short int bodyLen_;
};

//-----------------------------------------------------------------------
// Func Name   : GetPktInfo
// Description : Return the structure from packet parsing.
// Parameter   : void
// Return      : const PacketInfo*
//-----------------------------------------------------------------------

#endif
