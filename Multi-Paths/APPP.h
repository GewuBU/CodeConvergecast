
#ifndef CnSm1_h
#define CnSm1_h

#include <map>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "SimpleAddress.h"

class SimTracer;
class CnSm1Pkt;

class MIXIM_API CnSm1 : public BaseNetwLayer
{
private:
    CnSm1(const CnSm1&);
    CnSm1& operator=(const CnSm1&);


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
    CnSm1()
: BaseNetwLayer()
, sinkAddress()
, defaultTtl(0)
, floodSeqNumber(0)
, nbDataPacketsForwarded(0)
, nbDataPacketsReceived(0)
, nbDataPacketsSent(0)
, nbHops(0)

{}
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    virtual ~CnSm1();
public:
    int check;
    int defaultTtl;
    int tabmsgrec[7][5];
    int nbHopN;
    int tab[6];
    int i;
    int rep;
    int k;
    int rep1;
    int rnd;

protected:
    enum messagesTypes {
        UNKNOWN=0,
        DATA
    };



    LAddress::L3Type sinkAddress;
    unsigned long floodSeqNumber;
    long nbDataPacketsForwarded;
    long nbDataPacketsReceived;
    long nbDataPacketsSent;
    long dataReceived;
    long nbHops;
    unsigned long Deseqence0;
    unsigned long Deseqence2;
    unsigned long Deseqence3;
    unsigned long Deseqence4;
    unsigned long Deseqence5;
    unsigned long Deseqence6;
    unsigned long nbRecu0;
    unsigned long nbRecu2;
    unsigned long nbRecu3;
    unsigned long nbRecu4;
    unsigned long nbRecu5;
    unsigned long nbRecu6;
    unsigned long nbForwarded0;
    unsigned long nbForwarded2;
    unsigned long nbForwarded3;
    unsigned long nbForwarded4;
    unsigned long nbForwarded5;
    unsigned long nbForwarded6;
    long nbDesquence;
    int optiEnable;
    int posture;


    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    bool notBroadcasted( CnSm1Pkt* );

    void forwardControl( CnSm1Pkt* );

    void recuControl( CnSm1Pkt* );

    void multiSend( CnSm1Pkt* );

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(CnSm1Pkt *msg);


};

#endif
