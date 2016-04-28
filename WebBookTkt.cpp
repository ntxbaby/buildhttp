
#include "WebBookTkt.h"
#include "HttpParser.h"


void AnalyseNanHang(HttpSession* hs);
void AnalyseHaiHang(HttpSession* hs);
void AnalyseDongHang(HttpSession* hs);

void AnalyseXieChengGuoji(HttpSession* hs);
void AnalyseXieChengGuonei(HttpSession* hs);
void AnalyseXieChengBus(HttpSession* hs);
void AnalyseXieChengTrain(HttpSession* hs);

void Passenger12308(HttpSession* hs);
void CityTime12308(HttpSession* hs);

//HttpParser pdh("GET /otabooking/paxinfo-input!getBooingInfojsonView.shtml", AnalyseDongHang);
//HttpParser phh("GET /hainanair/ibe/checkout/paymentDetails.do?", AnalyseHaiHang);
//HttpParser pnhgn("POST /B2C40/showPayOrder/jaxb/order.ao", AnalyseNanHang);
//HttpParser pnhgj("POST /B2C40/showPayOrder/interOrder/jaxb/getInterOrderInfo.ao", AnalyseNanHang);
//
//HttpParser Pxcgj("GET /international/NewUI/Review.aspx", AnalyseXieChengGuoji);
//HttpParser pxcgn("POST /Book/Review HTTP/1.1", AnalyseXieChengGuonei);
//HttpParser pxcbus("POST /index.php?param=/buy/ajaxCommitOrder", AnalyseXieChengBus);
//HttpParser pxctrain("POST /TrainBooking/InputPassengers.aspx", AnalyseXieChengTrain);




void AnalyseDaBa(HttpSession* hs);
void Analyse966800(HttpSession* hs);
//guojihangkong
void AnalyseGHFlight(HttpSession* hs);
void AnalyseGHPsg(HttpSession* hs);
//chang tu
void AnalyseCTFlight(HttpSession* hs);
void AnalyseCTPsg(HttpSession* hs);

const string urls[] = {"GET /otabooking/paxinfo-input!getBooingInfojsonView.shtml",     //donghang
    "GET /hainanair/ibe/checkout/paymentDetails.do?",                                   //haihang 
    "POST /B2C40/showPayOrder/jaxb/order.ao",                                           //nanhang guonei
    "POST /B2C40/showPayOrder/interOrder/jaxb/getInterOrderInfo.ao",                    //nanhang guoji
    "GET /international/NewUI/Review.aspx",
    "POST /Book/Review HTTP/1.1",
    "POST /index.php?param=/buy/ajaxCommitOrder",
    "POST /TrainBooking/InputPassengers.aspx",
    "GET /orderPay.aspx?Ord_No=",
    "POST /order/submitOrder.html HTTP/1.1",
    "GET /toPay.htm?orderNo=",
    "POST /ticket/confirm.html",
    "POST /InternetBooking/ValidateFormAction.do",                                      //guojihangkong passengers infomation
    "GET /InternetBooking/ItinerarySummary.do",                                         //guojihangkong flight infomation
    "POST /trade/order/submitOrders.htm",
    "GET /pay/onlinePay.htm?orderId="
};

HttpCallBack cbs[] = {AnalyseDongHang, 
AnalyseHaiHang, 
AnalyseNanHang, 
AnalyseNanHang,
AnalyseXieChengGuoji,
AnalyseXieChengGuonei,
AnalyseXieChengBus,
AnalyseXieChengTrain,
AnalyseDaBa,
Passenger12308,
CityTime12308,
Analyse966800,
AnalyseGHPsg,
AnalyseGHFlight,
AnalyseCTPsg,
AnalyseCTFlight
};

UrlFilter uf(vector<string>(urls, urls+ sizeof(cbs)/sizeof(HttpCallBack) ), vector<HttpCallBack>(cbs, cbs+sizeof(cbs)/sizeof(HttpCallBack) ) );

HttpParser hp(&uf);


bool WebBookTkt::IsWebBookTkt(PacketInfo* pktInfo)
{

    bool isWebBookTkt = false;

//    Pxcgj.parse(pktInfo);
//    
//    pxcgn.parse(pktInfo);
//    pxcbus.parse(pktInfo);
//    pxctrain.parse(pktInfo);
//    pdh.parse(pktInfo);
//    phh.parse(pktInfo);
//    pnhgj.parse(pktInfo);
//    pnhgn.parse(pktInfo);
//    
//    phh.parse(pktInfo);
    hp.parse(pktInfo);
    return isWebBookTkt;
}

    
