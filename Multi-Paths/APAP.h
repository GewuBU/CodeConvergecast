
#ifndef CnSm_h
#define CnSm_h

#include <map>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "SimpleAddress.h"
//zkp
#include <fstream>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/pem2.h>
//zkp

class SimTracer;
class CnSmPkt;

class MIXIM_API CnSm : public BaseNetwLayer
{
private:
    CnSm(const CnSm&);
    CnSm& operator=(const CnSm&);

    //BGW
    class Bcast {
    public:
        unsigned long    seqNum;
        LAddress::L3Type srcAddr;
        simtime_t        delTime;
    public:
        Bcast(unsigned long n=0, const LAddress::L3Type& s = LAddress::L3NULL,  simtime_t_cref d=SIMTIME_ZERO) :
            seqNum(n), srcAddr(s), delTime(d) {
        }
    };

    typedef std::list<Bcast> cBroadcastList;

    /** @brief List of already broadcasted messages*/
    cBroadcastList bcMsgs;

    /**
     * @brief Max number of entries in the list of already broadcasted
     * messages
     **/
    unsigned int bcMaxEntries;

    /**
     * @brief Time after which an entry for an already broadcasted msg
     * can be deleted
     **/
    simtime_t bcDelTime;
    simtime_t delay;

public:
    CnSm()
: BaseNetwLayer()
, headerLength(0)
, sinkAddress()
, floodSeqNumber(0)
, defaultTtl(0)
, netQueue()
, nbDataPacketsForwarded(0)
, nbDataPacketsReceived(0)
, nbDataPacketsSent(0)
, nbHops(0)
, len(0)
, len11(0)
,delay(0.0)

{}
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    virtual ~CnSm();
public:
    int check;
    int tabmsgrec[7][5];
    int nbHopN;
    int tab[6];
    int tabson[2];
    int i;
    int rep;
    int k;
    int rep1;
    int n;
    int j;
    int y;


protected:
    //BGW Queue for network layer
    typedef std::list<CnSmPkt*> NetQueue;
    NetQueue netQueue;

    CnSmPkt* dataPck;

    enum messagesTypes {
        UNKNOWN=0,
        DATA,
        ZKPC0,
        ZKPC1,
        ZKPC2,
        ZKPC3
    };


    int headerLength;
    LAddress::L3Type sinkAddress;
    unsigned long floodSeqNumber;
    int defaultTtl;
    long nbDataPacketsForwarded;
    long nbDataPacketsReceived;
    long nbDataPacketsSent;
    long dataReceived;
    long overbuff;
    long nbHops;
    unsigned long Deseqence0;
    unsigned long Deseqence2;
    unsigned long Deseqence3;
    unsigned long Deseqence4;
    unsigned long Deseqence5;
    unsigned long Deseqence6;
    unsigned long nbForwarded0;
    unsigned long nbForwarded2;
    unsigned long nbForwarded3;
    unsigned long nbForwarded4;
    unsigned long nbForwarded5;
    unsigned long nbForwarded6;
    long nbDesquence;
    unsigned long nbRecu0;
    unsigned long nbRecu2;
    unsigned long nbRecu3;
    unsigned long nbRecu4;
    unsigned long nbRecu5;
    unsigned long nbRecu6;
    int maxNetQueueSize;
    int optiEnable;
    int zkpEnable;
    int posture;
    //bgw
    int t005;
    int t010;
    int t015;
    int t020;
    int t025;
    int t030;
    int t035;
    int t040;
    int t045;
    int t050;
    int t055;
    int t060;
    int t065;
    int t070;
    int t075;
    int t080;
    int t085;
    int t090;
    int t095;
    int t100;
    int t105;
    int t110;
    int t115;
    int t120;
    int t125;
    int t130;
    int t135;
    int t140;
    int t145;
    int t150;
    int t155;
    int t160;
    int t165;
    int t170;
    int t175;
    int t180;
    //bgw
    //zkp
    BIGNUM *rnd;
    BIGNUM *range;
    BIGNUM *resl;
    BIGNUM *cop;
    BIGNUM *puiss;
    BIGNUM *div;
    BIGNUM *rem;
    BIGNUM *reslf;
    BIGNUM *x;
    BIGNUM *yy;
    const BIGNUM *cop1;

    char const * V;
    char const * V01;
    char const * V02;
    char const * V03;
    char const * V04;
    char const * V05;
    char const * V06;
    //////////////////////////////////////////////////////////////////////////////////
    char const * q;
    char const * q01;
    char const * q02;
    char const * q03;
    char const * q04;
    char const * q05;
    char const * q06;
    char const * p01;
    char const * p02;
    char const * p03;
    char const * p04;
    char const * p05;
    char const * p06;

    char const * key0N;

    char const * key1;
    char const * key2;
    char const * key3;
    char const * key4;
    char const * key5;
    char const * key6;

    char const * key01;
    char const * key02;
    char const * key03;
    char const * key04;
    char const * key05;
    char const * key06;

    int IDbin[4];
    int len111;
    int siz;
    int tableau[2700];
    int tabx[2700];
    int tab1[2700];
    int Res0 [2700];
    int Res01 [2700];
    int k02[2700];
    char ResFN[2700];
    char ResFS[2700];
    char ResFN1[2700];
    char ResFS1[2700];
    char N[2700];
    char* Xm;
    int concat[750];
    int len;
    int len11;
    int len1;
    int len2;
    int len3;
    int Va;
    int Vacop;
    char lettre;
    //zkp



    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    bool notBroadcasted( CnSmPkt* );

    void forwardControl( CnSmPkt* );

    void recuControl( CnSmPkt* );

    void delayTest(CnSmPkt*);

    void TimeDis(CnSmPkt *);

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(CnSmPkt *msg);

    void multiSend( CnSmPkt* );

    void zkpC0(CnSmPkt*);
    void zkpC1( CnSmPkt* );
    void zkpC2( CnSmPkt* );
    void zkpC3( CnSmPkt* );


};

#endif
