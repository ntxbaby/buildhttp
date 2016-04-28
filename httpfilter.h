/* 
 * File:   httpfilter.h
 * Author: Administrator
 *
 * Created on 2016年4月7日, 下午3:12
 */

#ifndef HTTPFILTER_H
#define	HTTPFILTER_H
#include <string>
#include <list>
#include <iostream>
#include <algorithm>
#include "packet.h"

class HttpSession;
typedef void (*HttpCallBack)(HttpSession* hs);



class Filter
{
public:
    virtual int isInterested(const Packet& p) = 0;
    HttpCallBack getcb()
    {
        if(ix <0 || ix >= (int)cbs.size()) return NULL;
        return cbs[ix];
    }
protected:
    Filter(){}
    Filter(std::vector<std::string> urls0,std::vector<HttpCallBack> cbs0)
            :urls(urls0),cbs(cbs0),ix(0){}
    std::vector<std::string> urls;
    std::vector<HttpCallBack> cbs;
    int ix;
};

class GetFilter : public Filter
{
public:
    GetFilter(HttpCallBack cb)
    {
        cbs.push_back(cb);
    }
    int isInterested(const Packet& p)
    {
        if(0 == strncmp((const char*)p.data, "GET", 3))
            return ix = 0;
	return ix = -1;
    }
    
};

class UrlFilter : public Filter
{
public:
     UrlFilter(std::vector<std::string> interestedUrls,
            std::vector<HttpCallBack> callbacks )
        :Filter(interestedUrls,callbacks)
        {
//            for(size_t i=0; i<interestedUrls.size();++i)
//                cout << interestedUrls[i] << endl;
        }
    int isInterested(const Packet& p)
    {
        std::vector<std::string>::iterator it = find_if( urls.begin(), urls.end(), IsInterested(p) );
        if( urls.end() !=  it)
        {cout << *it << endl;
            return ix = it - urls.begin();
        }
        else
        {
            return ix = -1;
        }
    }
private:
    class IsInterested
    {
    public:
        IsInterested(const Packet& p)
        {
            packetData.assign((char*)p.data, (size_t)p.dsize);
        }
        bool operator()(const string& url)
        {
            return boost::starts_with(packetData, url);
        }
    private:
        string packetData;
    };
};


#endif	/* HTTPFILTER_H */

