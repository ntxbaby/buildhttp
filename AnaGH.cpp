#include "HttpParser.h"
#include "tkt.h"
#include <boost/algorithm/string.hpp>

TktInfo ti;

void AnalyseGHFlight(HttpSession* hs)
{
    string html(hs->getResponseBody());
//    ofstream f("./gh.html");
//    f << html << endl;
    
    boost::regex rFlightDetailsBlock("<div class=\"flightDetailsBlock\">([\\s\\S]*?)<div class=\"detailsSubBlock detailsSubBlockExpandable detailsSubBlockFareBreakdown\">");
    boost::smatch m;
    if (!boost::regex_search(html, m, rFlightDetailsBlock)) 
    {
        return ;
    }
    string flightDetailsBlock = m[1].str();
   // cout << "flightDetailsBlock=" << flightDetailsBlock << endl;
    
    boost::regex rFlight("<tbody>([\\s\\S]*?)</tbody>");
    boost::sregex_iterator pos(flightDetailsBlock.begin(), flightDetailsBlock.end(), rFlight);
    boost::sregex_iterator end;
    //flight info
    while (pos != end) 
    {
        string html((*pos)[1].str()); 
       // cout << "tbody=" << html << endl;
        Flight flight;

        boost::regex rSrc("<td class=\"colDeparture\">([\\s\\S]*?)</td>");
        boost::smatch what;
        if (boost::regex_search(html, what, rSrc)) 
        {
            flight.DepartureCity = flight.DepartureStation = strMid(what[1].str(), "<span class=\"orig\">", "</span>");

            string day = strMid(what[1].str(), "<span class=\"date\">", "</span>");
            string time = strMid(what[1].str(), "<span class=\"time\">", "</span>");
            flight.DepartureDate = day + " " + time;
        }

        boost::regex rDst("<td class=\"colArrival\">([\\s\\S]*?)</td>");
        if (boost::regex_search(html, what, rDst)) 
        {
            flight.ArrivalCity = flight.ArrivalStation = strMid(what[1].str(), "<span class=\"dest\">", "</span>");
            string day = strMid(what[1].str(), "<span class=\"date\">", "</span>");
            string time = strMid(what[1].str(), "<span class=\"time\">", "</span>");
            flight.ArrivalDate = day + " " + time;
        }

        boost::regex rFlight("<td class=\"colFlight\"  >([\\s\\S]*?)<a href=([^>]*?)>(\\w*?)</a>");
        if (boost::regex_search(html, what, rFlight)) 
        {
            flight.Number = what[3].str();
            cout << "flight Num=" << flight.Number << endl;
        }

        ti.flights.push_back(flight);
        ++pos;
     }
}


void AnalyseGHPsg(HttpSession* hs) 
{
    string html(hs->getRequestBody());
    if(html.find("validateAction=UpdateReservation") != string::npos)
    {
        cout << html << endl;
        string count = strMid(html, "travellersCount=", "&");
        int c = atoi(count.c_str());
        const char* szLead = "travellersInfo%%5B%d%%5D.%s=";
        char flag[64] = {0};
        for(int i = 0; i < c; ++i)
        {
            sprintf(flag, szLead, i, "lastName");
            string lastName = strMid(html, flag, "&");
            
            sprintf(flag, szLead, i, "firstName");
            string firstName = strMid(html, flag, "&");
            
            sprintf(flag, szLead, i, "advancedPassengerDetails(foid)");
            string foid = strMid(html, flag, "&");
            
            sprintf(flag, szLead, i, "advancedPassengerDetails(foidNumber)");
            string foidNumber = strMid(html, flag, "&");
                    
            sprintf(flag, szLead, i, "personalPhone.phoneNumber");
            string phoneNumber = strMid(html, flag, "&");
            
             Passenger psg;
             psg.Name = urlDecode(lastName + firstName);
             
             //const char* shenfenzheng = "\0xe8\0xba\0xab\0xe4\0xbb\0xbd\0xe8\0xaf\0x81";
             psg.CardType = foid;
             psg.CardID = foidNumber;
             psg.Phone = phoneNumber;
             ti.passengers.push_back(psg);
        }
        string contactName = strMid(html, "travelArranger.lastName=", "&") + strMid(html, "travelArranger.firstName=", "&");
        
        ti.ContractName = urlDecode(contactName);
        if(ti.passengers.size() > 0)
        {
            ti.ContractPhone = ti.passengers[0].Phone;
            sprintf(flag, szLead, 0, "emailAddress");
            string email = strMid(html, flag, "&");
            ti.ContractEmail = urlDecode(email);
        }
        
        ti.Protocol_Type = PROTOCOL_ID_XIEC;
        ti.setIpPortMac(hs);
        ti.InfoStorage();
        ti.flights.clear();
        ti.passengers.clear();
    }

}

