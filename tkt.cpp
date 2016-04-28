/*************************************************************************
	> File Name: tkt.cpp
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Wed 09 Dec 2015 01:46:27 PM CST
 ************************************************************************/

#include "tkt.h"
#include<iostream>
#include <cstring>
#include <iconv.h>
#include <zlib.h>
#include <cstdlib>
#include <cstdio>
#include <list>
#include <sys/stat.h>
#include <sys/types.h>
#include "../clue/Clue.h"
#include "../oracle.h"
#include <arpa/inet.h>
#include "../SqlHelper.h"
#include "HttpParser.h"
#include <boost/format.hpp>
#include "../global.h"

using namespace std;

int HashTime(char *path,unsigned long lHash)
{
	time_t timeval;
        int dirnum;

        time(&timeval);
        dirnum = (timeval / 300) % 12;
        if (nodeDataPath == 0)
        {
                sprintf(path, "/home/nodeData/%d/ticket_%lu%lu.xml", dirnum, timeval,lHash);
        }
        else
        {
                sprintf(path, "%s/nodeData/%d/ticket_%lu%lu.xml", nodeDataPath, dirnum, timeval,lHash);
        }
	return 0;
}

void TktInfo::setIpPortMac(HttpSession* hs)
{
    TcpSession& ts = hs->getTcpSession();
    clientIp = ts.getIp(1);
    serverIp = ts.getIp(0);
    clientPort = ts.getPort(1);
    serverPort = ts.getPort(0);
    clientMac = ts.getMac();
    lHash = ts.MkHashIndex();
}

void TktInfo::StoreTktInfo()
{
#if 0
#ifdef MYSQL
	char sql[] = "insert into booking(objectid,clientip,clientport,clientmac,serverip,"
			"serverport,capturetime,account,psgname,psgphone,psgnotype,certificateno,contactname,contactphone,"
			"contactmail,content,type,mtype"
			")values(?,?,?,?,?,?,now(),?,?,?,?,?,?,?,?,?,?,?)";
#else
	char sql[] = "insert into booking(bookingid,objectid,clientip,clientport,clientmac,serverip,"
			"serverport,capturetime,account,psgname,psgphone,psgnotype,certificateno,contactname,contactphone,"
			"contactmail,content,type,mtype"
			")values(bookingid.nextval,?,?,?,?,?,?,sysdate,?,?,?,?,?,?,?,?,?,?,?)";
#endif
	char s_sql[1024] = {0};
#ifdef VPDNLZ
#ifdef MYSQL
	char sql_vpdnlz[] = "insert into booking(objectid,clientip,clientport,clientmac,serverip,"
			"serverport,capturetime,account,psgname,psgphone,psgnotype,certificateno,contactname,contactphone,"
			"contactmail,content,type,mtype,pppoe"
			")values(?,?,?,?,?,?,now(),?,?,?,?,?,?,?,?,?,?,?,?)";
#else
	char sql_vpdnlz[] = "insert into booking(bookingid,objectid,clientip,clientport,clientmac,serverip,"
			"serverport,capturetime,account,psgname,psgphone,psgnotype,certificateno,contactname,contactphone,"
			"contactmail,content,type,mtype,pppoe"
			")values(bookingid.nextval,?,?,?,?,?,?,sysdate,?,?,?,?,?,?,?,?,?,?,?,?)";
#endif
	snprintf(s_sql, 1024, "%s", sql_vpdnlz);
#else
	snprintf(s_sql, 1024, "%s", sql);
#endif
	PreparedStatement_T p;

	std::string buf;
	std::vector<Flight>::iterator it = flights.begin();
	while(it != flights.end())
	{
		std::string strTmp = "departurecity---" + it->DepartureCity + 
                                    "-|-departurestation---" + it->DepartureStation +
                                    "-|-departuretime---" + it->DepartureDate + 
                                    "-|-arrivalcity---" + it->ArrivalCity +
                                    "-|-arrivalstation---" + it->ArrivalStation + 
                                    "-|-arrivaltime---" + it->ArrivalDate +
                                    "-|- flightnumber---" + it->Number + 
                                    " ###";
		buf += strTmp;
		it++;
	}

	int objectid = 0;
	char pppoe[60] = {0};
	
#ifdef VPDNLZ
	objectid = ClueGetObjectIdByPPPoE(clientip, pppoe);
	if(objectid == 0)
	{
		return;
	}
#else
		objectid = ClueGetObjectIdByMac(Mac_Str);
#endif
	std::vector<Passenger>::iterator itp = passengers.begin();
	while(itp != passengers.end())
	{
		Connection_T con = DbConnect();
		TRY
		{
			p = Connection_prepareStatement(con, sql); 
			PreparedStatement_setInt(p, 1, objectid);  
			PreparedStatement_setString(p, 2, clientIp.c_str());
			PreparedStatement_setString(p, 3, clientPort.c_str());
			PreparedStatement_setString(p, 4, clientMac.c_str());
			PreparedStatement_setString(p, 5, serverIp.c_str());
			PreparedStatement_setString(p, 6, serverPort.c_str());

			PreparedStatement_setString(p, 7, Account.c_str());
			PreparedStatement_setString(p, 8, itp->Name.c_str());
			PreparedStatement_setString(p, 9, itp->Phone.c_str());
			PreparedStatement_setString(p, 10, itp->CardType.c_str());
			PreparedStatement_setString(p, 11, itp->CardID.c_str());
			PreparedStatement_setString(p, 12, ContractName.c_str());
			PreparedStatement_setString(p, 13, ContractPhone.c_str());
			PreparedStatement_setString(p, 14, ContractEmail.c_str());
			PreparedStatement_setString(p, 15, buf.c_str());
			PreparedStatement_setInt(p, 16, Protocol_Type);
			PreparedStatement_setInt(p, 17, 0);
#ifdef VPDNLZ
			PreparedStatement_setString(p, 18, pppoe);
#endif
			PreparedStatement_execute(p);
#ifndef MYSQL
			p = Connection_prepareStatement(con, "commit;");
			PreparedStatement_execute(p);
#endif
		}
		CATCH(SQLException)
		{
			cout<<"SQLException --"<<Exception_frame.message<<endl;
		}
		FINALLY
		{      
		}
		END_TRY;
		itp++;
	}
#endif
	printf("[BOOKING]write db over!\n");
}



void TktInfo::InfoStorage()
{
    WriteXML();
    char sql[] = "insert into booking(bookingid,devicenum,clueid,clientip,clientport,clientmac,serverip,"
    "serverport,capturetime,account,psgname,psgphone,psgnotype,certificateno,contactname,contactphone,"
    "contactmail,content,protocolid,type,mtype"
    ")values(bookingid.nextval,?,?,?,?,?,?,?,sysdate,?,?,?,?,?,?,?,?,?,?,?,?)";
    
    PreparedStatement_T p;

    string buf;
    std::vector<Flight>::iterator it = flights.begin();
    while(it != flights.end())
    {
        std::string tmp = "departurecity---" + it->DepartureCity + 
        "-|-departurestation---" + it->DepartureStation + 
        "-|-departuretime---" + it->DepartureDate +
        "-|-arrivalcity---" + it->ArrivalCity + 
        "-|-arrivalstation---" + it->ArrivalStation + 
        "-|-arrivaltime---" + it->ArrivalDate + 
        "-|- flightnumber---" + it->Number + " ###";
        buf += tmp;
        it++;
    }

	int clueid = GetClueId(PROTOCOL_BOOKING, clientMac.c_str(), inet_addr(clientIp.c_str()), ContractName.c_str());
        if(clueid == 0)
            return ;
    std::vector<Passenger>::iterator itp = passengers.begin();
    while(itp != passengers.end())
    {
        Connection_T con = g_database.Connect();
        TRY
        {
            p = Connection_prepareStatement(con, sql); 
            PreparedStatement_setInt(p, 1, devicenum);//
            PreparedStatement_setInt(p, 2, clueid);//
            PreparedStatement_setString(p, 3, clientIp.c_str());
            PreparedStatement_setString(p, 4, clientPort.c_str());
            PreparedStatement_setString(p, 5, clientMac.c_str());
            PreparedStatement_setString(p, 6, serverIp.c_str());
            PreparedStatement_setString(p, 7, serverPort.c_str());
            PreparedStatement_setString(p, 8, Account.c_str());
            PreparedStatement_setString(p, 9, itp->Name.c_str());
            PreparedStatement_setString(p, 10, itp->Phone.c_str());
            PreparedStatement_setString(p, 11, itp->CardType.c_str());
            PreparedStatement_setString(p, 12, itp->CardID.c_str());
            PreparedStatement_setString(p, 13, ContractName.c_str());
            PreparedStatement_setString(p, 14, ContractPhone.c_str());
            PreparedStatement_setString(p, 15, ContractEmail.c_str());
            PreparedStatement_setString(p, 16, buf.c_str());
            PreparedStatement_setInt(p, 17, 19);
            PreparedStatement_setInt(p, 18, Protocol_Type);
            PreparedStatement_setInt(p, 19,0);

            PreparedStatement_execute(p);
            p = Connection_prepareStatement(con, "commit;");
            PreparedStatement_execute(p);
        }
        CATCH(SQLException)
        {
            cout<<"SQLException --"<<Exception_frame.message<<endl;
        }
        FINALLY
        {      
        }
        END_TRY;
		itp++;
    }
    printf("[BOOKING]write db over!\n");
    
}
void TktInfo::WriteXML() {
    string buf;
    std::vector<Flight>::iterator it = flights.begin();
    while (it != flights.end()) {
        std::string tmp = "departurecity---" + it->DepartureCity +
                "-|-departurestation---" + it->DepartureStation +
                "-|-departuretime---" + it->DepartureDate +
                "-|-arrivalcity---" + it->ArrivalCity +
                "-|-arrivalstation---" + it->ArrivalStation +
                "-|-arrivaltime---" + it->ArrivalDate +
                "-|- flightnumber---" + it->Number + " ###";
        buf += tmp;
        it++;
    }

    int clueid = GetClueId(PROTOCOL_BOOKING, clientMac.c_str(), inet_addr(clientIp.c_str()), ContractName.c_str());
        if(clueid == 0)
            return ;
    
        char newpath[255 + 1] = {0};
        HashTime(newpath,lHash);

    string xml(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<table name=\"booking\">\n"
            "  <data>\n"
            "    <devicenum>%d</devicenum>\n"
            "    <clueid>%d</clueid>\n"
            "    <clientip>%s</clientip>\n"
            "    <clientport>%s</clientport>\n"
            "    <clientmac>%s</clientmac>\n"
            "    <serverip>%s</serverip>\n"
            "    <serverport>%s</serverport>\n"
            "    <capturetime>%d</capturetime>\n"
            "    <account>%s</account>\n"
            "    <psgname><![CDATA[%s]]></psgname>\n"
            "    <psgphone>%s</psgphone>\n"
            "    <psgnotype><![CDATA[%s]]></psgnotype>\n"
            "    <certificateno><![CDATA[%s]]></certificateno>\n"
            "    <contactname><![CDATA[%s]]></contactname>\n"
            "    <contactphone>%s</contactphone>\n"
            "    <contactmail><![CDATA[%s]]></contactmail>\n"
            "    <content><![CDATA[%s]]></content>\n"
            "    <protocolid>%d</protocolid>\n"//PROTOCOL_ID_BOOKING
            "    <type>%d</type>\n"
            "    <mtype>0</mtype>\n"
            "  </data>\n"
            "</table>\n");

    std::vector<Passenger>::iterator itp = passengers.begin();
    
    time_t t = time(NULL);
    
              

    while (itp != passengers.end()) {

        boost::format fmt(xml);

        fmt % devicenum
                % clueid
                % clientIp
                % clientPort
                % clientMac
                % serverIp
                % serverPort
                % t
                % Account
                % itp->Name
                % itp->Phone
                % itp->CardType
                % itp->CardID
                % ContractName
                % ContractPhone
                % ContractEmail
                % buf
                % PROTOCOL_ID_BOOKING
                % Protocol_Type;

        ofstream f(newpath);
        f << fmt << endl;
        itp++;
    }

    printf("\n[BOOKING]write xml over! %s\n", newpath);

}
void test(TktInfo& ti)
{
    size_t i;
    for(i=0; i<ti.flights.size(); i++)
    {
        std::cout << i << "[Flight]    DepartureCity:    " << ti.flights[i].DepartureCity << std::endl;
        std::cout << i << "[Flight]    DepartureStation: " << ti.flights[i].DepartureStation << std::endl;
        std::cout << i << "[Flight]    DepartureDate:    " << ti.flights[i].DepartureDate << std::endl;
        std::cout << i << "[Flight]    ArrivalCity:      " << ti.flights[i].ArrivalCity << std::endl;
        std::cout << i << "[Flight]    ArrivalStation:   " << ti.flights[i].ArrivalStation << std::endl;
        std::cout << i << "[Flight]    ArrivalDate:      " << ti.flights[i].ArrivalDate << std::endl;
        std::cout << i << "[Flight]    FlightNumber:     " << ti.flights[i].Number << std::endl;
    }
    for(i=0; i<ti.passengers.size(); i++)
    {
        std::cout << i << "[Passenger] Name:             " << ti.passengers[i].Name << std::endl;
        std::cout << i << "[Passenger] CardType:         " << ti.passengers[i].CardType << std::endl;
        std::cout << i << "[Passenger] CardID:           " << ti.passengers[i].CardID << std::endl;
        std::cout << i << "[Passenger] Phone:            " << ti.passengers[i].Phone << std::endl;
    }
    std::cout << "0[Contract]  ContractName:     " << ti.ContractName << std::endl;
    std::cout << "0[Contract]  ContractPhone:    " << ti.ContractPhone << std::endl;
    std::cout << "0[Contract]  ContractEmail:    " << ti.ContractEmail << std::endl;
	std::cout << "client: " << ti.clientIp << ":" << ti.clientPort << " " << ti.clientMac << std::endl;
	std::cout << "server" << ti.serverIp << ":" << ti.serverPort << std::endl;
}









