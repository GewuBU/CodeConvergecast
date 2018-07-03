
#ifndef Ctp_h
#define Ctp_h

#include <map>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "SimpleAddress.h"

class SimTracer;
class CtpPkt;

class MIXIM_API Ctp : public BaseNetwLayer
{
private:
    Ctp(const Ctp&);
    Ctp& operator=(const Ctp&);

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
    Ctp()
: BaseNetwLayer()
, headerLength(0)
, floodSeqNumber(0)
,beaconNumber(0)
, defaultTtl(0)
, netQueue()
,ackTimer(NULL)
, nbDataPacketsForwarded(0)
, nbDataPacketsReceived(0)
, nbDataPacketsSent(0)
, nbHops(0)

{}
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    virtual ~Ctp();
public:
    int ordre[7][5];
    int nbHopN;
    int i;
    int rep;
    int k;
    int rep1;

protected:
    //BGW Queue for network layer
    typedef std::list<CtpPkt*> NetQueue;
    NetQueue netQueue;

    CtpPkt* ackTimer;

    enum messagesTypes {
        UNKNOWN=0,
        DATA,
        BEACON,
        ACK,
        ACKTimer
    };


    int headerLength;
    unsigned long floodSeqNumber;
    unsigned long beaconNumber;
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
    unsigned int maxNetQueueSize;
    double etxM;
    double etxM_v[3][7];
    double etx1_0;
    double etx1_1;
    double etx1_2;
    double etx1_3;
    double etx1_4;
    double etx1_5;
    double etx1_6;
    double parent;
    double contBeacon0;
    double seqBeacon0;
    double contBeacon1;
    double seqBeacon1;
    double contBeacon2;
    double seqBeacon2;
    double contBeacon3;
    double seqBeacon3;
    double contBeacon4;
    double seqBeacon4;
    double contBeacon5;
    double seqBeacon5;
    double contBeacon6;
    double seqBeacon6;
    double Qb0;
    double Qb1;
    double Qb2;
    double Qb3;
    double Qb4;
    double Qb5;
    double Qb6;
    bool valParent;
    int txAttempts;
    int maxTry;


    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    /** @brief we have no self messages */
    virtual  void handleSelfMsg(cMessage* msg) ;

    bool notBroadcasted( CtpPkt* );

    void forwardControl( CtpPkt* );

    void recuControl( CtpPkt* );

    void tryToSend();

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(CtpPkt *msg);


};

#endif
