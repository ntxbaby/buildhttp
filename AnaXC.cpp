#include "HttpParser.h"
#include "tkt.h"
#include <boost/algorithm/string.hpp>






void AnalyseXieChengGuoji(HttpSession* hs) {
    string html(hs->getResponseBody());
//    ofstream f("./dir/xcguoji.html");
//    f << html << endl;

    boost::regex r("<div class=\'flight_info\'>([\\s\\S]*?)class=\'icon-round-up\'></a>");
    boost::sregex_iterator pos(html.begin(), html.end(), r);
    boost::sregex_iterator end;
    TktInfo ti;
    //flight info
    while (pos != end) {
        string f((*pos)[1].str());
        f = conv("gbk", "utf8", f);
//        cout << f;
        Flight flight;

        flight.DepartureCity = strMid(f, "<span class=\'flight_from\'>", "<i class=\'date\'>");

        string depar = strMid(f, "<div class=\'flight_from\'>", "</div>");
        size_t ix = depar.find("<span class=\'time\'>");
        flight.DepartureStation = strMid(string(depar, ix), "<p>", "</p>");
        string date1 = strMid(string(depar, 0, ix), "<p>", "</p>");
        string date2 = strMid(depar, "<span class=\'time\'>", "</span>");
        flight.DepartureDate = date1 + date2;

        flight.ArrivalCity = strMid(f, "<span class=\'flight_to\'>", "<i class=\'date\'>");
        string addri = strMid(f, "<div class=\'flight_to\'>", "</div>");
        ix = addri.find("<span class=\'time\'>");
        flight.ArrivalStation = strMid(string(addri, ix), "<p>", "</p>");
        date1 = strMid(string(depar, 0, ix), "<p>", "</p>");
        date2 = strMid(depar, "<span class=\'time\'>", "</span>");
        flight.ArrivalDate = date1 + date2;

        flight.Number = strMid(f, "<span class=\'flight_number pubFlights_CZ\'>", "<span class=\'plane_type abbr\'");

        boost::trim(flight.DepartureCity);
        boost::trim(flight.DepartureStation);
        boost::trim(flight.DepartureDate);
        boost::trim(flight.ArrivalCity);
        boost::trim(flight.ArrivalStation);
        boost::trim(flight.ArrivalDate);
        boost::trim(flight.Number);
        cout << "++++++++flight info++++++++" << endl;
        cout << flight.DepartureCity << " "
                << flight.DepartureStation << " "
                << flight.DepartureDate << " "
                << flight.ArrivalCity << " "
                << flight.ArrivalStation << " "
                << flight.ArrivalDate << " "
                << flight.Number << " " << endl;

        ti.flights.push_back(flight);
        ++pos;
    }

    //<dl class="dl_horizontal passneger_info">
    boost::regex rPassnegerInfo("<dl class=\'dl_horizontal passneger_info\'>([\\s\\S]*?)</dl>");
    boost::sregex_iterator itPI(html.begin(), html.end(), rPassnegerInfo);
    while (itPI != end) {
        string f((*itPI)[1].str());
        f = conv("gbk", "utf8", f);

        Passenger pass;

        boost::smatch what;
        boost::regex dd("<dd>(.*?)</dd>");
        boost::sregex_iterator it(f.begin(), f.end(), dd);
        int i = 0;
        while (it != end) {
            if (i == 0) {
                pass.Name = (*it)[1].str();
                pass.Name = strMid(pass.Name, "<span class=\'name\'>", "</span>");

            }
            if (i == 1 || i == 2) {
                string card = (*it)[1].str();
                pass.CardType = strMid(card, "<span style=\'margin-left: 10px;\'>", "</span>");
                pass.CardID = strMid(card, "<b>", "</b>");
            }

            ++it;
            ++i;
        }

        cout << "++++++++passneger info++++++++" << endl;
        cout << pass.Name << " "
                << pass.CardType << " "
                << pass.CardID << " "
                << " " << endl;

        ti.passengers.push_back(pass);
        ++itPI;
    }

    // contact info
    boost::regex rContactInfo("<dl class=\"dl_horizontal\">([\\s\\S]*.?)</dl>");
    boost::smatch what;
    if (boost::regex_search(html, what, rContactInfo)) {
        boost::regex dd("<dd>([^<]*?)</dd>");
        string contactInfo(what[0].str());
        contactInfo = conv("gbk", "utf8", contactInfo);
        boost::sregex_iterator it(contactInfo.begin(), contactInfo.end(), dd);
        int i = 0;
        while (it != end) {
            if (i == 0) ti.ContractName = (*it)[1].str();
            if (i == 1) ti.ContractPhone = (*it)[1].str();
            if (i == 2) ti.ContractEmail = (*it)[1].str();

            ++it;
            ++i;
        }
        cout << "++++++++contact info++++++++" << endl;
        cout << ti.ContractName << " "
                << ti.ContractPhone << " "
                << ti.ContractEmail << " "
                << " " << endl;
    }
    ti.Account = ti.ContractPhone;
    ti.Protocol_Type = PROTOCOL_ID_XIEC;
    ti.setIpPortMac(hs);
    ti.InfoStorage();

}

void AnalyseXieChengGuonei(HttpSession* hs) {
    string html(hs->getResponseBody());
//    ofstream f("./dir/xcguonei.html");
//    f << html << endl;
    boost::regex r("<div class=\"flight-info ([\\s\\S]*?)</table>");
    boost::sregex_iterator pos(html.begin(), html.end(), r);
    boost::sregex_iterator end;
    TktInfo ti;
    //flight info
    while (pos != end) {
        string f((*pos)[1].str());
        f = conv("gbk", "utf8", f);
//        cout << f;
        Flight flight;

        string depar = strMid(f, "<td class=\"depart-city ", "</td>");
        flight.DepartureCity = strMid(depar, "<div class=\"city\">", "</div>");
        flight.DepartureStation = strMid(depar, "<div class=\"airport\">", "</div>");
        flight.DepartureDate = strMid(depar, "<div class=\"time\">", "</div>");

        string addri = strMid(f, "<td class=\"arrive-city ", "</td>");
        flight.ArrivalCity = strMid(addri, "<div class=\"city\">", "</div>");
        flight.ArrivalStation = strMid(addri, "<div class=\"airport\">", "</div>");
        flight.ArrivalDate = strMid(addri, "<div class=\"time\">", "</div>");

        string flight_line = strMid(f, "<td class=\"flight-line ", "</td>");
        string num1 = strMid(flight_line, "<span class=\"pubFlights_cz flight_logo\">", "</span>");
        string num2 = strMid(flight_line, "<strong>", "</strong>");
        flight.Number = num1 + num2;
        boost::trim(flight.DepartureCity);
        boost::trim(flight.DepartureStation);
        boost::trim(flight.DepartureDate);
        boost::trim(flight.ArrivalCity);
        boost::trim(flight.ArrivalStation);
        boost::trim(flight.ArrivalDate);
        boost::trim(flight.Number);
        cout << "++++++++flight info++++++++" << endl;
        cout << flight.DepartureCity << " "
                << flight.DepartureStation << " "
                << flight.DepartureDate << " "
                << flight.ArrivalCity << " "
                << flight.ArrivalStation << " "
                << flight.ArrivalDate << " "
                << flight.Number << " " << endl;

        ti.flights.push_back(flight);
        ++pos;
    }

    //<dl class="dl_horizontal passneger_info">
    boost::regex rPassnegerInfo("<dl class=\"dl_horizontal passneger_info\">([\\s\\S]*?)</dl>");
    boost::sregex_iterator itPI(html.begin(), html.end(), rPassnegerInfo);
    while (itPI != end) {
        string f((*itPI)[1].str());
        f = conv("gbk", "utf8", f);
//        cout << f;

        Passenger pass;

        pass.Name = strMid(f, "<span class=\"name\">", "</span>");
        size_t ixName = pass.Name.find("<i>");
        pass.Name = pass.Name.substr(0, ixName);
        string card = strMid(f, "<span class=\"idcard\">", "</span>");
        size_t ixID = card.find(" ");

        pass.CardType = card.substr(0, ixID);
        pass.CardID = card.substr(ixID);


        cout << "++++++++passneger info++++++++" << endl;
        cout << pass.Name << " "
                << pass.CardType << " "
                << pass.CardID << " "
                << " " << endl;
        ti.passengers.push_back(pass);
        ++itPI;
    }

    // contact info
    boost::regex rContactInfo("<dl class=\"dl_horizontal\">([\\s\\S]*.?)</dl>");
    boost::smatch what;
    if (boost::regex_search(html, what, rContactInfo)) {
        boost::regex dd("<dd>([^<]*?)</dd>");
        std::string contactInfo(what[0].str());
        contactInfo = conv("gbk", "utf8", contactInfo);
        boost::sregex_iterator it(contactInfo.begin(), contactInfo.end(), dd);
        int i = 0;
        while (it != end) {
            if (i == 0) ti.ContractName = (*it)[1].str();
            if (i == 1) ti.ContractPhone = (*it)[1].str();
            if (i == 2) ti.ContractEmail = (*it)[1].str();

            ++it;
            ++i;
        }
        cout << "++++++++contact info++++++++" << endl;
        cout << ti.ContractName << " "
                << ti.ContractPhone << " "
                << ti.ContractEmail << " "
                << " " << endl;
    }
    ti.Account = ti.ContractPhone;
    ti.Protocol_Type = PROTOCOL_ID_XIEC;
    ti.setIpPortMac(hs);
    ti.InfoStorage();

}




void AnalyseXieChengBus(HttpSession* hs) 
{
    string req(hs->getRequestBody());
    req = urlDecode(req);
    TktInfo ti;
    Flight flight;

    flight.DepartureCity = strMid(req, "fromCityName=", "&") ;
    flight.ArrivalCity = strMid(req, "toCityName=", "&") ;
    flight.DepartureStation =  strMid(req, "fromStationName=", "&") ;
    flight.DepartureDate = strMid(req, "ticketDate=", "&") + " " + strMid(req, "ticketTime=", "&");
    
    
    ti.flights.push_back(flight);
    
    string passInfo( strMid(req, "passenger_info=", "&") );
    
    vector<string> pi;
    boost::split( pi, passInfo, boost::is_any_of("|") );
    for(vector<string>::iterator it = pi.begin(); it != pi.end(); ++it)
    {
        if(it->empty()) continue;
        vector<string> fi;
        boost::split( fi, *it, boost::is_any_of(",") );
         vector<string>::iterator itf = fi.begin();
         Passenger pass;
        for(int i=0; itf != fi.end(); ++itf,++i)
        {


            
            if(i==0) pass.Name = *itf;
            if(i==1) pass.CardType = *itf;
            if(i==2) {pass.CardID = *itf;  break;}

            
        }
         ti.passengers.push_back(pass);
       
    }
    
    ti.ContractName =  strMid(req, "contactName=", "&") ;
    ti.ContractPhone = strMid(req, "contactMobile=", "&") ;
    ti.Account = ti.ContractPhone;
    
    ti.Protocol_Type = PROTOCOL_ID_XIEC;
    ti.setIpPortMac(hs);
    ti.InfoStorage();
}


void AnalyseXieChengTrain(HttpSession* hs)
{
    string req(hs->getRequestBody());
    req = urlDecode(req);
    TktInfo ti;
    
    
    
    string param( strMid(req, "requestParameter", "&"));
    cout << param << endl; 
    
    Flight w;
    w.DepartureCity = strMid(param, "\"DepartureCityName\":\"", "\",") ;
    w.ArrivalCity = strMid(param, "\"ArrivalCityName\":\"", "\",") ;
    w.DepartureStation =  strMid(param, "\"DepartureStation\":\"", "\",") ;
    w.ArrivalStation =  strMid(param, "\"ArrivalStation\":\"", "\",") ;
    w.DepartureDate = strMid(param, "\"DepartureDate\":\"", "\",") + " " + strMid(param, "\"DepartureTime\":\"", "\",");
    w.ArrivalDate = strMid(param, "\"ArrivalDate\":\"", "\",")+ " " + strMid(param, "\"ArrivalTime\":\"", "\",");
    w.Number = strMid(param, "\"TrainNumber\":\"", "\",") ;
    if(!w.Number.empty())
    ti.flights.push_back(w);
    
    Flight f;
    f.DepartureCity = w.ArrivalCity ;
    f.ArrivalCity =  w.DepartureCity ;
    f.DepartureStation =  strMid(param, "\"ReturnDepartureStation\":\"", "\",") ;
    f.ArrivalStation =  strMid(param, "\"ReturnArrivalStation\":\"", "\",") ;
    f.DepartureDate = strMid(param, "\"ReturnDepartureDate\":\"", "\",") + " " + strMid(param, "\"ReturnDepartureTime\":\"", "\",");
    f.ArrivalDate = strMid(param, "\"ReturnArrivalDate\":\"", "\",")+ " " + strMid(param, "\"ReturnArrivalTime\":\"", "\",");
    f.Number = strMid(param, "\"ReturnTrainNumber\":\"", "\",") ;
    if(!f.Number.empty())
    ti.flights.push_back(f);
     
    string passName( strMid(req, "passengerName=", "&") );
    string passIDName( strMid(req, "passengerIDCardName=", "&") );
    string passCardID( strMid(req, "passengerCardID=", "&") );
     
    vector<string> pn, pc, pid;
    boost::split( pn, passName, boost::is_any_of(",") );
    boost::split( pc, passIDName, boost::is_any_of(",") );
    boost::split( pid, passCardID, boost::is_any_of(",") );
    int i=0, count = pn.size();
    for( ; i < count; ++i)
    {
        if(pn[i].empty()) continue;
       
        Passenger pass;
        pass.Name = pn[i];
        pass.CardType = pc[i];
        pass.CardID = pid[i];
      
        ti.passengers.push_back(pass);
       
    }
    
    ti.ContractName =  strMid(req, "hidContactName=", "&") ;
    ti.ContractPhone = strMid(req, "hidContactMobile=", "&") ;
    ti.ContractEmail = strMid(req, "hidContactEmail=", "&") ;
    ti.Account = ti.ContractPhone;
    ti.Protocol_Type = PROTOCOL_ID_XIEC;
    if(ti.ContractName.empty()) 
        return;
   
    ti.setIpPortMac(hs);
    ti.InfoStorage();
}