
#ifndef Hybrid_h
#define Hybrid_h

#include <map>
#include <omnetpp.h>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "SimpleAddress.h"

#include <list>


#include "NetwControlInfo.h"
#include "NetwToMacControlInfo.h"
#include "MacToNetwControlInfo.h"
#include "MacPkt_m.h"
#include "BaseMacLayer.h"
using namespace std;

class SimTracer;
class CnSmPkt;

class MIXIM_API Hybrid : public BaseNetwLayer
{
private:
   Hybrid(const Hybrid&);
    Hybrid& operator=(const Hybrid&);

public:
    Hybrid()
        : BaseNetwLayer()
        , headerLength(0)
        , sinkAddress(0)
        , floodSeqNumber(0)
        , defaultTtl(0)
        , nbDataPacketsForwarded(0)
        , nbDataPacketsReceived(0)
        , nbDataPacketsSent(0)
        , nbHops(0)

    {}
    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);
    virtual void finish();

    virtual ~Hybrid();
public:
    int check;
    int tabmsgrec[7];
    int nbHopN;
    int tab[6];
    int i;
    int rep;
    int k;
    int rep1;

protected:
    enum messagesTypes {
        UNKNOWN=0,
        DATA,
        PING,
        PONG,
        PONG_TIMER
    };

    int defaultTtl;
    int headerLength;
    LAddress::L3Type sinkAddress;
    unsigned long floodSeqNumber;
    long nbDataPacketsForwarded;
    long nbDataPacketsReceived;
    long nbDataPacketsSent;
    long dataReceived;
    long nbHops;
    int nbPINGsent;
    int nbPINGrecv;
    int nbPONGsent;
    int nbPONGrecv;
    int pong;
    int nummess;
    int NbDiffMax;


    typedef map<int, CnSmPkt*> waitingMap;
    waitingMap waitMap;


    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage* msg);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage* msg);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage* msg);

    /** @brief Decapsulate a message */
    cMessage* decapsMsg(CnSmPkt *msg);

    virtual void handleSelfMsg(cMessage *msg);
    void insertmessage(CnSmPkt *msg);
    void sendmessages();


};

#endif

