#include "RoutGeo5.h"

#include <cassert>

#include "NetwPkt_m.h"
#include "NetwControlInfo.h"
#include "LogNormalShadowing.h"


using std::endl;

Define_Module(RoutGeo5);


void RoutGeo5::initialize(int stage) {
    BaseNetwLayer::initialize(stage);

    if (stage == 0)
    {
        timerOutofSequencer=par("timerOutofSequencer");
        posture=par("posture");
        optiEnable=par("optiEnable");
        sequencerEnable=par("sequencerEnable");
        seqNum = 0;
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
        nbDataPacketsForwarded = 0;
        nbHops = 0;
        dataReceived=0;
        Deseqence0=0;
        Deseqence2=0;
        Deseqence3=0;
        Deseqence4=0;
        Deseqence5=0;
        Deseqence6=0;
        nbDesquence=0;
        nbForwarded0=0;
        nbForwarded2=0;
        nbForwarded3=0;
        nbForwarded4=0;
        nbForwarded5=0;
        nbForwarded6=0;
        nbRecu0=0;
        nbRecu2=0;
        nbRecu3=0;
        nbRecu4=0;
        nbRecu5=0;
        nbRecu6=0;
        nbTTL=0;
        countNext0=0;
        countNext2=0;
        countNext3=0;
        countNext4=0;
        countNext5=0;
        countNext6=0;

        delay = 0;

        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 5;
        EV<< "defaultTtl = " << defaultTtl<< endl;


        //these parameters are only needed for plain RoutGeo5ing
        hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;

        hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 180.0;
        EV <<"bcMaxEntries = "<<bcMaxEntries
                <<" bcDelTime = "<<bcDelTime<<endl;
    }
    sequencerTimer0 = new NetwPkt("SequencerTimer0", SequencerTimer0);
    sequencerTimer2 = new NetwPkt("SequencerTimer2", SequencerTimer2);
    sequencerTimer3 = new NetwPkt("SequencerTimer3", SequencerTimer3);
    sequencerTimer4 = new NetwPkt("SequencerTimer4", SequencerTimer4);
    sequencerTimer5 = new NetwPkt("SequencerTimer5", SequencerTimer5);
    sequencerTimer6 = new NetwPkt("SequencerTimer6", SequencerTimer6);

}

void RoutGeo5::finish() {

    bcMsgs.clear();
    cOwnedObject *Del=NULL;
    int OwnedSize=this->defaultListSize();
    for(int i=0;i<OwnedSize;i++){
        Del=this->defaultListGet(0);
        this->drop(Del);
        delete Del;
    }

    EV<<" Delay:  "<<delay / nbDataPacketsReceived  << " ; "<< delay<< " ; "<< nbDataPacketsReceived<<endl;

    int t = nbDataPacketsReceived + sequencerQueue0.size() + sequencerQueue2.size() + sequencerQueue3.size() + sequencerQueue4.size() + sequencerQueue5.size() + sequencerQueue6.size();

    recordScalar("nbDataPacketsReceived", t);
    recordScalar("nbDataPacketsSent", nbDataPacketsSent);
    recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
    recordScalar("dataReceived", dataReceived);
    recordScalar("nbDesquence", nbDesquence);
    recordScalar("nbRecu0", nbRecu0);
    recordScalar("nbRecu2", nbRecu2);
    recordScalar("nbRecu3", nbRecu3);
    recordScalar("nbRecu4", nbRecu4);
    recordScalar("nbRecu5", nbRecu5);
    recordScalar("nbRecu6", nbRecu6);
    recordScalar("nbForwarded0", nbForwarded0);
    recordScalar("nbForwarded2", nbForwarded2);
    recordScalar("nbForwarded3", nbForwarded3);
    recordScalar("nbForwarded4", nbForwarded4);
    recordScalar("nbForwarded5", nbForwarded5);
    recordScalar("nbForwarded6", nbForwarded6);
    recordScalar("nbTTL", nbTTL);




    if (nbDataPacketsReceived > 0) {
        recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);
    } else {
        recordScalar("meanNbHops", 0);
    }
}



void RoutGeo5::handleUpperMsg(cMessage* m) {

    assert(dynamic_cast<cPacket*> (m));
    NetwPkt *msg = encapsMsg(static_cast<cPacket*> (m));

    msg->setSeqNum(seqNum);
    seqNum++;
    msg->setTtl(defaultTtl);
    msg->setInitialsrcAddr(myNetwAddr);
    msg->setSrcAddr(myNetwAddr);
    msg->setFinaldestAddr(1);
    msg->setDestAddr(LAddress::L3BROADCAST);
    NetwPkt* pkt =msg->dup();
    if(optiEnable==0)
    {
        setDownControlInfo(pkt, LAddress::L2BROADCAST);
        nbDataPacketsSent++;
        sendDown(pkt);
        EV<<" time "<<pkt->getSendingTime()<<"  simtime "<<simTime()<<endl;
    }
    else
        multiSend(pkt);
    delete(msg);
}

void RoutGeo5::handleLowerMsg(cMessage* m)

{
    NetwPkt *msg = static_cast<NetwPkt *> (m);

    if(msg->getFinaldestAddr()==myNetwAddr)
    {

        dataReceived++;
        if (notBroadcasted(msg))
        {
            if(sequencerEnable == 1)
            {
                sequencer(msg);
            }
            else
            {
                nbDataPacketsReceived++;
                recuControl(msg);
                testDesequ(msg);
                delayTest(msg);
                nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
                msg->setSrcAddr(msg->getInitialsrcAddr());
                sendUp( decapsMsg(msg) );
            }
        }
        else
            delete msg;
    }
    else
    {
        if( LAddress::isL3Broadcast(msg->getDestAddr()) )
        {
            dataReceived++;
            if( msg->getTtl() > 1 )
            {
                NetwPkt *dMsg;
                EV <<" data msg BROADCAST! ttl = "<<msg->getTtl()
                                                                                                                                                                                                                                                                                                <<" > 1 -> rebroadcast msg \n";
                msg->setTtl( msg->getTtl()-1 );
                dMsg = static_cast<NetwPkt*>(msg->dup());
                if(optiEnable==0)
                {
                    forwardControl(dMsg);
                    setDownControlInfo(dMsg, LAddress::L2BROADCAST);
                    sendDown( dMsg );
                    nbDataPacketsForwarded++;
                }
                else
                    multiSend(dMsg);
                delete(msg);
            }
            else
            {
                nbTTL++;
                delete msg;
            }
        }
        else
            delete msg;
    }
}

bool RoutGeo5::notBroadcasted(NetwPkt* msg) {

    cBroadcastList::iterator it;

    //serach the broadcast list of outdated entries and delete them
    for (it = bcMsgs.begin(); it != bcMsgs.end(); it++) {
        /*
            if (it->delTime < simTime()) {
                bcMsgs.erase(it);
                it--;
            }
         */
        //message was already broadcasted
        if ((it->srcAddr == msg->getInitialsrcAddr()) && (it->seqNum
                == msg->getSeqNum())) {
            // update entry
            it->delTime = simTime() + bcDelTime;
            return false;
        }
    }
    /*
        //delete oldest entry if max size is reached
        if (bcMsgs.size() >= bcMaxEntries) {
            EV<<"bcMsgs is full, delete oldest entry\n";
            bcMsgs.pop_front();
        }
     */
    /*
    switch (msg->getInitialsrcAddr())
    {
    case 0 :
        if(msg->getSeqNum() < Deseqence0)
        {
            nbDesquence++;
        }
        Deseqence0=msg->getSeqNum();
        break;
    case 2 :
        if(msg->getSeqNum() < Deseqence2)
        {
            nbDesquence++;
        }
        Deseqence2=msg->getSeqNum();
        break;
    case 3 :
        if(msg->getSeqNum() < Deseqence3)
        {
            nbDesquence++;
        }
        Deseqence3=msg->getSeqNum();
        break;
    case 4 :
        if(msg->getSeqNum() < Deseqence4)
        {
            nbDesquence++;
        }
        Deseqence4=msg->getSeqNum();
        break;
    case 5 :
        if(msg->getSeqNum() < Deseqence5)
        {
            nbDesquence++;
        }
        Deseqence5=msg->getSeqNum();
        break;
    case 6 :
        if(msg->getSeqNum() < Deseqence6)
        {
            nbDesquence++;
        }
        Deseqence6=msg->getSeqNum();
        break;
    default:
        break;
    }
     */
    bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getInitialsrcAddr(), simTime() +bcDelTime));
    return true;
}

void RoutGeo5::sequencer(NetwPkt* msg)
{
    SequencerQueue::iterator it;
    NetwPkt* pkt;
    int k = 0;
    switch (msg->getInitialsrcAddr())
    {
    case 0 :
        if(msg->getSeqNum() < countNext0)
        {
            delete msg;
        }
        else if (msg->getSeqNum() > countNext0)
        {
            for ( it = sequencerQueue0.begin(); it != sequencerQueue0.end(); it++)
            {
                pkt =  *it;
                if (pkt->getSeqNum() > msg->getSeqNum())
                {
                    sequencerQueue0.insert (it, msg);
                    k = 1;
                    break;
                }
            }
            if (k ==0)
            {
                sequencerQueue0.push_back(msg);
            }
            if (sequencerQueue0.size() == 1)
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer0);
            }
        }
        else if (msg->getSeqNum() == countNext0)
        {
            nbDataPacketsReceived++;
            recuControl(msg);
            testDesequ(msg);
            delayTest(msg);
            countNext0++;
            nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
            msg->setSrcAddr(msg->getInitialsrcAddr());
            sendUp( decapsMsg(msg) );
            cancelEvent(sequencerTimer0);
            if (!sequencerQueue0.empty())
            {
                while (!sequencerQueue0.empty())
                {
                    pkt = sequencerQueue0.front()->dup();
                    if (pkt->getSeqNum() == countNext0)
                    {
                        nbDataPacketsReceived++;
                        sequencerQueue0.pop_front();
                        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                        recuControl(pkt);
                        testDesequ(pkt);
                        delayTest(pkt);
                        pkt->setSrcAddr(pkt->getInitialsrcAddr());
                        sendUp(decapsMsg(pkt->dup()));
                        countNext0++;
                    }
                    else
                        break;
                }
                if (!sequencerQueue0.empty())
                {
                    scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer0);
                }
            }
        }
        break;
    case 2 :
        if(msg->getSeqNum() < countNext2)
        {
            delete msg;
        }
        else if (msg->getSeqNum() > countNext2)
        {
            for ( it = sequencerQueue2.begin(); it != sequencerQueue2.end(); it++)
            {
                pkt =  *it;
                if (pkt->getSeqNum() > msg->getSeqNum())
                {
                    sequencerQueue2.insert (it, msg);
                    k = 1;
                    break;
                }
            }
            if (k ==0)
            {
                sequencerQueue2.push_back(msg);
            }
            if (sequencerQueue2.size() == 1)
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer2);
            }
        }
        else if (msg->getSeqNum() == countNext2)
        {
            nbDataPacketsReceived++;
            countNext2++;
            recuControl(msg);
            testDesequ(msg);
            delayTest(msg);
            nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
            msg->setSrcAddr(msg->getInitialsrcAddr());
            sendUp( decapsMsg(msg) );
            cancelEvent(sequencerTimer2);
            if (!sequencerQueue2.empty())
            {
                while (!sequencerQueue2.empty())
                {
                    pkt = sequencerQueue2.front()->dup();
                    if (pkt->getSeqNum() ==countNext2 )
                    {
                        nbDataPacketsReceived++;
                        sequencerQueue2.pop_front();
                        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                        recuControl(pkt);
                        testDesequ(pkt);
                        delayTest(pkt);
                        pkt->setSrcAddr(pkt->getInitialsrcAddr());
                        sendUp(decapsMsg(pkt->dup()));
                        countNext2++;
                    }
                    else
                        break;
                }
                if (!sequencerQueue2.empty())
                {
                    scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer2);
                }
            }
        }
        break;
    case 3 :
        if(msg->getSeqNum() < countNext3)
        {
            delete msg;
        }
        else if (msg->getSeqNum() > countNext3)
        {
            for ( it = sequencerQueue3.begin(); it != sequencerQueue3.end(); it++)
            {
                pkt =  *it;
                if (pkt->getSeqNum() > msg->getSeqNum())
                {
                    sequencerQueue3.insert (it, msg);
                    k = 1;
                    break;
                }
            }
            if (k ==0 )
            {
                sequencerQueue3.push_back(msg);
            }
            if (sequencerQueue3.size() == 1)
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer3);
            }
        }
        else if (msg->getSeqNum() == countNext3)
        {
            nbDataPacketsReceived++;
            countNext3++;
            recuControl(msg);
            testDesequ(msg);
            delayTest(msg);
            nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
            msg->setSrcAddr(msg->getInitialsrcAddr());
            sendUp(decapsMsg(msg) );
            cancelEvent(sequencerTimer3);
            if (!sequencerQueue3.empty())
            {
                while (!sequencerQueue3.empty())
                {
                    pkt = sequencerQueue3.front()->dup();
                    if (pkt->getSeqNum() == countNext3)
                    {
                        nbDataPacketsReceived++;
                        sequencerQueue3.pop_front();
                        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                        recuControl(pkt);
                        testDesequ(pkt);
                        delayTest(pkt);
                        pkt->setSrcAddr(pkt->getInitialsrcAddr());
                        sendUp(decapsMsg(pkt->dup()));
                        countNext3++;
                    }
                    else
                        break;
                }
                if (!sequencerQueue3.empty())
                {
                    scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer3);
                }
            }
        }
        break;
    case 4 :
        if(msg->getSeqNum() < countNext4)
        {
            delete msg;
        }
        else if (msg->getSeqNum() > countNext4)
        {
            for ( it = sequencerQueue4.begin(); it != sequencerQueue4.end(); it++)
            {
                pkt =  *it;
                if (pkt->getSeqNum() > msg->getSeqNum())
                {
                    sequencerQueue4.insert (it, msg);
                    k = 1;
                    break;
                }
            }
            if (k == 0)
            {
                sequencerQueue4.push_back(msg);
            }
            if (sequencerQueue4.size() == 1)
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer4);
            }
        }
        else if (msg->getSeqNum() == countNext4)
        {
            nbDataPacketsReceived++;
            countNext4++;
            recuControl(msg);
            testDesequ(msg);
            delayTest(msg);
            nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
            msg->setSrcAddr(msg->getInitialsrcAddr());
            sendUp( decapsMsg(msg) );
            cancelEvent(sequencerTimer4);
            if (!sequencerQueue4.empty())
            {
                while (!sequencerQueue4.empty())
                {
                    sequencerQueue4.pop_front();
                    if (pkt->getSeqNum() == countNext4)
                    {
                        nbDataPacketsReceived++;
                        pkt = sequencerQueue4.front()->dup();
                        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                        recuControl(pkt);
                        testDesequ(pkt);
                        delayTest(pkt);
                        pkt->setSrcAddr(pkt->getInitialsrcAddr());
                        sendUp(decapsMsg(pkt->dup()));
                        countNext4++;
                    }
                    else
                        break;
                }
                if (!sequencerQueue4.empty())
                {
                    scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer4);
                }
            }
        }
        break;
    case 5 :
        if(msg->getSeqNum() < countNext5)
        {
            delete msg;
        }
        else if (msg->getSeqNum() > countNext5)
        {
            for ( it = sequencerQueue5.begin(); it != sequencerQueue5.end(); it++)
            {
                pkt =  *it;
                if (pkt->getSeqNum() > msg->getSeqNum())
                {
                    sequencerQueue5.insert (it, msg);
                    k = 1;
                    break;
                }
            }
            if (k == 0)
            {
                sequencerQueue5.push_back(msg);
            }
            if (sequencerQueue5.size() == 1)
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer5);
            }
        }
        else if (msg->getSeqNum() == countNext5)
        {
            nbDataPacketsReceived++;
            countNext5++;
            recuControl(msg);
            testDesequ(msg);
            delayTest(msg);
            nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
            msg->setSrcAddr(msg->getInitialsrcAddr());
            sendUp( decapsMsg(msg) );
            cancelEvent(sequencerTimer5);
            if (!sequencerQueue5.empty())
            {
                while (!sequencerQueue5.empty())
                {
                    pkt = sequencerQueue5.front()->dup();
                    if (pkt->getSeqNum() == countNext5)
                    {
                        nbDataPacketsReceived++;
                        sequencerQueue5.pop_front();
                        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                        recuControl(pkt);
                        testDesequ(pkt);
                        delayTest(pkt);
                        pkt->setSrcAddr(pkt->getInitialsrcAddr());
                        sendUp(decapsMsg(pkt->dup()));
                        countNext5++;
                    }
                    else
                        break;
                }
                if (!sequencerQueue5.empty())
                {
                    scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer5);
                }
            }
        }
        break;
    case 6 :
        if(msg->getSeqNum() < countNext6)
        {
            delete msg;
        }
        else if (msg->getSeqNum() > countNext6)
        {
            for ( it = sequencerQueue6.begin(); it != sequencerQueue6.end(); it++)
            {
                pkt =  *it;
                if (pkt->getSeqNum() > msg->getSeqNum())
                {
                    sequencerQueue6.insert (it, msg);
                    k = 1;
                    break;
                }
            }
            if (k == 0)
            {
                sequencerQueue6.push_back(msg);
            }
            if (sequencerQueue6.size() == 1)
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer6);
            }
        }
        else if (msg->getSeqNum() == countNext6)
        {
            nbDataPacketsReceived++;
            countNext6++;
            recuControl(msg);
            testDesequ(msg);
            delayTest(msg);
            nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
            msg->setSrcAddr(msg->getInitialsrcAddr());
            sendUp( decapsMsg(msg) );
            cancelEvent(sequencerTimer6);
            if (!sequencerQueue6.empty())
            {
                while (!sequencerQueue6.empty())
                {
                    pkt = sequencerQueue6.front()->dup();
                    if (pkt->getSeqNum() == countNext6)
                    {
                        nbDataPacketsReceived++;
                        sequencerQueue6.pop_front();
                        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                        recuControl(pkt);
                        testDesequ(pkt);
                        delayTest(pkt);
                        pkt->setSrcAddr(pkt->getInitialsrcAddr());
                        sendUp(decapsMsg(pkt->dup()));
                        countNext6++;
                    }
                    else
                        break;
                }
                if (!sequencerQueue6.empty())
                {
                    scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer6);
                }
            }
        }
        break;
    default:
        break;
    }
}

void RoutGeo5::forwardControl(NetwPkt *msg)
{
    int x=msg->getInitialsrcAddr();
    switch (x)
    {
    case 0 :
        nbForwarded0++;
        break;
    case 2 :
        nbForwarded2++;
        break;
    case 3 :
        nbForwarded3++;
        break;
    case 4 :
        nbForwarded4++;
        break;
    case 5 :
        nbForwarded5++;
        break;
    case 6 :
        nbForwarded6++;
        break;
    default:
        break;
    }
}

void RoutGeo5::recuControl(NetwPkt *msg)
{
    int x=msg->getInitialsrcAddr();
    switch (x)
    {
    case 0 :
        nbRecu0++;
        break;
    case 2 :
        nbRecu2++;
        break;
    case 3 :
        nbRecu3++;
        break;
    case 4 :
        nbRecu4++;
        break;
    case 5 :
        nbRecu5++;
        break;
    case 6 :
        nbRecu6++;
        break;
    default:
        break;
    }
}

void RoutGeo5::multiSend(NetwPkt* msg)
{
    //BGW
    NetwPkt* msgCopy;
    int nbT=0;
    int ls=-1;
    LogNormalShadowing LS1;
    LS1.sender=(int)(myNetwAddr);
    LS1.posture=posture;
    for (int i=0;i<7;i++)
    {
        if(i != myNetwAddr)
        {
            LS1.receiver=i;
            if(LS1.nbForTrying()<ls || ls ==-1)
            {
                ls = LS1.nbForTrying();
            }
        }
        else
            continue;
    }
    nbT = ls;


    for (int i =0; i<nbT; i++)
    {
        msgCopy =msg->dup();
        if(msgCopy->getInitialsrcAddr()!=myNetwAddr)
        {
            nbDataPacketsForwarded++;
            forwardControl(msgCopy);
        }
        else
            nbDataPacketsSent++;
        setDownControlInfo(msgCopy, LAddress::L2BROADCAST);
        sendDown(msgCopy);
    }
}


NetwPkt* RoutGeo5::encapsMsg(cPacket *appPkt) {
    LAddress::L2Type macAddr;
    LAddress::L3Type netwAddr;

    EV<<"in encaps...\n";

    NetwPkt *pkt = new NetwPkt(appPkt->getName(), appPkt->getKind());
    pkt->setBitLength(headerLength);

    cObject* cInfo = appPkt->removeControlInfo();

    if(cInfo == NULL){
        EV << "warning: Application layer did not specifiy a destination L3 address\n"
                << "\tusing broadcast address instead\n";
        netwAddr = LAddress::L3BROADCAST;
    } else {
        EV <<"CInfo removed, netw addr="<< NetwControlInfo::getAddressFromControlInfo( cInfo ) <<endl;
        netwAddr = NetwControlInfo::getAddressFromControlInfo( cInfo );
        delete cInfo;
    }

    pkt->setSrcAddr(myNetwAddr);
    pkt->setDestAddr(netwAddr);
    EV << " netw "<< myNetwAddr << " sending packet" <<endl;

    EV << "sendDown: nHop=L3BROADCAST -> message has to be broadcasted"
            << " -> set destMac=L2BROADCAST" << endl;
    macAddr = LAddress::L2BROADCAST;

    setDownControlInfo(pkt, macAddr);

    //encapsulate the application packet
    pkt->encapsulate(appPkt);
    EV <<" pkt encapsulated\n";
    return pkt;
}

void RoutGeo5::handleSelfMsg(cMessage *m)
{
    SequencerQueue::iterator it;
    NetwPkt* pkt;
    switch (m->getKind())
    {
    case SequencerTimer0 :
        nbDataPacketsReceived++;
        pkt = sequencerQueue0.front()->dup();
        sequencerQueue0.pop_front();
        countNext0 = pkt->getSeqNum();
        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
        recuControl(pkt);
        testDesequ(pkt);
        delayTest(pkt);
        pkt->setSrcAddr(pkt->getInitialsrcAddr());
        sendUp(decapsMsg(pkt->dup()));
        if (!sequencerQueue0.empty())
        {
            while (!sequencerQueue0.empty())
            {
                pkt = sequencerQueue0.front()->dup();
                if (pkt->getSeqNum() == countNext0)
                {
                    nbDataPacketsReceived++;
                    sequencerQueue0.pop_front();
                    pkt->setSrcAddr(pkt->getInitialsrcAddr());
                    recuControl(pkt);
                    testDesequ(pkt);
                    delayTest(pkt);
                    nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                    sendUp(decapsMsg(pkt->dup()));
                    countNext0++;
                }
                else
                    break;
            }
            if (!sequencerQueue0.empty())
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer0);
            }
        }
        break;
    case SequencerTimer2 :
        nbDataPacketsReceived++;
        pkt = sequencerQueue2.front()->dup();
        sequencerQueue2.pop_front();
        countNext2 = pkt->getSeqNum();
        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
        recuControl(pkt);
        testDesequ(pkt);
        delayTest(pkt);
        pkt->setSrcAddr(pkt->getInitialsrcAddr());
        sendUp(decapsMsg(pkt->dup()));
        if (!sequencerQueue2.empty())
        {
            while (!sequencerQueue2.empty())
            {
                pkt = sequencerQueue2.front()->dup();
                if (pkt->getSeqNum() ==countNext2 )
                {
                    nbDataPacketsReceived++;
                    countNext2++;
                    sequencerQueue2.pop_front();
                    nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                    recuControl(pkt);
                    testDesequ(pkt);
                    delayTest(pkt);
                    pkt->setSrcAddr(pkt->getInitialsrcAddr());
                    sendUp(decapsMsg(pkt->dup()));
                }
                else
                    break;
            }
            if (!sequencerQueue2.empty())
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer2);
            }
        }
        break;
    case SequencerTimer3 :
        nbDataPacketsReceived++;
        pkt = sequencerQueue3.front()->dup();
        sequencerQueue3.pop_front();
        countNext3 = pkt->getSeqNum();
        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
        recuControl(pkt);
        testDesequ(pkt);
        delayTest(pkt);
        pkt->setSrcAddr(pkt->getInitialsrcAddr());
        sendUp(decapsMsg(pkt->dup()));
        if (!sequencerQueue3.empty())
        {
            while (!sequencerQueue3.empty())
            {
                pkt = sequencerQueue3.front()->dup();
                if (pkt->getSeqNum() == countNext3)
                {
                    nbDataPacketsReceived++;
                    countNext3++;
                    sequencerQueue3.pop_front();
                    nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                    recuControl(pkt);
                    testDesequ(pkt);
                    delayTest(pkt);
                    pkt->setSrcAddr(pkt->getInitialsrcAddr());
                    sendUp(decapsMsg(pkt->dup()));
                }
                else
                    break;
            }
            if (!sequencerQueue3.empty())
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer3);
            }
        }
        break;
    case SequencerTimer4 :
        nbDataPacketsReceived++;
        pkt = sequencerQueue4.front()->dup();
        countNext4 = pkt->getSeqNum();
        sequencerQueue4.pop_front();
        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
        recuControl(pkt);
        testDesequ(pkt);
        delayTest(pkt);
        pkt->setSrcAddr(pkt->getInitialsrcAddr());
        sendUp(decapsMsg(pkt->dup()));
        if (!sequencerQueue4.empty())
        {
            while (!sequencerQueue4.empty())
            {
                pkt = sequencerQueue4.front()->dup();
                if(pkt->getSeqNum() == countNext4)
                {
                    nbDataPacketsReceived++;
                    countNext4++;
                    sequencerQueue4.pop_front();
                    recuControl(pkt);
                    testDesequ(pkt);
                    delayTest(pkt);
                    nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                    pkt->setSrcAddr(pkt->getInitialsrcAddr());
                    sendUp(decapsMsg(pkt->dup()));
                }
                else
                    break;
            }
            if (!sequencerQueue4.empty())
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer4);
            }
        }
        break;
    case SequencerTimer5 :
        nbDataPacketsReceived++;
        pkt = sequencerQueue5.front()->dup();
        sequencerQueue5.pop_front();
        countNext5 = pkt->getSeqNum();
        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
        recuControl(pkt);
        testDesequ(pkt);
        delayTest(pkt);
        pkt->setSrcAddr(pkt->getInitialsrcAddr());
        sendUp(decapsMsg(pkt->dup()));
        if (!sequencerQueue5.empty())
        {
            while (!sequencerQueue5.empty())
            {
                pkt = sequencerQueue5.front()->dup();
                if (pkt->getSeqNum() == countNext5)
                {
                    nbDataPacketsReceived++;
                    countNext5++;
                    sequencerQueue5.pop_front();
                    nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                    recuControl(pkt);
                    testDesequ(pkt);
                    delayTest(pkt);
                    pkt->setSrcAddr(pkt->getInitialsrcAddr());
                    sendUp(decapsMsg(pkt->dup()));
                }
                else
                    break;
            }
            if (!sequencerQueue5.empty())
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer5);
            }
        }
        break;
    case SequencerTimer6 :
        nbDataPacketsReceived++;
        pkt = sequencerQueue6.front()->dup();
        sequencerQueue6.pop_front();
        countNext6 = pkt->getSeqNum();
        nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
        recuControl(pkt);
        testDesequ(pkt);
        delayTest(pkt);
        pkt->setSrcAddr(pkt->getInitialsrcAddr());
        sendUp(decapsMsg(pkt->dup()));
        if (!sequencerQueue6.empty())
        {
            while (!sequencerQueue5.empty())
            {
                pkt = sequencerQueue6.front()->dup();
                if (pkt->getSeqNum() == countNext6)
                {
                    nbDataPacketsReceived++;
                    countNext6++;
                    sequencerQueue6.pop_front();
                    nbHops = nbHops + (defaultTtl + 1 - pkt->getTtl());
                    recuControl(pkt);
                    testDesequ(pkt);
                    delayTest(pkt);
                    pkt->setSrcAddr(pkt->getInitialsrcAddr());
                    sendUp(decapsMsg(pkt->dup()));
                }
                else
                    break;
            }
            if (!sequencerQueue6.empty())
            {
                scheduleAt(simTime()+ timerOutofSequencer, sequencerTimer6);
            }
        }
        break;
    default:
        break;
    }
}
void RoutGeo5::testDesequ(NetwPkt *msg)
{
    switch (msg->getInitialsrcAddr())
    {
    case 0 :
        if(msg->getSeqNum() < Deseqence0)
        {
            nbDesquence++;
        }
        Deseqence0=msg->getSeqNum();
        break;
    case 2 :
        if(msg->getSeqNum() < Deseqence2)
        {
            nbDesquence++;
        }
        Deseqence2=msg->getSeqNum();
        break;
    case 3 :
        if(msg->getSeqNum() < Deseqence3)
        {
            nbDesquence++;
        }
        Deseqence3=msg->getSeqNum();
        break;
    case 4 :
        if(msg->getSeqNum() < Deseqence4)
        {
            nbDesquence++;
        }
        Deseqence4=msg->getSeqNum();
        break;
    case 5 :
        if(msg->getSeqNum() < Deseqence5)
        {
            nbDesquence++;
        }
        Deseqence5=msg->getSeqNum();
        break;
    case 6 :
        if(msg->getSeqNum() < Deseqence6)
        {
            nbDesquence++;
        }
        Deseqence6=msg->getSeqNum();
        break;
    default:
        break;
    }
}

void RoutGeo5::delayTest(NetwPkt *msg)
{
    delay = delay + (simTime() -  msg->getCreationTime());
}
