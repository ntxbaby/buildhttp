#include "HttpParser.h"
#include "tkt.h"
#include <boost/algorithm/string.hpp>






void AnalyseDaBa(HttpSession* hs) {
    string html(hs->getResponseBody());
//    ofstream f("./daba.html");
//    f << html << endl;


    boost::regex r("<tr class=\"border\">([\\s\\S]*?)</tr>");
    boost::sregex_iterator pos(html.begin(), html.end(), r);
    boost::sregex_iterator end;
    TktInfo ti;
    
    int ixTr = 0;
    while (pos != end) {
        
        string f((*pos)[1].str());
        //f = conv("gbk", "utf8", f);
       // cout << "tr=" << f << endl;
        if(ixTr == 0)
        {
             Flight flight;
             boost::regex td("<td align=\"center\">([^<]*?)</td>");
          
                boost::sregex_iterator it(f.begin(), f.end(), td);
                int i = 0;
                while (it != end && i < 2) {
                    if (i == 0)
                    {
                        flight.DepartureDate = flight.ArrivalDate = (*it)[1].str();
                    }
                    if (i == 1)
                    {
                        string stations((*it)[1].str());
                        size_t sep = stations.find("-");
                        if(sep != string::npos)
                        { 
                            flight.DepartureCity = flight.DepartureStation = stations.substr(0, sep);
                            flight.ArrivalCity = flight.ArrivalStation = stations.substr(sep+1);
                        }
                    }
                    ++it;
                    ++i;
                }
              ti.flights.push_back(flight); 
              ++pos;
              ++ixTr;
              continue;
        }
        Passenger psg;
        boost::regex td("<td [^<]*?align=\"center\">([^<]*?)</td>");
          
        boost::sregex_iterator it(f.begin(), f.end(), td);
        int i = 0;
        while (it != end && i < 3) {
            cout << (*it)[1].str() << endl;
            if (i == 0)
            {
                psg.Name =  (*it)[1].str();
            }
            if (i == 1)
            {
                psg.CardType = "身份证";
                psg.CardID = (*it)[1].str();
                boost::trim(psg.CardID);
            }

             if (i == 2)
            {
                 psg.Phone = (*it)[1].str();
                 boost::trim(psg.Phone);
            }
            ++it;
            ++i;
        }
        if(!psg.Name.empty())
            ti.passengers.push_back(psg);
    ++pos;
     ++ixTr;   
    }
    

    // contact info
    boost::regex rContactInfo("<tr  class=\"border detailse\">([\\s\\S]*?)</tr>");
    boost::smatch what;
    if (boost::regex_search(html, what, rContactInfo)) {
        boost::regex dd("<td [^<]*?align=\"center\">([^<]*?)</td>");
        string contactInfo(what[0].str());
        //contactInfo = conv("gbk", "utf8", contactInfo);
        boost::sregex_iterator it(contactInfo.begin(), contactInfo.end(), dd);
        int i = 0;
        while (it != end) {
            cout << (*it)[1].str() << endl;
            if (i == 0) ti.ContractName = (*it)[1].str();
            if (i == 2) ti.ContractPhone = (*it)[1].str();
           // if (i == 2) ti.ContractEmail = (*it)[1].str();
            ++it;
            ++i;
        }
    }
    ti.Account = ti.ContractPhone;
    ti.Protocol_Type = PROTOCOL_ID_XIEC;
    ti.setIpPortMac(hs);
    ti.InfoStorage();

}

