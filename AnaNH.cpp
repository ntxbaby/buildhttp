#include "tkt.h"
#include "HttpParser.h"
#include <string>
#include <iostream>
#include <boost/regex.hpp>
using namespace std;

static void NanHangGetFlightInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("\"arrairport\":\"([^\"]+)\",\"arrairportName\":\"([^\"]+)\".*?\"arrtime\":\"([^\"]+)\".*?\"depairport\":\"([^\"]+)\",\"depairportName\":\"([^\"]+)\".*?\"deptime\":\"([^\"]+)\".*?\"flightdate\":\"([^\"]+)\",\"flightno\":\"([^\"]+)\"");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    while(boost::regex_search(begin, end, what, rule) > 0)
    {
        Flight tmp;
        string date, time1, time2;
        date = what[7].str() + " ";
        time1 = what[3].str();
        time1.insert(2, 1, ':');
        time2 = what[6].str();
        time2.insert(2, 1, ':');
        
        tmp.ArrivalStation.assign(what[2].str());
        tmp.ArrivalCity.assign(what[1].str());
        tmp.ArrivalDate.assign(date+time1);
        tmp.DepartureStation.assign(what[5].str());
        tmp.DepartureCity.assign(what[4].str());
        tmp.DepartureDate.assign(date+time2);
        tmp.Number.assign(what[8].str());
        ti.flights.push_back(tmp);

        begin = what[0].second;
    }
}

static void NanHangGetPassengerInfoInner(TktInfo& ti, string& payload)
{
    boost::regex rule("\"idcard\":\"([^\"]+)\",\"idtype\":\"([^\"]+)\",\"psgname\":\"([^\"]+)\",\"type\"");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    while(boost::regex_search(begin, end, what, rule) > 0)
    {
        Passenger tmp;
		string cardtype;
		cardtype = what[2].str();
		if(cardtype == "NI") cardtype.assign("身份证");
		else if(cardtype == "PP" || cardtype == "p") cardtype.assign("护照");
		else cardtype.assign("护照");

        tmp.CardID.assign(what[1].str());
        tmp.CardType = cardtype;
        tmp.Name.assign(what[3].str());
        ti.passengers.push_back(tmp);

        begin = what[0].second;
    }
}

static void NanHangGetPassengerInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("\"psgname\":\\[(.*?)\\]");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    if(boost::regex_search(begin, end, what, rule) > 0)
    {
        string tmp;
        tmp = what[1].str();
        NanHangGetPassengerInfoInner(ti, tmp);
    }
}

static void NanHangGetContactInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("\"contact\":\"([^\\|]+)\\|\\|(\\d+)\\|?([a-zA-Z0-9\\._-]+@[a-zA-Z0-9\\._-]+)?\"");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    if(boost::regex_search(begin, end, what, rule) > 0)
    {
        ti.ContractEmail.assign(what[3].str());
        ti.ContractPhone.assign(what[2].str());
        ti.ContractName.assign(what[1].str());
    }
}



void AnalyseNanHang(HttpSession* hs)
{
     string payload = hs->getResponseBody();
     TktInfo ti;
	 ti.setIpPortMac(hs);
     NanHangGetFlightInfo(ti, payload);
     NanHangGetContactInfo(ti, payload);     
     NanHangGetPassengerInfo(ti, payload);
     ti.Account = ti.ContractPhone;
     ti.Protocol_Type = PROTOCOL_ID_SOUTHERN;
     ti.InfoStorage();
}


