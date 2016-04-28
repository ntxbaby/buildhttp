/* 
 * File:   HttpGet.h
 * Author: Administrator
 *
 * Created on 2015年10月1日, 上午10:54
 */

#ifndef HTTPGET_H
#define	HTTPGET_H

#include <map>
#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "assert.h"
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "TcpSession.h"
#include "PacketParser.h"
#include "httpfilter.h"
using namespace std;
#define CRLF "\r\n"
#define HTMLDIR "/home/LzData/moduleData/httpGet/sepCtrl"


bool isGetPacket(const Packet& p);  
bool isOkPacket(const Packet& p);
string strMid(const string& src, const string& begin, const string& end);
bool uncompressBody(string& body);
int log(int f, const char* fmt, ... );
int IsTextUTF8(const char* str, long length); 
string conv(const string& from, const string& to, const string& src);
string urlDecode(const string& src);
char* urlDecodec(char* src);
#define HTTP_START                  0x00000001
#define HTTP_REQUEST_HEAD           0x00000002
#define HTTP_REQUEST_BODY           0x00000004
#define HTTP_REQUEST_OK             0x00000008
#define HTTP_RESPONSE_HEAD          0x00000010
#define HTTP_RESPONSE_BODY          0x00000012

#define RESPONSE_HEAD_CHUNKED   0x00010000
#define RESPONSE_HEAD_GZIP      0x00020000


#define HTTP_RESPONSE_FINISH        0
#define HTTP_RESPONSE_BUILDING      1
#define HTTP_RESPONSE_NOT200        2
#define HTTP_RESPONSE_200        4
#define HTTP_RESPONSE_NOTHTML       3


class DisorderData
{
    unsigned int seq;
    string data;
public:
    DisorderData():seq(0){}
    
    DisorderData(const Packet& p)
    :seq(ntohl(p.tcph->th_seq))
    ,data(p.data, p.data+p.dsize)
    {}
    
    size_t getDataSize()
    {
        return data.size();
    }
    string getData()
    {
        return data;
    }
    unsigned int getSeq()
    {
        return seq;
    }
};
  
class HttpMsg
{
public:
    typedef std::map<uint32_t, DisorderData> PacketMap;
    typedef std::map<uint32_t, DisorderData>::iterator PacketMapITER;
    HttpMsg():_seq(0),_startSeq(0){}
    HttpMsg(uint32_t seq, uint32_t startSeq):_seq(seq),_startSeq(startSeq){}
    virtual ~HttpMsg(){}
    virtual bool buildHead(const Packet& p, uint32_t& state){return false;}
    virtual bool buildBody(const Packet& p, uint32_t& state){return false;}
    virtual string getHead(){ return _head;}
    virtual string getBody(){ return "";}
    void pushIntoMap(const Packet& p);
     int disOrderMapProc(DisorderData& cur, string& nextData); 
     size_t isHeadOk(const Packet& p);
    size_t isHeadOk(const string& data);
    
  
protected:
    uint32_t _seq, _startSeq;
    PacketMap _disorder;
    string _head;
};



class HttpGet_: public HttpMsg
{
public:
    HttpGet_(){}
    HttpGet_(uint32_t seq, uint32_t startSeq):HttpMsg(seq, startSeq){}
    virtual bool buildHead(const Packet& p, uint32_t& state);
    virtual bool buildBody(const Packet& p, uint32_t& state); 
};

class HttpMsgWithBody: public HttpMsg
{
public:
    HttpMsgWithBody():_flag(0),_bodyLen(0){}
    HttpMsgWithBody(uint32_t seq, uint32_t startSeq):HttpMsg(seq, startSeq),_flag(0),_bodyLen(0){}
   
    void dealHead();
    uint32_t parseContentLength(const string& src);
    
    bool isChunkBodyOk(const string& packetData);

    bool isBodyOk();
    
    virtual string getBody(){ return _body;}
    
    virtual bool dealBody(uint32_t& state){return false;}
    virtual uint32_t getHeadState(){return 0;}
    virtual uint32_t getBodyState(){return 0;}
    virtual HttpMsg* getResObject(){return NULL;}
    
    //请求行或响应行ok GET、POST、HTTP/1.1 ok
    virtual bool isMsgHeadOk(const Packet& p){return true;}
    virtual bool buildHead(const Packet& p, uint32_t& state);
    virtual bool buildBody(const Packet& p, uint32_t& state);
    
  
protected:
    uint32_t _flag;
    uint32_t _bodyLen;
    string  _body;
    
};

class HttpPost_: public HttpMsgWithBody
{
public:
    HttpPost_(){}
    HttpPost_(uint32_t seq, uint32_t startSeq):HttpMsgWithBody(seq, startSeq){}
    virtual bool dealBody(uint32_t& state)
    {
        state = HTTP_RESPONSE_HEAD;
        return false;
    }
    virtual uint32_t getHeadState(){ return HTTP_REQUEST_HEAD; }
    virtual uint32_t getBodyState(){ return HTTP_REQUEST_BODY;}
    
};

class HttpResponse: public HttpMsgWithBody
{
public:
    HttpResponse(){}
    HttpResponse(uint32_t seq, uint32_t startSeq):HttpMsgWithBody(seq, startSeq){}
    bool isResponsePacket(const Packet& p)
    {
        return strncmp((char*)p.data, "HTTP/1.", strlen("HTTP/1.")) == 0;
    }
    bool isOkPacket(const Packet& p)
    {
        return strncmp((const char*)p.data, "HTTP/1.1 200 OK", strlen("HTTP/1.1 200 OK")) == 0;
    }
    virtual bool dealBody(uint32_t& state);
    virtual uint32_t getHeadState(){ return HTTP_RESPONSE_HEAD; }
    virtual uint32_t getBodyState(){ return HTTP_RESPONSE_BODY;}
    virtual bool isMsgHeadOk(const Packet& p);
};



struct HttpSession
{
public:
    HttpSession(const Packet& p, HttpCallBack cb, time_t t,int nsec = 5)
        :tcpSession(p)
        ,_req(NULL)
        ,_res(NULL)
        ,_cb(cb)
        ,start(t)
        ,life(nsec)
    {}
    ~HttpSession()
    {
        if(_req != NULL)
            delete _req;
        if(_res != NULL)
            delete _res;
    }
    bool operator==(const HttpSession& rhs)
    {
        return tcpSession.isSameTcpSession(rhs.tcpSession) != TcpSession::DIFFERNT;
    }
    bool isSameSession(const Packet& p)
    {
        return tcpSession.isSameTcpSession(p) != TcpSession::DIFFERNT;
    }
    
    void initNewSession(const Packet& p, uint32_t http);
    bool dealPacket(const Packet& p);
    string getRequestBody()
    {
        return _req->getBody();
    }
    string getResponseBody()
    {
        return _res->getBody();
    }
    string getRequestHead()
    {
        return _req->getHead();
    }
    string getResponseHead()
    {
        return _res->getHead();
    }
    
     TcpSession& getTcpSession()
    {
        return tcpSession;
    }
     HttpCallBack callback()
     {
         return _cb;
     }
     
     bool isTimeOut(time_t now)
     {
         return now - start > life;
     }
    
private:
    TcpSession tcpSession;
    HttpMsg* _req, *_res;
    uint32_t _state;
    HttpCallBack _cb;
    time_t start;
    int life;
};



class FindHttpSession
{
public:
    FindHttpSession(const Packet& p):packet(p){}
    bool operator() (HttpSession* hs)
    {
        return hs->isSameSession(packet);
    }
private:
    Packet packet;
};





class HttpParser
{
public:
    HttpParser(Filter* filter0):filter(filter0){}
    int isHttpPacket(const Packet& p);
    bool parse(const Packet& p);
    bool parse(PacketInfo* p);
    
private:
    typedef std::list<HttpSession*>::iterator ITER;
    std::list<HttpSession*> httpss;
    Filter* filter;
    time_t curtime;
};


#endif	/* HTTPGET_H */

