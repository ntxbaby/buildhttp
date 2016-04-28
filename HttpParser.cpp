#include "HttpParser.h"
#include <errno.h>
#include <zlib.h>
#include <fstream>
#include <sys/stat.h>
#include <zdb.h>
#include <iconv.h>
#include <iterator>
#include <algorithm>
#include <signal.h>




void HttpSession::initNewSession(const Packet& p, uint32_t http)
{
    _state = HTTP_REQUEST_HEAD;
    uint32_t seq = ntohl(p.tcph->th_seq);
    if(1 == http)
    {
        _req = new HttpGet_(seq, seq);
    }
    else
    {
        _req = new HttpPost_(seq, seq);
    }
    _res = new HttpResponse;
    _req->buildHead(p, _state);

}

bool HttpSession::dealPacket(const Packet& p)
{
    TcpSession::DIRECT dire = tcpSession.isSameTcpSession(p);
    if(TcpSession::UP == dire)
    {
         _req->buildHead(p, _state);
         return false;
    }
    else if(TcpSession::DOWN == dire)
    {
        bool bfin = _res->buildHead(p, _state);
        if(bfin)
        {
//            ofstream f("./dir/log", ios_base::out | ios_base::app);
//            f << "===============================" << endl;
//            f << _req->getHead() << endl;
//            f << _req->getBody() << endl;
//             f << "-------------------------------" << endl;
//            f << _res->getHead() << endl;
//            f << _res->getBody() << endl;
        }
        return bfin;
    }
    else
    {
        return false;
    }
    
}


string strMid(const string& src, const string& begin, const string& end)
{
    size_t b = src.find(begin);
    if(string::npos == b)
    {
        return "";
    }
    b += begin.size();
    size_t e = src.find(end, b);
    if(string::npos == e)
    {
        return "";
    }
    return src.substr(b, e - b);
}

bool uncompressBody(string& body)
{
    z_stream d_stream;
    d_stream.zalloc = NULL;
    d_stream.zfree = NULL;
    d_stream.opaque = NULL;
    d_stream.next_in = (Bytef *)body.c_str();
    d_stream.avail_in = body.size();
    inflateInit2(&d_stream, 15 + 32);
    
    string result;
    Bytef tmp[256] = {0};
    do 
    {
	d_stream.next_out = (Bytef *)tmp;
	d_stream.avail_out = 256;
	int res = inflate(&d_stream, Z_NO_FLUSH);
        
	if (res != Z_OK && res != Z_STREAM_END) 
	{
	    return false;
	} 
	else 
	{
            int n = 256 - d_stream.avail_out;
	    result.append(tmp, tmp + n);
	}
    } 
    while (d_stream.avail_out == 0);
        
    inflateEnd(&d_stream);
    body = result;
    return true;
}



/*
int conv(const string& from, const string& to, const string& src, string& dst)
{
	char* ps = new char[src.size() + 1];
	strcpy(ps, src.c_str());
	ps[src.size()] = 0;
	char* pd = new char[3 * src.size() + 1];
	int ret = conv(from.c_str(), to.c_str(), ps, src.size(), pd, 3*src.size() + 1);
	if( ret > 0)
		dst = pd;
	delete [] ps;
	delete [] pd;
	return ret; 	
}
*/
string conv(const string& from, const string& to, const string& src) 
{
    iconv_t h = iconv_open(to.c_str(), from.c_str());
    if (iconv_t(-1) == h)
        return "";

    size_t srclen = src.size();
    char* pSrc = const_cast<char*> (src.c_str());

    size_t dstbuffsize = srclen * 6;
    char *pDst0, *pDst;
    pDst0 = pDst = new char[dstbuffsize];
    memset(pDst, 0, dstbuffsize);

    size_t ret = iconv(h, &pSrc, &srclen, &pDst, &dstbuffsize);
    iconv_close(h);
    if (size_t(-1) == ret) 
    {
        delete [] pDst0;
        return "";
    }

    //pDst will point the next position of result bytes;
    string dst(pDst0, pDst);
    delete [] pDst0;
    return dst;
}

int IsTextUTF8(const char* str, long length) {
    int i;
    //utf8 usually 1-6bytes, ascii is 1 byte
    int nBytes = 0;
    unsigned char chr;
    for (i = 0; i < length; i++) {
        chr = *(str + i);
        if (nBytes == 0) {
            if (chr >= 0x80) {
                //leading byte of multi-bytes char set, to calculate count of bytes 
                if (chr >= 0xE0 && chr < 0xF0)
                    nBytes = 3;
                else {
                    return 0;
                }
                nBytes--;
            } else {
                //is utf8 compatible with ascii
                continue;
            }
        } else {
            // is not leading byte of multi-bytes char set, must be 10xxxxxx 
            if ((chr & 0xC0) != 0x80) {
                return 0;
            }
            nBytes--;
        }
    }

    if (nBytes > 0) //go against utf8 rule 
    {
        return 0;
    }
    return 1;
}

    void HttpMsg::pushIntoMap(const Packet& p)
    {
        uint32_t curSeq = ntohl(p.tcph->th_seq);
        PacketMapITER it = _disorder.find(curSeq);

        if(_disorder.end() == it)
        {
            //not found
            _disorder.insert( make_pair( curSeq, DisorderData(p) ) );
        }
        else
        {
            //found, repeate packet, reserve the longer one
            if(p.dsize > it->second.getDataSize())
            {
                it->second = DisorderData(p);
            }
        }
    }
     
     int HttpMsg::disOrderMapProc(DisorderData& cur, string& nextData)
    {
      

         uint32_t curSeq = cur.getSeq();
         uint32_t curSize = cur.getDataSize();
         string curData = cur.getData();

            if(curSeq > _seq)
            {
                return 1;
            }
            else if(_seq == curSeq)
            {
                nextData = curData;
            }
            else if( _startSeq <= curSeq  && curSeq  < _seq && (curSeq + curSize) > _seq)
            {
                uint32_t offset = _seq - curSeq ;
                nextData = curData.substr(offset);
            }
            else
            {
                return 2;
            }
         return 0;
    }
     
     size_t HttpMsg::isHeadOk(const Packet& p)
    {
        std::string data((const char*)p.data, (const char*)p.data + p.dsize);
        return data.find(CRLF CRLF);
    }
    size_t HttpMsg::isHeadOk(const string& data)
    {
        return data.find(CRLF CRLF);
    }
    

    bool HttpGet_::buildHead(const Packet& p, uint32_t& state)
    {
        if(state <= HTTP_REQUEST_HEAD)
        {
            pushIntoMap(p);
            while(!_disorder.empty())
            {
                PacketMapITER it = _disorder.begin();
                string curData;
                int ret = disOrderMapProc(it->second, curData);

                if( 0== ret)
                {
                    _seq += curData.size();
                    size_t headEnd = isHeadOk(curData);
                    if(std::string::npos != headEnd)
                    {
                        state = HTTP_RESPONSE_HEAD;
                        _head.append(curData);
                    }
                    else
                    {
                        _head.append(curData);
                    }
                    _disorder.erase(it);

                }
                else if( 1 == ret)
                {
                    return false;
                }
                else if( 2 == ret)
                {
                    _disorder.erase(it);
                }

            }
            return false;
        }
        return false;
    }
    bool HttpGet_::buildBody(const Packet& p, uint32_t& state)
    {
        
        return false;
    }
    
   
    void HttpMsgWithBody::dealHead()
    {
         string chunked = strMid(_head, "Transfer-Encoding: ", CRLF);
        if(chunked.empty())
        {
            _bodyLen = parseContentLength(_head);
        }
        else
        {
            _flag |= RESPONSE_HEAD_CHUNKED;
        }

        string gzip = strMid(_head, "Content-Encoding: ", CRLF);
        if(!gzip.empty())
        {
            _flag |= RESPONSE_HEAD_GZIP;
        }
    }
    uint32_t HttpMsgWithBody::parseContentLength(const string& src)
    {
        string len = strMid(src, "Content-Length: ", CRLF);
        if(len.empty())
            return 0;
        return atol(len.c_str());
    }
    
    bool HttpMsgWithBody::isChunkBodyOk(const string& packetData)
    {
        boost::regex p("(0+?)\r\n\r\n");
        boost::smatch what;
        if(boost::regex_search(packetData,  what, p))
        {
                    //remove chunk tailer if existing
            return true;
        }
        return false;
    }

    bool HttpMsgWithBody::isBodyOk()
    {
        if( _flag & RESPONSE_HEAD_CHUNKED)
        {
           return isChunkBodyOk(_body);
        }
        else
        {
            if(_body.size() >= _bodyLen)
            {
                return true;
            }
            return false;
        }
    }
    
    
    
    bool HttpMsgWithBody::buildHead(const Packet& p, uint32_t& state)
    {
        if(state <= getHeadState())
        {
            pushIntoMap( p);
            if(!isMsgHeadOk(p))
                return false;
            while(!_disorder.empty())
            {
                    PacketMapITER it = _disorder.begin();
                    string curData;
                    int ret = disOrderMapProc(it->second, curData);

                    if( 0== ret)
                    {
                        _seq += curData.size();
                        size_t headEnd = isHeadOk(curData);
                        if(std::string::npos != headEnd)
                        {
                            state = getBodyState();
                            _head.append(curData.substr(0, headEnd+4));
                            _body.assign(curData.substr(headEnd+4));
                            dealHead();
                            if(isBodyOk())
                            {
                                return dealBody(state);
                            }
                        }
                        else
                        {
                            _head.append(curData);
                        }
                        _disorder.erase(it);

                    }
                    else if( 1 == ret)
                    {
                        return false;
                    }
                    else if( 2 == ret)
                    {
                        _disorder.erase(it);
                    }

            }
            return false;
        }
        else
        {
            return buildBody(p, state);
        }
    }
    bool HttpMsgWithBody::buildBody(const Packet& p, uint32_t& state)
    {
        if(state <= getBodyState())
        {
            pushIntoMap( p);
            while(!_disorder.empty())
            {
               PacketMapITER it = _disorder.begin();
                string curData;
                int ret = disOrderMapProc(it->second, curData);

                if( 0== ret)
                {
                    _seq += curData.size();

                    _body.append(curData);
                    _disorder.erase(it);
                    if(isBodyOk())
                    {
                        return dealBody(state);
                    }
                }
                else if( 1 == ret)
                {
                    return false;
                }
                else if( 2 == ret)
                {
                    _disorder.erase(it);
                }
            }
        }
        return false;
    }
  
        bool HttpResponse::dealBody(uint32_t& state)
    {
        //http session ok
        if( _flag & RESPONSE_HEAD_CHUNKED)
        {
            _body = CRLF + _body;
            boost::regex p("\r\n([0-9a-fA-F]+?)\r\n");
            _body = boost::regex_replace(_body,  p, "");
        }
        if( _flag & RESPONSE_HEAD_GZIP)
        {
            uncompressBody(_body);
        }
        return true;
    }
    bool HttpResponse::isMsgHeadOk(const Packet& p)
    {
        if(_startSeq != 0)
        {
            return true;
        }
        else
        {
            if(isResponsePacket(p))
            {
                _startSeq = _seq = ntohl(p.tcph->th_seq);
                return true;
            }
        }
        return false;
    }                                                                                                                                                                                                                       
	

    int HttpParser::isHttpPacket(const Packet& p)
    {
     /*
    boost::regex except("^GET\\s([^\\?\\s\r\n]*?)\\.(jpg|png|gif|css|js)(\\s|\\?)");    
    boost::smatch what;
    string data(p.data, p.data+p.dsize);
    return ( 0 == strncmp((const char*)p.data, "GET", 3) && !boost::regex_search(data,  what, except) );
GET /hainanair/ibe/checkout/paymentDetails.do?
 const char* urlgj="GET /international/NewUI/Review.aspx";
     const char* urlgn="POST /Book/Review HTTP/1.1";
    */
        if(0 == strncmp((const char*)p.data, "GET", 3)) return 1;
        if(0 == strncmp((const char*)p.data, "POST", 4)) return 2;
	return 0;
    }
    bool HttpParser::parse(const Packet& p)
    {
        bool httpPort = (p.dp == 80 || p.sp == 80); 
        if(!httpPort) return false;
         
        if(p.tcph == 0) return false;
        
        bool b1 = p.dsize >0 ;
        if(!b1) return false;
  //      bool b2 = (p.tcph->th_flags & TH_RST);
//        if(!b1 && !b2)
//            return;

        // if(b2)
//        {
//            printf("flag=%hhX\n", p.tcph->th_flags);
//           if(b2) cout << "++++reset"<< endl;
//            uint32_t seq = ntohl(p.tcph->th_seq);
//            printf("++++seq=%X\n", seq);
//            TcpSession t(p);
//            cout << "srcip:" << t.getIp(1) << endl;
//            cout << "dstip:" << t.getIp(0) << endl;
//            cout << "srcport:" << t.getPort(1) << endl;
//            cout << "dstport:" << t.getPort(0) << endl;
//          
//        }
//        string data((char*)p.data, (int)p.dsize);
//        cout << "data=" << data << endl;
        /*
        time_t t = time(NULL);
        
        if(t - curtime > 2)
        {
            curtime = t;
            for(ITER it = httpss.begin(); it != httpss.end(); )
            {
                if( (*it)->isTimeOut(curtime) )
                {
                    delete *it;
                    it = httpss.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
        */
        ITER it = find_if(httpss.begin(), httpss.end(), FindHttpSession(p));
        
        if(it != httpss.end())
        {

            bool ok = (*it)->dealPacket(p);
            
            if(ok)
            {   
                (*it)->callback()(*it);
                delete *it;
                httpss.erase(it);
                return true;
            }
        }
        else
        {
            
            if(filter->isInterested(p) == -1) 
                return false;
            int http = isHttpPacket(p);
            if(0 != http)
            {
                HttpSession* hs = new HttpSession(p, filter->getcb(), /*t*/0);;
                hs->initNewSession(p, http);
                httpss.push_back(hs);
            }
        }
        return false;

    }
    
    bool HttpParser::parse(PacketInfo* p)
    {
        Packet packet;
        
    char ether[14] = {0};
    ether[12] = 0x08;
    memcpy(ether, p->destMac, 6);
    memcpy(ether + 6, p->srcMac, 6);
    
    packet.proto_bits = 0;
    packet.dsize = p->bodyLen;            /* packet payload size */

    //vvv-----------------------------
    packet.sp = p->srcPort;              /* source port (TCP/UDP) */
    packet.dp = p->destPort;                /* dest port (TCP/UDP) */

    packet.direction = 0;

    //vvv-----------------------------
    packet.eh = (EtherHdr*)ether;         /* standard TCP/IP/Ethernet/ARP headers */
    //const PPPoEHdr *pppoeh;     /* Encapsulated PPP of Ether header */
    packet.iph = (IPHdr *)p->ip;
    //const UDPHdr *udph;
    packet.tcph = (TCPHdr *)p->tcp;
    packet.pkth = NULL;
    packet.data = (const uint8_t *)p->body;        /* packet payload pointer */
    
    //sMac
    packet.pkt = NULL;         /* raw packet data */

        
        return parse(packet);
    }
    
inline int char2num(char ch) {
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
}
/**
 * @brief   translate %uxxxx to xxx(a utf8 char)
 * @return  pointer to result string
 */
inline char* zhUni2Utf8(char* uni) {
    //'%uxxxx' to 'xxx'
    unsigned short uxxxx = 0;
    for (int i = 2; i <= 5; ++i)
        uxxxx = uxxxx * 16 + char2num(uni[i]);

    uni[2] = (uxxxx & 0x3F) | 0x80;
    uni[1] = ((uxxxx >> 6) & 0x3f) | 0x80;
    uni[0] = ((uxxxx >> 12) & 0x0f) | 0xE0;
    return uni;
}

/**
 * @brief remove '%', and translate %uXXXX%uXXXX, %2525uXXXX(one or more '25' is ok) to utf8 string
 * @param  src (in and out)  a string with '%XX' or '%2525...XX' or '%2525...uXXXX' or all of them  
 * @return pointer to result string 
 */
char* urlDecodec(char* src) {
    if (src == NULL) return NULL;
    char* p = src;
    while (*p != '\0') {
        if (*p != '%') {
            p++;
            continue;
        } else {
            if (isxdigit(p[1])) {
                *p = char2num(p[1]) * 16 + char2num(p[2]);
                memmove(p + 1, p + 3, strlen(p + 3) + 1);
            } else if (p[1] = 'u') {
                zhUni2Utf8(p);
                memmove(p + 3, p + 6, strlen(p + 6) + 1);
            }

        }
    }

    return src;
}

string urlDecode(const string& src) {
    if(src.empty()) return "";
    char* b = new char[src.size() + 1];
    bzero(b, src.size() + 1);
    strcpy(b, src.c_str());
    string ret(urlDecodec(b));
    delete [] b;
    return ret;
}
