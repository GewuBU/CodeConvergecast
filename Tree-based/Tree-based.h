
#ifndef CnSmOpt_h
#define CnSmOpt_h

#include <map>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "SimpleAddress.h"

class SimTracer;
class CnSmPkt;

class MIXIM_API CnSmOpt : public BaseNetwLayer
{
private:
    CnSmOpt(const CnSmOpt&);
    CnSmOpt& operator=(const CnSmOpt&);

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


public:
    CnSmOpt()
        : BaseNetwLayer()
        , headerLength(0)
        ,appLength(0)
        , sinkAddress()
        , floodSeqNumber(0)
        , defaultTtl(0)
        , netQueue()
        , nbDataPacketsForwarded(0)
        , nbDataPacketsReceived(0)
        , nbDataPacketsSent(0)
        , nbHops(0)

    {}
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    virtual ~CnSmOpt();
public:
    int check;
    int tabmsgrec[7][5];
    int nbHopN;
    int tab[2][7];
    int i;
    int rep;
    int k;
    int rep1;

protected:
    //BGW Queue for network layer
    typedef std::list<CnSmPkt*> NetQueue;
    NetQueue netQueue;

    enum messagesTypes {
        UNKNOWN=0,
        DATA
    };


    int headerLength;
    int appLength;
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
    long nbDesquence0;
    long nbDesquence2;
    long nbDesquence3;
    long nbDesquence4;
    long nbDesquence5;
    long nbDesquence6;
    unsigned long nbRecu0;
    unsigned long nbRecu2;
    unsigned long nbRecu3;
    unsigned long nbRecu4;
    unsigned long nbRecu5;
    unsigned long nbRecu6;
    int maxNetQueueSize;
    int posture;
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


    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    bool notBroadcasted( CnSmPkt* );

    void forwardControl( CnSmPkt* );

    void recuControl( CnSmPkt* );

    void tryToSend( CnSmPkt* );
    void multiSend( CnSmPkt* );
    void TimeDis(CnSmPkt *);

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(CnSmPkt *msg);


};

#endif
