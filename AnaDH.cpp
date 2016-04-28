#include "tkt.h"
#include "HttpParser.h"
#include <string>
#include <iostream>
#include <boost/regex.hpp>
using namespace std;

static void DongHangGetFlightInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("\"arriveAirportName\":\"([^\"]*?)\".*?\"arriveCityName\":\"([^\"]*?)\".*?\"arriveDate\":\"([^\"]*?)\".*?\"departAirportName\":\"([^\"]*?)\".*?\"departCityName\":\"([^\"    ]*?)\".*?\"departDate\":\"([^\"]*?)\".*?\"marketFltNo\":\"([^\"]*?)\"");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    while(boost::regex_search(begin, end, what, rule) > 0)
    {
        Flight tmp;
        tmp.ArrivalStation.assign(what[1].str());
        tmp.ArrivalCity.assign(what[2].str());
        tmp.ArrivalDate.assign(what[3].str());
        tmp.DepartureStation.assign(what[4].str());
        tmp.DepartureCity.assign(what[5].str());
        tmp.DepartureDate.assign(what[6].str());
        tmp.Number.assign(what[7].str());
        ti.flights.push_back(tmp);

        begin = what[0].second;
    }
}

static void DongHangGetPassengerInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("\"idNo\":\"([^\"]*?)\".*?\"idType\":\"([^\"]*?)\".*?\"mobile\":\"(\\d*?)\".*?\"paxName\":\"([^\"]*?)\"");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    while(boost::regex_search(begin, end, what, rule) > 0)
    {
        Passenger tmp;
        tmp.CardID.assign(what[1].str());
        tmp.CardType.assign(what[2].str());
        tmp.Phone.assign(what[3].str());
        tmp.Name.assign(what[4].str());
        ti.passengers.push_back(tmp);

        begin = what[0].second;
    }
}

static void DongHangGetContactInfo(TktInfo& ti, string& payload)
{
    boost::regex rule("\"contactEmail\":\"([^\"]*?)\",\"contactMobile\":\"(\\d*?)\".*?\"contactName\":\"([^\"]*?)\"");
    boost::cmatch what;
    const char *begin = payload.c_str();
    const char *end = payload.c_str() + payload.size();
    if(boost::regex_search(begin, end, what, rule) > 0)
    {
        ti.ContractEmail.assign(what[1].str());
        ti.ContractPhone.assign(what[2].str());
        ti.ContractName.assign(what[3].str());
    }
}


void AnalyseDongHang(HttpSession* hs)
{
     string payload = hs->getResponseBody();
     TktInfo ti;
	 ti.setIpPortMac(hs);
     DongHangGetFlightInfo(ti, payload);
     DongHangGetPassengerInfo(ti, payload);
     DongHangGetContactInfo(ti, payload);
     ti.Account = ti.ContractPhone;
     ti.Protocol_Type = PROTOCOL_ID_EASTERN;
     ti.InfoStorage();
}


