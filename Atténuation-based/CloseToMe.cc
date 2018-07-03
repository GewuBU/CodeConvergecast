#include "RoutGeo3.h"

#include <cassert>

#include "NetwPkt_m.h"
#include "NetwControlInfo.h"
#include "LogNormalShadowing.h"
#include "Mapping.h"
#include "MiximAirFrame_m.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;


using std::endl;

Define_Module(RoutGeo3);

/**
 * Reads all parameters from the ini file. If a parameter is not
 * specified in the ini file a default value will be set.
 **/
void RoutGeo3::initialize(int stage) {
    BaseNetwLayer::initialize(stage);

    if (stage == 0)
    {
        //initialize seqence number to 0
        maxNetQueueSize=par("maxNetQueueSize");
        posture = par("posture");
        optiEnable=par("optiEnable");
        seqNum = 0;
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
        nbDataPacketsForwarded = 0;
        nbHops = 0;
        dataReceived=0;
        nbAttREPsent=0;
        nbAttREPreceived=0;
        nbAttREQsent=0;
        nbAttREQreceived=0;
        posBestAtt=0;
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

        for (int j=0;j<7;j++)
        {
            BestAtt[0][j]=-1.0;
            BestAtt[1][j]=0.0;
            BestAtt[2][j]=0.0;
        }

        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 5;
        EV<< "defaultTtl = " << defaultTtl<< endl;


        //these parameters are only needed for plain flooding
        hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;

        hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 180.0;
        EV <<"bcMaxEntries = "<<bcMaxEntries
                <<" bcDelTime = "<<bcDelTime<<endl;

        repTimer = new NetwPkt("REQTimer", REQTimer);
    }
}

void RoutGeo3::finish() {

    bcMsgs.clear();
    cOwnedObject *Del=NULL;
    int OwnedSize=this->defaultListSize();
    for(int i=0;i<OwnedSize;i++){
        Del=this->defaultListGet(0);
        this->drop(Del);
        delete Del;
    }

    recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
    recordScalar("nbDataPacketsSent", nbDataPacketsSent);
    recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
    recordScalar("dataReceived", dataReceived);
    recordScalar("nbAttREQsent",nbAttREQsent);
    recordScalar("nbAttREQreceived",nbAttREQreceived);
    recordScalar("nbAttREPsent",nbAttREPsent);
    recordScalar("nbAttREPreceived",nbAttREPreceived);
    recordScalar("nbDesquence", nbDesquence);

    if (nbDataPacketsReceived > 0) {
        recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);
    } else {
        recordScalar("meanNbHops", 0);
    }
}

void RoutGeo3::handleUpperMsg(cMessage* m) {


    assert(dynamic_cast<cPacket*> (m));
    NetwPkt *msg = encapsMsg(static_cast<cPacket*> (m));

    msg->setSeqNum(seqNum);
    seqNum++;
    msg->setTtl(defaultTtl);
    msg->setInitialsrcAddr(myNetwAddr);
    msg->setSrcAddr(myNetwAddr);
    msg->setFinaldestAddr(1);
    //msg->setDestAddr(NextHopAddr);

    //BGW
    if(netQueue.size()<maxNetQueueSize)
    {
        netQueue.push_back(static_cast<  NetwPkt*>(msg));
    }
    else
    {
        netQueue.pop_front();
        //netQueue.erase(netQueue.front());
        netQueue.push_back(static_cast<  NetwPkt*>(msg));
        //cancelEvent(repTimer);
    }

    if(netQueue.size()==1)
    {
        NetwPkt* Pmsg= new NetwPkt("AttenuationMsg", AttenuationREQ);
        Pmsg->setByteLength(headerLength);
        Pmsg->setSrcAddr(myNetwAddr);
        Pmsg->setDestAddr(LAddress::L3BROADCAST);
        setDownControlInfo(Pmsg, LAddress::L2BROADCAST);
        sendDown(Pmsg);
        nbAttREQsent++;
        scheduleAt(simTime()+0.02, repTimer);
    }
    EV<<"Fin handleUpperMessage "<<endl;

}


void RoutGeo3::handleLowerMsg(cMessage* m)

{

    NetwPkt *msg = static_cast<NetwPkt *> (m);
    double AttValueS;
    double AttValueN;


    if (msg->getKind()== AttenuationREQ)
    {

        nbAttREQreceived++;
        LogNormalShadowing LS;
        LS.sender=1;
        LS.receiver=(int)myNetwAddr;
        LS.posture=posture;
        AttValueS=LS.randomLogNormalGain();

        LS.sender=msg->getSrcAddr();
        AttValueN=LS.randomLogNormalGain();


        EV<<" Je suis le noeud "<<myNetwAddr<<" j'ai reçu un AttenuationREQ de " << msg->getSrcAddr() <<endl;

        // Je réponds à l'atténuation request
        NetwPkt* pkt = new NetwPkt("AttenuationREP", AttenuationREP);
        pkt->setByteLength(headerLength);
        pkt->setSrcAddr(myNetwAddr);
        pkt->setInitialsrcAddr(msg->getInitialsrcAddr());
        pkt->setDestAddr(msg->getSrcAddr());
        pkt->setAttS(AttValueS);
        pkt->setAttN(AttValueN);
        setDownControlInfo(pkt, LAddress::L2BROADCAST);
        sendDown(pkt);
        nbAttREPsent++;
        delete msg;

    }


    else if (msg->getKind()== AttenuationREP && msg->getInitialsrcAddr()!=myNetwAddr)
    {
        nbAttREPreceived++;
        int ID=-2;
        if((msg->getDestAddr()==myNetwAddr) )
        {
            for (int h=0; h<7;h++)
            {
                if (BestAtt[0][h]==msg->getSrcAddr())
                {
                    ID=h;
                }
            }
            if(ID!=-2)
            {
                EV<<" cette réponse de " <<msg->getSrcAddr()<<" a moi " <<myNetwAddr<<endl;
                BestAtt[1][ID]=msg->getAttS();
                BestAtt[2][ID]=msg->getAttN();
                delete msg;

            }
            else
            {
                EV<<" cette réponse de " <<msg->getSrcAddr()<<" a moi " <<myNetwAddr<<endl;
                BestAtt[0][posBestAtt]=msg->getSrcAddr();
                BestAtt[1][posBestAtt]=msg->getAttS();
                BestAtt[2][posBestAtt]=msg->getAttN();
                posBestAtt++;
                delete msg;
            }
        }


        else
        {
            EV<<"me  "<<myNetwAddr<<"  AttenuationREP for "<<msg->getDestAddr()<<endl;
            delete msg;
        }

    }

    else
    {
        dataReceived++;
        if (msg->getFinaldestAddr()==myNetwAddr)
        {

            if(notBroadcasted(msg))
            {

                nbDataPacketsReceived++;
                recuControl(msg);
                EV<<"je suis "<<myNetwAddr<< " et j'ai reçu des données de "<< msg->getInitialsrcAddr()<< " à travers "<<msg->getSrcAddr()<<endl;
                msg->setSrcAddr(msg->getInitialsrcAddr());
                sendUp(msg);
                //sendUp(decapsMsg(msg) );
            }
            else
                delete msg;
        }

        else
        {
            if(msg->getDestAddr()==myNetwAddr)
            {
                if (msg->getTtl()>1)
                {
                    msg->setTtl(msg->getTtl()-1);
                    //BGW
                    if(netQueue.size()<maxNetQueueSize)
                    {
                        netQueue.push_back(static_cast<  NetwPkt*>(msg));
                    }
                    else
                    {
                        netQueue.pop_front();
                        //netQueue.erase(netQueue.front());
                        netQueue.push_back(static_cast<  NetwPkt*>(msg));
                        //cancelEvent(repTimer);
                    }

                    if(netQueue.size()==1)
                    {
                        // demander la position des noeuds
                        NetwPkt* Pmsg= new NetwPkt("AttenuationMsg", AttenuationREQ);
                        Pmsg->setByteLength(headerLength);
                        Pmsg->setSrcAddr(myNetwAddr);
                        Pmsg->setDestAddr(LAddress::L3BROADCAST);
                        setDownControlInfo(Pmsg, LAddress::L2BROADCAST);
                        sendDown(Pmsg);
                        nbAttREQsent++;
                        scheduleAt(simTime()+0.02, repTimer);
                    }
                }
                else
                    delete msg;
            }
            else
                delete msg;
        }
    }


}

bool RoutGeo3::notBroadcasted(NetwPkt* msg) {

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
    bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getInitialsrcAddr(), simTime() +bcDelTime));
    return true;
}

void RoutGeo3::forwardControl(NetwPkt *msg)
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

void RoutGeo3::recuControl(NetwPkt *msg)
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

void RoutGeo3::multiSend(NetwPkt* msg)
{
    //BGW
    NetwPkt* msgCopy;
    int nbT=0;
    LogNormalShadowing LS1, LS2;
    LS1.sender=(int)(myNetwAddr);
    LS1.receiver=msg->getDestAddr();
    LS1.posture=posture;
    nbT=LS1.nbForTrying();

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

NetwPkt* RoutGeo3::encapsMsg(cPacket *appPkt) {
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


void RoutGeo3::handleSelfMsg(cMessage *m)
{
    //NetwPkt *msg = static_cast<NetwPkt *> (m);
    int reREQ=0;

    EV<<"handle self message"<<endl;

    double p,q,v,r;
    int x=0;

    for(int k=0;k<7;k++)
    {
        if (BestAtt[1][k]!=0.0)
        {

            for(int j=0;j<7;j++)
            {
                if(BestAtt[1][j]!=0.0)
                {
                    if(BestAtt[1][j]>BestAtt[1][k] )
                    {
                        x++;
                    }
                }

            }
            if (x!=0)
            {
                p=BestAtt[1][x];
                q=BestAtt[0][x];
                r=BestAtt[2][x];

                BestAtt[1][x]=BestAtt[1][k];
                BestAtt[0][x]=BestAtt[0][k];
                BestAtt[2][x]=BestAtt[2][k];

                BestAtt[1][k]=p;
                BestAtt[0][k]=q;
                BestAtt[2][k]=r;
            }

        }

        x=0;
    }

    for (int i=0;i<7;i++)
    {

        EV <<"je suis  "<<BestAtt[0][i]<<" i égal à "<<i<<" Att égale à "<<BestAtt[1][i]<<endl;
    }

    /*
    if(BestAtt[0][0]!=-1.0)
    {
        while(!netQueue.empty())
        {
            if (BestAtt[2][0]>BestAtt[2][1])
            {

                EV<<"me "<<myNetwAddr<<"  NextHOP égal à "<<BestAtt[0][0]<<endl;
                NetwPkt *dMsg;
                dMsg = static_cast<NetwPkt*>(netQueue.front()->dup());
                dMsg->setSrcAddr(myNetwAddr);
                dMsg->setDestAddr((int)BestAtt[0][0]);
                if(dMsg->getInitialsrcAddr()!=myNetwAddr)
                {
                    nbDataPacketsForwarded++;
                    forwardControl(dMsg);
                }
                else
                    nbDataPacketsSent++;
                setDownControlInfo(dMsg, LAddress::L2BROADCAST);
                sendDown(dMsg);

                reREQ=1;
            }
            else
            {
                EV<<"me "<<myNetwAddr<<"  NextHOP égal à "<<BestAtt[0][1]<<endl;
                NetwPkt *dMsg;
                dMsg = static_cast<NetwPkt*>(netQueue.front()->dup());
                dMsg->setSrcAddr(myNetwAddr);
                dMsg->setDestAddr((int)BestAtt[0][1]);
                if(dMsg->getInitialsrcAddr()!=myNetwAddr)
                {
                    nbDataPacketsForwarded++;
                    forwardControl(dMsg);
                }
                else
                    nbDataPacketsSent++;
                setDownControlInfo(dMsg, LAddress::L2BROADCAST);
                sendDown(dMsg);

                reREQ=1;
            }
            netQueue.pop_front();
        }
    }
     */

    if(BestAtt[0][0]!=-1.0)
    {
        if (BestAtt[2][0]>BestAtt[2][1])
        {
            EV<<"me "<<myNetwAddr<<"  NextHOP égal à "<<BestAtt[0][0]<<endl;
            NetwPkt *dMsg;
            dMsg = static_cast<NetwPkt*>(netQueue.front()->dup());
            dMsg->setSrcAddr(myNetwAddr);
            dMsg->setDestAddr((int)BestAtt[0][0]);
            if(optiEnable==0)
            {
                if(dMsg->getInitialsrcAddr()!=myNetwAddr)
                {
                    nbDataPacketsForwarded++;
                    forwardControl(dMsg);
                }
                else
                    nbDataPacketsSent++;
                setDownControlInfo(dMsg, LAddress::L2BROADCAST);
                sendDown(dMsg);
            }
            else
                multiSend(dMsg);

            reREQ=1;
        }
        else
        {
            EV<<"me "<<myNetwAddr<<"  NextHOP égal à "<<BestAtt[0][1]<<endl;
            NetwPkt *dMsg;
            dMsg = static_cast<NetwPkt*>(netQueue.front()->dup());
            dMsg->setSrcAddr(myNetwAddr);
            dMsg->setDestAddr((int)BestAtt[0][1]);
            if(optiEnable==0)
            {
                if(dMsg->getInitialsrcAddr()!=myNetwAddr)
                {
                    nbDataPacketsForwarded++;
                    forwardControl(dMsg);
                }
                else
                    nbDataPacketsSent++;
                setDownControlInfo(dMsg, LAddress::L2BROADCAST);
                sendDown(dMsg);
            }
            else
                multiSend(dMsg);
            reREQ=1;
        }
        netQueue.pop_front();
        for (int j=0;j<7;j++)
        {
            BestAtt[0][j]=-1.0;
            BestAtt[1][j]=0.0;
            BestAtt[2][j]=0.0;
        }
        posBestAtt=0;
        if(!netQueue.empty())
        {
            EV<<"reREQ égal à 0, je n'est pas de NextHop"<<endl;
            NetwPkt* Pmsg= new NetwPkt("AttenuationMsg", AttenuationREQ);
            Pmsg->setByteLength(headerLength);
            Pmsg->setSrcAddr(myNetwAddr);
            Pmsg->setDestAddr(LAddress::L3BROADCAST);
            setDownControlInfo(Pmsg, LAddress::L2BROADCAST);
            sendDown(Pmsg);
            nbAttREQsent++;
            scheduleAt(simTime()+0.02, repTimer);
        }
    }

    if(reREQ==0)
    {
        EV<<"reREQ égal à 0, je n'est pas de NextHop"<<endl;
        NetwPkt* Pmsg= new NetwPkt("AttenuationMsg", AttenuationREQ);
        Pmsg->setByteLength(headerLength);
        Pmsg->setSrcAddr(myNetwAddr);
        Pmsg->setDestAddr(LAddress::L3BROADCAST);
        setDownControlInfo(Pmsg, LAddress::L2BROADCAST);
        sendDown(Pmsg);
        nbAttREQsent++;
        scheduleAt(simTime()+0.02, repTimer);
    }
}
