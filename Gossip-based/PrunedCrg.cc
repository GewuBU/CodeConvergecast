/* -*- mode:c++ -*- ********************************************************
 * file:        Flood.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: a simple flooding protocol
 *              the user can decide whether to use plain flooding or not
 **************************************************************************/

#include "RoutGeo6.h"

#include <cassert>

#include "NetwPkt_m.h"
#include "NetwControlInfo.h"
#include "LogNormalShadowing.h"

using std::endl;

Define_Module(RoutGeo6);

/**
 * Reads all parameters from the ini file. If a parameter is not
 * specified in the ini file a default value will be set.
 **/
void RoutGeo6::initialize(int stage) {
    BaseNetwLayer::initialize(stage);

    if (stage == 0) {
        //initialize seqence number to 0

        posture=par("posture");
        optiEnable=par("optiEnable");
        seqNum = 0;
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
        nbDataPacketsForwarded = 0;
        dataReceived=0;
        Deseqence0=0;
        Deseqence2=0;
        Deseqence3=0;
        Deseqence4=0;
        Deseqence5=0;
        Deseqence6=0;
        nbDesquence=0;
        nbHops = 0;
        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 6;
        hasPar("ProbabilisticFlooding") ? RoutGeo6ing = par("RoutGeo6ing")
                : RoutGeo6ing = true;

        EV<< "defaultTtl = " << defaultTtl
                << " RoutGeo6ing = " << RoutGeo6ing << endl;


        //these parameters are only needed for plain flooding
        hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;

        hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 180.0;
        EV <<"bcMaxEntries = "<<bcMaxEntries
                <<" bcDelTime = "<<bcDelTime<<endl;

    }
}

void RoutGeo6::finish() {

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
    recordScalar("nbDesquence", nbDesquence);
    if (nbDataPacketsReceived > 0) {
        recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);
    } else {
        recordScalar("meanNbHops", 0);
    }

}

void RoutGeo6::handleUpperMsg(cMessage* m) {

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

void RoutGeo6::handleLowerMsg(cMessage* m) {
    NetwPkt *msg = static_cast<NetwPkt *> (m);
    //int K=1;


    if (msg->getFinaldestAddr()==myNetwAddr)
    {
        dataReceived++;

        //msg not broadcastes yet
        if (notBroadcasted(msg))
        {
            recuControl(msg);
            nbDataPacketsReceived++;
            // message has to be forwarded to upper layer
            nbHops = nbHops + (defaultTtl + 1 - msg->getTtl());
            msg->setSrcAddr(msg->getInitialsrcAddr());
            sendUp( decapsMsg(msg) );

        }
        else
            delete msg;
    }
    else
    {
        dataReceived++;
        if(( LAddress::isL3Broadcast(msg->getDestAddr()) )||(msg->getDestAddr() == myNetwAddr))
        {

            //check ttl and rebroadcast
            if( msg->getTtl() > 1 )
            {
                msg->setTtl( msg->getTtl()-1 );

                int r;
                r = rand() % 7 + 0;

                NetwPkt *dMsg;
                EV<<" le noeud choisi est "<<r<<endl;
                msg->setDestAddr(r);
                msg->setSrcAddr(myNetwAddr);
                dMsg = static_cast<NetwPkt*>(msg->dup());
                //msg->removeControlInfo();
                if(optiEnable==0)
                {
                    forwardControl(dMsg);
                    setDownControlInfo(dMsg, LAddress::L2BROADCAST);
                    EV<<" msgDestAddr "<<dMsg->getDestAddr()<<endl;
                    sendDown(dMsg);
                    nbDataPacketsForwarded++;
                }
                else
                    multiSend(dMsg);
                delete(msg);
            }

            // fin ttl > 1

            else
            {
                EV <<" max hops reached (ttl = "<<msg->getTtl()<<") -> delete\n";
                delete msg;
            }


        }
        //not for me -> rebroadcast
        else
        {
            EV<<" ni broadcast msg ni for me "<<endl;
            delete msg;
        }
    }
}

bool RoutGeo6::notBroadcasted(NetwPkt* msg) {

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

void RoutGeo6::forwardControl(NetwPkt *msg)
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

void RoutGeo6::recuControl(NetwPkt *msg)
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

void RoutGeo6::multiSend(NetwPkt* msg)
{
    //BGW
    NetwPkt* msgCopy;
    int nbT=0;
    int ls =-1;
    LogNormalShadowing LS1;
    LS1.sender=(int)(myNetwAddr);
    LS1.posture=posture;

    if(msg->getInitialsrcAddr()==myNetwAddr)
    {
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
    }
    else
    {
        LS1.receiver=msg->getDestAddr();
        nbT = LS1.nbForTrying();
    }

    if(nbT>30) nbT=30;
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

NetwPkt* RoutGeo6::encapsMsg(cPacket *appPkt) {
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
