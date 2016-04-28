#include "tkt.h"
#include "HttpParser.h"
#include <string>
#include <iostream>
#include <boost/regex.hpp>
using namespace std;


static void HaiHangGetFlightInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("<td>([^<]*?)</td><td>([^-]*?)-([^<]*?)</td><td>([^-]*?)-([^<]*?)</td><td>.*?</td><td>\r?\n");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    while(boost::regex_search(begin, end, what, rule) > 0)
    {
        Flight tmp;
        tmp.Number.assign(what[1].str());
        tmp.DepartureStation.assign(what[2].str());
        tmp.ArrivalStation.assign(what[3].str());
        tmp.DepartureDate.assign(what[4].str());
        tmp.ArrivalDate.assign(what[5].str());
        ti.flights.push_back(tmp);

        begin = what[0].second;
    }
}

static void HaiHangGetPassengerInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("<td>([^<]*?)</td><td>([^<]*?)</td><td>([^<]*?)</td><td>([^<]*?)</td><td>([^<]*?)</td><td>([^<]*?)</td><td>.*?</td><td>\r?\n");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    while(boost::regex_search(begin, end, what, rule) > 0)
    {
        Passenger tmp;
        tmp.Name.assign(what[1].str());
        tmp.CardType.assign(what[3].str());
        tmp.CardID.assign(what[4].str());
        ti.passengers.push_back(tmp);

        begin = what[0].second;
    }
}

static void HaiHangGetContactInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("<td>([^<]*?)</td><td>(\\d*?)</td><td>([^<]*?)</td><td>([^<]*?)</td>\r?\n");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    if(boost::regex_search(begin, end, what, rule) > 0)
    {
        ti.ContractName.assign(what[1].str());
        ti.ContractPhone.assign(what[2].str());
        ti.ContractEmail.assign(what[4].str());
    }
}


void AnalyseHaiHang(HttpSession* hs)
{
     string payload = hs->getResponseBody();
     TktInfo ti;
	 ti.setIpPortMac(hs);
     HaiHangGetFlightInfo(ti, payload);
     HaiHangGetPassengerInfo(ti, payload);
     HaiHangGetContactInfo(ti, payload);
	 ti.Account = ti.ContractPhone;
	 ti.Protocol_Type = PROTOCOL_ID_HAINANAIRLINE;
	 ti.InfoStorage();
}


