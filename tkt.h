#ifndef __TKT_H__
#define __TKT_H__

#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <../clue/ProtocolID.h>

struct Passenger
{
	std::string               Name;
	std::string               CardType;
	std::string               CardID;
	std::string               Phone;
};

struct Flight
{
	std::string               DepartureCity;
	std::string               DepartureStation;
	std::string               DepartureDate;
	std::string               ArrivalCity;
	std::string               ArrivalStation;
	std::string               ArrivalDate;
	std::string               Number;
};

class HttpSession;

class TktInfo
{
public:
	std::vector<Passenger>    passengers;
	std::vector<Flight>       flights;
	std::string               ContractName, ContractPhone, ContractEmail, Account;
	std::string              clientIp,serverIp;
	std::string            clientPort,serverPort;
	std::string             clientMac;
	int						  Protocol_Type;
        unsigned long   lHash;
        void setIpPortMac(HttpSession* ts);
	void StoreTktInfo();
	void InfoStorage();
        void WriteXML();
};

 void test(TktInfo& ti);
#endif
