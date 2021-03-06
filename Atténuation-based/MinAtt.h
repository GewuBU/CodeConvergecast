#ifndef _RoutGeo1_H_
#define _RoutGeo1_H_

#include <list>

#include "MiXiMDefs.h"
#include "BaseNetwLayer.h"
#include "SimpleAddress.h"

/**
 * @brief A simple flooding protocol
 *
 * This implementation uses plain flooding, i.e. it "remembers"
 * (stores) already broadcasted messages in a list and does not
 * rebroadcast them again, if it gets another copy of that message.
 *
 * The maximum number of entires for that list can be defined in the
 * .ini file (@ref bcMaxEntries) as well as the time after which an entry
 * is deleted (@ref bcDelTime).
 *
 * If you prefere a memory-less version you can comment out the 
 * @verbatim #define PLAINFLOODING @endverbatim
 *
 * @ingroup netwLayer
 * @author Daniel Willkomm
 *
 * ported to Mixim 2.0 by Theodoros Kapourniotis
 **/
class MIXIM_API RoutGeo1 : public BaseNetwLayer
{

protected:

    typedef std::list<NetwPkt*> NetQueue;
    NetQueue netQueue;
    NetwPkt* repTimer;

    enum messagesTypes {
            UNKNOWN=0,
            AttenuationREQ,
            MESSAGE_DATA,
            AttenuationREP,
            REQTimer
        };

    /** @brief Network layer sequence number*/
    unsigned long seqNum;
    long int NextHopAddr;

    /** @brief Default time-to-live (ttl) used for this module*/
    int defaultTtl;

    /** @brief Defines whether to use plain flooding or not*/
    bool boolRoutGeo1;

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
    RoutGeo1()
        : BaseNetwLayer()
        , seqNum(0)
        , defaultTtl(0)
        , boolRoutGeo1(false)
        , bcMsgs()
        , bcMaxEntries(0)
        , bcDelTime()
        , netQueue()
        , nbDataPacketsReceived(0)
        , nbDataPacketsSent(0)
        , nbDataPacketsForwarded(0)
        , nbHops(0)
    {}

    /** @brief Initialization of omnetpp.ini parameters*/
    virtual void initialize(int);
    virtual void finish();    

public:
    //SoundManager& ptr1=SoundManager::Instance();
protected:

    long nbDataPacketsReceived;
    long nbDataPacketsSent;
    long nbDataPacketsForwarded;
    long nbHops;
    long dataReceived;
    long AllmsgReceived;

    double BestAtt;
    int nbAttREPsent;
    int nbAttREPreceived;
    int nbAttREQsent;
    int nbAttREQreceived;
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
    int optiEnable;
    long nbOverFlowNetBuff;
    int posture;



    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage *);

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage *);

    /** @brief we have no self messages */
    void handleSelfMsg(cMessage* /*msg*/) ;
    
    /** @brief Checks whether a message was already broadcasted*/
    bool notBroadcasted( NetwPkt* );

    void forwardControl( NetwPkt* );

    void recuControl( NetwPkt* );

    void multiSend( NetwPkt* );

    void delayTest(NetwPkt*);

    //overloading encaps method
    virtual NetwPkt* encapsMsg(cPacket*);
};

#endif
