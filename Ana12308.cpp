#include "tkt.h"
#include "HttpParser.h"
#include <string>
#include <iostream>
#include <map>
#include <boost/regex.hpp>
using namespace std;

map<string, TktInfo*> tktmap12308;

static char x2c(const char *what)
{
    char digit;
    digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A') + 10 : (what[0] - '0'));
    digit *= 16; 
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A') + 10 : (what[1] - '0'));
    return digit;
}

void UrlDecode(const string& source, string& dest)
{
    if(source.size()==0)
        return;

    const char *src = source.c_str();
    while(*src != '\0')
    {   
        if(*src == '+')
        {   
            dest.append(" ");
        }   
        else if(*src == '%' && *(src+1) != 'u')
        {   
            char ch; 
            ch = x2c(src+1);
            dest.append(1, ch);
            src+=2;
        }   
        else
        {   
            dest.append(1, *src);
        }   
        src++;
    }   
}

static string GetMidStr(string& payload, const char *start, const char *end)
{
    string tmp;
    int pos = payload.find(start);
    if(pos != string::npos)
    {   
        pos += strlen(start);
        int pos1 = payload.find(end, pos);
        if(pos1 != string::npos)
        {   
            tmp.assign(payload, pos, pos1-pos);
        }   
    }   
    return tmp;
}

void Passenger12308(HttpSession* hs)
{
    TktInfo* ti = new TktInfo;
    string tmpdec, passengerTotal, orderNo;
    string orderinfo = hs->getResponseBody();

    UrlDecode(hs->getRequestBody(), tmpdec);

    orderNo = GetMidStr(orderinfo, "\"orderNo\":\"", "\"");
    ti->ContractName = GetMidStr(tmpdec, "&realName=", "&");
    ti->ContractPhone = GetMidStr(tmpdec, "&mobilePhone=", "&");
    passengerTotal = GetMidStr(tmpdec, "&passengerInfo=", "&");

    boost::regex rule("([^,]+),(\\d+),([^,]+),[^;]+;");
    boost::cmatch what;
    const char *begin = passengerTotal.c_str();
    const char *end = passengerTotal.c_str() + passengerTotal.size();
    while(boost::regex_search(begin, end, what, rule) > 0)
    {   
        Passenger tmp;
        tmp.Name.assign(what[1].str());
        tmp.Phone.assign(what[2].str());
        tmp.CardID.assign(what[3].str());
        tmp.CardType.assign("身份证");

        ti->passengers.push_back(tmp);

        begin = what[0].second;
    }

    if(tktmap12308.find(orderNo)==tktmap12308.end())
        tktmap12308[orderNo] = ti;
}


void CityTime12308(HttpSession* hs)
{
    string orderNo, ReqHeader, ResHeader, ResBody;

    ResHeader = hs->getResponseHead();
    string tmp("HTTP/1.1 200 OK");
    if(ResHeader.compare(0, tmp.size(), tmp) == 0)
    {
        TktInfo* ti = NULL;
        ReqHeader = hs->getRequestHead();
        orderNo = GetMidStr(ReqHeader, "GET /toPay.htm?orderNo=", " HTTP/1.1");
        if(orderNo.size()==0)
            return;

        map<string, TktInfo*>::iterator it = tktmap12308.find(orderNo);
        if(it != tktmap12308.end())
        {
            Flight ftmp;
            string city;

            ti = it->second;
            ResBody = hs->getResponseBody();

            city = GetMidStr(ResBody, "<th>车票信息</th>", "/td>");
            ftmp.DepartureStation = GetMidStr(city, ">", " -- ");
            ftmp.DepartureCity = ftmp.DepartureStation;
            ftmp.ArrivalStation = GetMidStr(city, " -- ", "<");
            ftmp.ArrivalCity = ftmp.ArrivalStation;
            ftmp.DepartureDate = GetMidStr(ResBody, "发车时间:<span>", "</span>");
            ftmp.ArrivalDate = ftmp.DepartureDate;
            ti->flights.push_back(ftmp);
            ti->Account = GetMidStr(ReqHeader, "LN=\"", "::");
            cout << "ti->Account: " << ti->Account << endl;

            ti->setIpPortMac(hs);
            ti->Protocol_Type = PROTOCOL_ID_HAINANAIRLINE;
            //test(*ti);
            ti->InfoStorage();
            delete ti;
            tktmap12308.erase(orderNo);
        }
    }
}
