
#ifndef CnSmp_h
#define CnSmp_h

#include <map>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "SimpleAddress.h"

class SimTracer;
class CnSmpPkt;

class MIXIM_API CnSmp : public BaseNetwLayer
{
private:
   CnSmp(const CnSmp&);
    CnSmp& operator=(const CnSmp&);

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
    CnSmp()
        : BaseNetwLayer()
        , headerLength(0)
        , sinkAddress()
        , useSimTracer(false)
        , floodSeqNumber(0)
        , tracer(NULL)
        , nbDataPacketsForwarded(0)
        , nbDataPacketsReceived(0)
        , nbDataPacketsSent(0)
        , nbHops(0)
        , trace(false), stats(false), debug(false)
    {}
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    virtual ~CnSmp();
public:
    int check;
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


    int headerLength;
    LAddress::L3Type sinkAddress;
    bool useSimTracer;
    unsigned long floodSeqNumber;
    SimTracer *tracer;
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
    int optiEnable;
    int posture;


    bool trace, stats, debug;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    bool notBroadcasted( CnSmpPkt* );

    void forwardControl( CnSmpPkt* );

    void recuControl( CnSmpPkt* );

    void multiSend( CnSmpPkt* );

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(CnSmpPkt *msg);


};

#endif
