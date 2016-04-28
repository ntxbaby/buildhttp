#include "HttpParser.h"
#include "tkt.h"
#include <boost/algorithm/string.hpp>






void Analyse966800(HttpSession* hs) {
    string html(hs->getResponseBody());
    ofstream f("./966800.html");
    f << html << endl;

    TktInfo ti;
    boost::regex r("<div class=\"infor_cont tit\">\r\n([\\s\\S]*?)</div>");
    
    
    boost::smatch what;
    if (boost::regex_search(html, what, r)) 
    {
        
        string fi(what[1].str());
        boost::trim(fi);
        
        string citys = strMid(fi, " ", "\r\n");
        cout << "citys=" << citys << endl;
        const char sep[] = {0xe2, 0x80, 0x94, 0};
        size_t seppos = citys.find(sep);
        cout << "seppos=" << seppos << endl;
        Flight flight;
        
        flight.DepartureCity = flight.DepartureStation = citys.substr(0, seppos);
        flight.ArrivalCity = flight.ArrivalStation = citys.substr(seppos+3);
        
        boost::regex rDate("\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}");
    
    
        boost::smatch date;
        if (boost::regex_search(html, date, rDate)) 
        {
            flight.DepartureDate = flight.ArrivalDate = date[0].str();
        }
        ti.flights.push_back(flight);
    }
    else
    {
        return ;
    }
    
    Passenger psg;
    
    const char buyer[] = {0xe5,0x8f,0x96,0xe7,0xa5,0xa8,0xe4,0xba,0xba,0xef,0xbc,0x9a,0x00};
    const char identity[] = {0xe5,0x8f,0x96,0xe7,0xa5,0xa8,0xe4,0xba,0xba,0xe8,0xba,0xab,0xe4,0xbb,0xbd,0xe8,0xaf,0x81,0xe5,0x8f,0xb7,0xef,0xbc,0x9a,0x00};
    const char phone[] = {0xe6,0x89,0x8b,0xe6,0x9c,0xba,0xe5,0x8f,0xb7,0xe7,0xa0,0x81,0xef,0xbc,0x9a,0x00};
    
    const char* s = "</span>([^<]*?)</p>";
    string sbuyer(buyer);
    sbuyer+=s;
    string sidentity(identity);
    sidentity+=s;
    string sphone(phone);
    sphone+=s;
    
    boost::regex rbuyer(sbuyer);
    //boost::smatch what;
    if (boost::regex_search(html, what, rbuyer)) {
        
        string fi(what[1].str());
        psg.Name =  fi;
        cout << "buyer=" << fi << endl;
        ti.ContractName = fi;
    }
    
    boost::regex ridentity(sidentity);
    //boost::smatch what;
    if (boost::regex_search(html, what, ridentity)) {
        
        string fi(what[1].str());
        cout << "identity=" << fi << endl;
        psg.CardType = "身份证";
        psg.CardID = fi;
    }
    
    boost::regex rphone(sphone);
    //boost::smatch what;
    if (boost::regex_search(html, what, rphone)) {
        
        string fi(what[1].str());
        cout << "phone=" << fi << endl;
        psg.Phone = fi;
        ti.ContractPhone = fi;
    }
    
    ti.passengers.push_back(psg);
    
    ti.Account = ti.ContractPhone;
    ti.Protocol_Type = PROTOCOL_ID_XIEC;
    ti.setIpPortMac(hs);
    ti.InfoStorage();
        
        
    

}

