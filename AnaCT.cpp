#include "HttpParser.h"
#include "tkt.h"
#include <boost/algorithm/string.hpp>

TktInfo ct;
string orderid;

void AnalyseCTFlight(HttpSession* hs)
{
    string html(hs->getResponseBody());
    ofstream f("./ct.html");
    f << html << endl;
    
    string req(hs->getRequestHead());
    string id(strMid(req, "GET /pay/onlinePay.htm?orderId=", "&"));
    if(orderid.empty() || id!=orderid)
        return ;
    
    
    Flight flight;
    boost::regex rSrc("<ul class=\"odi_item([\\s\\S]*?)</ul>");
    boost::smatch what;
    if (boost::regex_search(html, what, rSrc)) 
    {
        string fi(what[1].str()); 
        string day = strMid(fi, "<strong>", "</strong>");
        string time = strMid(fi, "<span class=\"fontA hui333 font14 mgl10 mgr5\">", "</span>");
        flight.DepartureDate = flight.ArrivalDate = day + " " + time;
        
        boost::regex rCity("<p class=\"odi_city font18 bold\">([^<]*?)</p>");
        boost::sregex_iterator posCity(fi.begin(), fi.end(), rCity);
        boost::sregex_iterator end;
        int i = 0;
        while (posCity != end) {
            string city((*posCity)[1].str());
            if(i==0) 
            {
                flight.DepartureCity = flight.DepartureStation = city;
            }
            if(i==1) 
            {
                flight.ArrivalCity = flight.ArrivalStation  = city;
            }
            ++posCity;
            ++i;
        }
        
        
        boost::regex rStation("<p class=\"odi_station font14\">([^<]*?)</p>");
        boost::sregex_iterator posStation(fi.begin(), fi.end(), rStation);
        
        i = 0;
        while (posStation != end) {
            string station((*posStation)[1].str());
            if(i==0) 
            {
                flight.DepartureStation = station;
            }
            if(i==1) 
            {
                flight.ArrivalStation  = station;
            }
            ++posStation;
            ++i;
        }
        ct.flights.push_back(flight);
        
        ct.Protocol_Type = PROTOCOL_ID_XIEC;
        ct.setIpPortMac(hs);
        ct.InfoStorage();
        ct.flights.clear();
        ct.passengers.clear();
        
    }
    
}


void AnalyseCTPsg(HttpSession* hs) 
{
    string req(hs->getRequestBody());
    
    string res(hs->getResponseBody());
    if(res.find("orderIds") != string::npos)
    {
        orderid = strMid(res, "\"orderId\":\"", "\"");
        Passenger psg;
        psg.Name = urlDecode( strMid(req, "receUserName=", "&") );
        psg.CardID = strMid(req, "receUserCardCode=", "&");
        psg.Phone = strMid(req, "receUserContact=", "&");
        ct.passengers.push_back(psg);
        
        ct.ContractName = psg.Name;
        ct.ContractPhone = psg.Phone;
        
        
    }

}


