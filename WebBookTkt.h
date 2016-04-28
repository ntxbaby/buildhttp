//------------------------------------------------------------------------
// Nodes Monitor System is a sniffer to get evidences on Internet.      
//                                                                       
// Copyright (C) 2010 BaiHong Information Security Techology CO., Ltd.
// This program belongs to BaiHong ISTC, which shall not be reproduced,   
// copied, or used in other ways without permission. Otherwise BaiHong    
// ISTC will have the right to pursue legal responsibilities.            
//
//------------------------------------------------------------------------
//
// Module Name      :WebTrade.h
//
//------------------------------------------------------------------------
// Notes:
//      The file define the interface of WEBTRADE processor.
//------------------------------------------------------------------------
// Change Activities:
// tag  Reason   Ver  Rev Date   Origin      Description
// ---- -------- ---- --- ------ ----------- -----------------------------
// 
//
//------------------------------------------------------------------------

#ifndef WEBBOOKTKT
#define WEBBOOKTKT
#include "packet.h"


class WebBookTkt
{
public:
	bool IsWebBookTkt(PacketInfo* pktInfo/*const char* packet*/);

};




#endif

