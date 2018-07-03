
#include "Orw.h"

#include <cassert>
#include <stdlib.h>
#include "NetwControlInfo.h"
#include "OrwPkt_m.h"
#include "SimTracer.h"
#include "LogNormalShadowing.h"

Define_Module(Orw);

void Orw::initialize(int stage)
{
    BaseNetwLayer::initialize(stage);

    if(stage == 1) {
        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 6;
        i=0;

        if(myNetwAddr!=1)
        {
            edc=-1;
        }
        else
        {
            edc=0;
        }
        contBeacon0=0;
        contBeacon1=0;
        contBeacon2=0;
        contBeacon3=0;
        contBeacon4=0;
        contBeacon5=0;
        contBeacon6=0;
        seqBeacon0=0;
        seqBeacon1=0;
        seqBeacon2=0;
        seqBeacon3=0;
        seqBeacon4=0;
        seqBeacon5=0;
        seqBeacon6=0;

        Qb0=0;
        Qb1=0;
        Qb2=0;
        Qb3=0;
        Qb4=0;
        Qb5=0;
        Qb6=0;

        for (i=0; i<2; i++){
            for(int j =0; j<7;j++){
                edc_v[i][j]=-1;
            }
        }

        maxTry = par("maxTry");
        maxNetQueueSize=par("maxNetQueueSize");

        floodSeqNumber = 0;
        beaconNumber = 0;
        nbDataPacketsForwarded = 0;
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
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
        dataReceived=0;
        overbuff=0;
        nbHops = 0;
        nbHopN=0;
        txAttempts = 0;
        i=0;

        for (i=0; i<7; i++){
            for(int j =0; j<5;j++){
                ordre[i][j]=-1;
            }
        }

        i=0;

        ackTimer = new OrwPkt("ACKTimer", ACKTimer);
        probingTimer = new OrwPkt("PROBINGTimer", PROBINGTimer);

        //BGW
        //these parameters are only needed for plain flooding
        hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;
        //BGW
        hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 180.0;
        EV <<"bcMaxEntries = "<<bcMaxEntries
                <<" bcDelTime = "<<bcDelTime<<endl;

        if(edc==-1)
        {
            scheduleAt(simTime()  +uniform(0.125, 0.01), probingTimer);
        }
    }
}

Orw::~Orw()
{
    cancelAndDelete(0);
}

void Orw::handleLowerMsg(cMessage* msg)
{
    OrwPkt*           pkt        = check_and_cast<OrwPkt*>(msg);
    double sum = 0;
    double sum_edc = 0;
    double oldEdc = edc;
    switch (pkt->getKind())
    {
    case UNKNOWN:
        delete(pkt);
        break;
    case ACK:
        if(pkt->getDestAddr()==myNetwAddr)
        {
            if(netQueue.size()!=0)
            {
                if(netQueue.front()->getSeqNum()==pkt->getSeqNum() && netQueue.front()->getSrcAddr()==pkt->getInitialsrcAddr())
                {
                    netQueue.pop_front();
                    txAttempts = 0;
                    cancelEvent(ackTimer);
                    tryToSend();
                }
            }
        }
        delete(pkt);
        break;
    case DATA:
        if(myNetwAddr!=1)
        {
            switch (pkt->getRelaySrcAdd())
            {
            case 0:
                contBeacon0++;
                if(contBeacon0==1)
                {
                    seqBeacon0=pkt->getSeqNum();
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                else if(contBeacon0==3)
                {
                    contBeacon0=0;
                    Qb0=((pkt->getSeqNum()-seqBeacon0+1)/3);
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    edc_v[1][pkt->getRelaySrcAdd()]=Qb0;
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu 3eme DATA de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<",  et p : "<<edc_v[1][pkt->getRelaySrcAdd()]<<endl;
                }
                else
                {
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                break;
            case 1:
                contBeacon1++;
                if(contBeacon1==1)
                {
                    seqBeacon1=pkt->getSeqNum();
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                else if(contBeacon1==3)
                {
                    contBeacon1=0;
                    Qb1=((pkt->getSeqNum()-seqBeacon1+1)/3);
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    edc_v[1][pkt->getRelaySrcAdd()]=Qb1;
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu 3eme DATA de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<",  et p : "<<edc_v[1][pkt->getRelaySrcAdd()]<<endl;
                }
                else
                {
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                break;
            case 2:
                contBeacon2++;
                if(contBeacon2==1)
                {
                    seqBeacon2=pkt->getSeqNum();
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                else if(contBeacon2==3)
                {
                    contBeacon2=0;
                    Qb2=((pkt->getSeqNum()-seqBeacon2+1)/3);
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    edc_v[1][pkt->getRelaySrcAdd()]=Qb2;
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu 3eme DATA de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<",  et p : "<<edc_v[1][pkt->getRelaySrcAdd()]<<endl;
                }
                else
                {
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                break;
            case 3:
                contBeacon3++;
                if(contBeacon3==1)
                {
                    seqBeacon3=pkt->getSeqNum();
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                else if(contBeacon3==3)
                {
                    contBeacon3=0;
                    Qb3=((pkt->getSeqNum()-seqBeacon3+1)/3);
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    edc_v[1][pkt->getRelaySrcAdd()]=Qb3;
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu 3eme DATA de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<",  et p : "<<edc_v[1][pkt->getRelaySrcAdd()]<<endl;
                }
                else
                {
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                break;
            case 4:
                contBeacon4++;
                if(contBeacon4==1)
                {
                    seqBeacon4=pkt->getSeqNum();
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                else if(contBeacon4==3)
                {
                    contBeacon4=0;
                    Qb4=((pkt->getSeqNum()-seqBeacon4+1)/3);
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    edc_v[1][pkt->getRelaySrcAdd()]=Qb4;
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu 3eme DATA de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<",  et p : "<<edc_v[1][pkt->getRelaySrcAdd()]<<endl;
                }
                else
                {
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                break;
            case 5:
                contBeacon5++;
                if(contBeacon5==1)
                {
                    seqBeacon5=pkt->getSeqNum();
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                else if(contBeacon5==3)
                {
                    contBeacon5=0;
                    Qb5=((pkt->getSeqNum()-seqBeacon5+1)/3);
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    edc_v[1][pkt->getRelaySrcAdd()]=Qb5;
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu 3eme DATA de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<",  et p : "<<edc_v[1][pkt->getRelaySrcAdd()]<<endl;
                }
                else
                {
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                break;
            case 6:
                contBeacon6++;
                if(contBeacon6==1)
                {
                    seqBeacon6=pkt->getSeqNum();
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                else if(contBeacon6==3)
                {
                    contBeacon6=0;
                    Qb6=((pkt->getSeqNum()-seqBeacon6+1)/3);
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    edc_v[1][pkt->getRelaySrcAdd()]=Qb6;
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu 3eme DATA de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<",  et p : "<<edc_v[1][pkt->getRelaySrcAdd()]<<endl;
                }
                else
                {
                    edc_v[0][pkt->getRelaySrcAdd()]=pkt->getEdc();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai endentu un DATA  de : "<<pkt->getRelaySrcAdd()<<" avec edc = "<<pkt->getEdc()<<endl;
                }
                break;
            default:
                break;
            }
        }

        // If pkt is for me
        if(pkt->getTabAdd(myNetwAddr)==1)
        {
            OrwPkt* ack = new OrwPkt("ACK", ACK);
            ack->setDestAddr(pkt->getRelaySrcAdd());
            ack->setSeqNum(pkt->getSeqNum());
            ack->setInitialsrcAddr(pkt->getSrcAddr());
            ack->setSrcAddr(myNetwAddr);
            setDownControlInfo(ack, LAddress::L2BROADCAST);
            sendDown(ack);

            dataReceived++;
            if(myNetwAddr==1)
            {
                if(notBroadcasted(pkt)){
                    recuControl(pkt);
                    EV<<"Je suis node 1, j'ai recu un paqute de node: "<<pkt->getSrcAddr()<<endl;
                    sendUp(pkt);
                    nbDataPacketsReceived++;
                }
                else
                    delete(pkt);
                break;
            }
            //if I am not 1,
            //Update EDC
            sum = 0;
            sum_edc = 0;
            for(int i=0; i<7; i++)
            {
                if(edc_v[0][i]!=-1 &&edc_v[1][i]!=-1 )
                {
                    sum= sum + edc_v[1][i];
                    sum_edc= sum_edc + edc_v[0][i]*edc_v[1][i];
                }
            }
            edc=(1+ sum_edc)/sum;
            EV<<" Table is : "<<endl;
            for(i=0;i<7;i++)
            {
                EV<< i <<"  : "<<edc_v[0][i]<<endl;
            }
            EV<<"Donc, mon EDC = "<< edc<<endl;

            pkt->setTtl(pkt->getTtl()-1);
            pkt->setRelaySrcAdd(myNetwAddr);
            if(netQueue.size()<maxNetQueueSize)
            {
                netQueue.push_back(static_cast<  OrwPkt*>(pkt));
            }
            else
            {
                netQueue.pop_front();
                netQueue.push_back(static_cast<  OrwPkt*>(pkt));
                txAttempts = 0;
                cancelEvent(ackTimer);
            }
            EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un paqute de node: "<<pkt->getSrcAddr()<<", et je le mettre dans la queue." <<endl;

            if(netQueue.size()==1 && edc!=-1)
            {
                tryToSend();
            }
            break;

        }
        //Data comme ACK
        else if(!netQueue.empty() && pkt->getSrcAddr() ==netQueue.front()->getSrcAddr()  && netQueue.front()->getSeqNum()==pkt->getSeqNum())
        {
            netQueue.pop_front();
            txAttempts = 0;
            cancelEvent(ackTimer);
            tryToSend();
        }
        else
            delete(pkt);
        break;
    case REPPRO:
        if(pkt->getDestAddr()==myNetwAddr)
        {
            edc_v[0][pkt->getSrcAddr()]=pkt->getEdc();
            edc_v[1][pkt->getSrcAddr()]=1;
            sum = 0;
            sum_edc = 0;
            for(int i=0; i<7; i++)
            {
                if(edc_v[0][i]!=-1 &&edc_v[1][i]!=-1 )
                {
                    sum= sum + edc_v[1][i];
                    sum_edc= sum_edc + edc_v[0][i]*edc_v[1][i];
                }
            }
            edc=(1+ sum_edc)/sum;
            EV<<"Je suis "<< myNetwAddr<<"  Table is : "<<endl;
            for(i=0;i<7;i++)
            {
                EV<< i <<"  : "<<edc_v[0][i]<<endl;
            }
            EV<<"Donc, mon EDC = "<< edc<<endl;
            cancelEvent(probingTimer);
            if(oldEdc==-1 && edc!=-1)
            {
                tryToSend();
            }
        }
        delete(pkt);
        break;
    case PROBING:
        if(edc!=-1)
        {
            OrwPkt* repPro = new OrwPkt("REPPRO", REPPRO);
            repPro->setSrcAddr(myNetwAddr);
            repPro->setDestAddr(pkt->getSrcAddr());
            repPro->setEdc(edc);
            setDownControlInfo(repPro, LAddress::L2BROADCAST);
            sendDown(repPro);
        }
        delete (pkt);
        break;
    default:
        break;
    }
}

void Orw::handleLowerControl(cMessage *msg)
{
    delete msg;
}

void Orw::handleUpperMsg(cMessage* msg)
{
    OrwPkt*    pkt   = new OrwPkt(msg->getName(), DATA);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setRelaySrcAdd(myNetwAddr);
    pkt->setSeqNum(floodSeqNumber);
    floodSeqNumber++;
    pkt->setTtl(defaultTtl);
    if(netQueue.size()<maxNetQueueSize)
    {
        netQueue.push_back(static_cast<  OrwPkt*>(pkt));
    }
    else
    {
        netQueue.pop_front();
        netQueue.push_back(static_cast<  OrwPkt*>(pkt));
        txAttempts = 0;
        cancelEvent(ackTimer);
    }
    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai un paqute et je le mettre dans la queue." <<endl;
    if(netQueue.size()==1 && edc!=-1)
    {
        tryToSend();
    }
}

void Orw::finish()
{

    bcMsgs.clear();
    cOwnedObject *Del=NULL;
    int OwnedSize=this->defaultListSize();
    for(int i=0;i<OwnedSize;i++){
        Del=this->defaultListGet(0);
        this->drop(Del);
        delete Del;
    }

    cancelEvent(ackTimer);

    recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
    recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
    recordScalar("nbDataPacketsSent", nbDataPacketsSent);
    recordScalar("dataReceived", dataReceived);
    recordScalar("nbDesquence", nbDesquence);
    recordScalar("nbForwarded0", nbForwarded0);
    recordScalar("nbForwarded2", nbForwarded2);
    recordScalar("nbForwarded3", nbForwarded3);
    recordScalar("nbForwarded4", nbForwarded4);
    recordScalar("nbForwarded5", nbForwarded5);
    recordScalar("nbForwarded6", nbForwarded6);
    recordScalar("nbRecu0", nbRecu0);
    recordScalar("nbRecu2", nbRecu2);
    recordScalar("nbRecu3", nbRecu3);
    recordScalar("nbRecu4", nbRecu4);
    recordScalar("nbRecu5", nbRecu5);
    recordScalar("nbRecu6", nbRecu6);

    recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);


}

bool Orw::notBroadcasted(OrwPkt* msg) {

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
        if ((it->srcAddr == msg->getSrcAddr()) && (it->seqNum
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
    switch (msg->getSrcAddr())
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
    bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getSrcAddr(), simTime() +bcDelTime));
    return true;
}

void Orw::forwardControl(OrwPkt *msg)
{
    int x=msg->getSrcAddr();
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

void Orw::recuControl(OrwPkt *msg)
{
    int x=msg->getSrcAddr();
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


void Orw::tryToSend()
{

    if(netQueue.size()!=0)
    {
        if(txAttempts>=maxTry)
        {
            netQueue.pop_front();
            //netQueue.erase(netQueue.front());
            txAttempts=0;
            cancelEvent(ackTimer);
        }
        OrwPkt *msg = static_cast<OrwPkt *>(netQueue.front()->dup());
        msg->setEdc(edc);
        for(i =0; i<7; i++)
        {
            if(edc_v[0][i]<edc && edc_v[0][i]!=-1)
                msg->setTabAdd(i, 1);
            else
                msg->setTabAdd(i, -1);
        }
        if(myNetwAddr!=msg->getSrcAddr())
        {
            OrwPkt* coypkt = static_cast<OrwPkt*>(msg->dup());
            forwardControl(coypkt);
            setDownControlInfo(coypkt, LAddress::L2BROADCAST);
            EV<<"Je suis node: "<<myNetwAddr<< ", j'envoie le paqute de Queue de node: "<<coypkt->getSrcAddr()<<",  a "<<endl;
            for(i =0; i<7; i++)
            {
                if(coypkt->getTabAdd(i)==1)
                    EV<<i<<" , "<<endl;
            }
            sendDown(coypkt);
            nbDataPacketsForwarded++;
            delete (msg);
        }
        else
        {
            OrwPkt* coypkt = static_cast<OrwPkt*>(msg->dup());
            setDownControlInfo(coypkt, LAddress::L2BROADCAST);
            EV<<"Je suis node: "<<myNetwAddr<< ", j'envoie le paqute de Queue de moi meme,  a " <<endl;
            for(i =0; i<7; i++)
            {
                if(coypkt->getTabAdd(i)==1)
                    EV<<i<<" , "<<endl;
            }
            sendDown(coypkt);
            nbDataPacketsSent++;
            delete (msg);
        }
        txAttempts ++;
        scheduleAt(simTime() + 0.1, ackTimer);
    }
}




void Orw::handleSelfMsg(cMessage *m)
{
    OrwPkt* probing  = new OrwPkt("PROBING",PROBING);
    switch (m->getKind())
    {
    case PROBINGTimer:
        probing->setSrcAddr(myNetwAddr);
        scheduleAt(simTime()+0.125, probingTimer);
        setDownControlInfo(probing, LAddress::L2BROADCAST);
        sendDown(probing);
        break;
    case ACKTimer:
        //ACK timerout =0.1s
        tryToSend();
        break;
    default:
        break;
    }
}


cMessage* Orw::decapsMsg(OrwPkt *msg)
{
    cMessage *m = msg->decapsulate();
    //setUpControlInfo(m, msg->getSrcAddr());
    nbHops = nbHops + msg->getNbHops();
    delete msg;
    return m;
}

