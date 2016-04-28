/*************************************************************************
	> File Name: TcpSession.h
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Tue 05 Apr 2016 05:35:40 PM CST
 ************************************************************************/
#include <string.h>
#include <string>
#include "packet.h"

using namespace std;

struct TcpSession
{
    enum DIRECT{UP, DOWN, DIFFERNT};
    TcpSession( unsigned long int sa,
        unsigned long int da,
        unsigned short sp,
        unsigned short dp):
            saddr(sa),daddr(da),sport(sp),dport(dp){}
    TcpSession(const Packet& p)
        :saddr(p.iph->ip_src.s_addr)
        ,daddr(p.iph->ip_dst.s_addr)
        ,sport(ntohs(p.tcph->th_sport))
        ,dport(ntohs(p.tcph->th_dport))
    {
        if(p.eh != NULL)
        memcpy(smac, p.eh->ether_src, 6);
    }
    
    DIRECT isSameTcpSession(const TcpSession& rhs)
    {
        if(saddr == rhs.saddr 
            && daddr == rhs.daddr 
            && sport == rhs.sport
            && dport == rhs.dport)
            return UP;
        else if(saddr == rhs.daddr 
            && daddr == rhs.saddr 
            && sport == rhs.dport
            && dport == rhs.sport)
            return DOWN;
        else
            return DIFFERNT;
    }
      DIRECT isSameTcpSession(const Packet& p)
    {
         if(saddr == p.iph->ip_src.s_addr 
            && daddr == p.iph->ip_dst.s_addr 
            && sport == ntohs(p.tcph->th_sport)
            && dport == ntohs(p.tcph->th_dport))
            return UP;
        else if(saddr == p.iph->ip_dst.s_addr 
            && daddr == p.iph->ip_src.s_addr 
            && sport == ntohs(p.tcph->th_dport)
            && dport == ntohs(p.tcph->th_sport))
            return DOWN;
        else
            return DIFFERNT;
    }
    string getIp(int src = 1)
    {
        struct in_addr addr;
        addr.s_addr = src == 1 ? saddr : daddr;
        return inet_ntoa(addr);
    }
    string getMac()
    {
        char sMac[32] = {0};
        sprintf(sMac, "%02x-%02x-%02x-%02x-%02x-%02x", 
                smac[0], smac[1], smac[2], smac[3], smac[4], smac[5]);
        return sMac;
    }
    string getPort(int src = 1)
    {
        char sPort[16] = {0};
        sprintf(sPort, "%d", src == 1 ? sport : dport);
        return sPort;
    }
    
    unsigned long MkHashIndex()
    {
        unsigned long res = 0;
        res = (sport << 7)^(sport << 8)^(dport << 6)^(dport << 9)^(saddr >> 1)^(daddr >> 1)^(saddr >> 2)^(daddr << 1);
        res = (res>>1);
        char str[32] = {0};
        snprintf(str,sizeof(str),"%ld",res);
        str[0] = (str[0] == '-') ? ' ' : str[0];
        res = atol(str);
        return res;
    }
    
public:
    unsigned long int saddr;
    unsigned long int daddr;
    unsigned short sport;
    unsigned short dport;
    
    unsigned char smac[6];
    
};

