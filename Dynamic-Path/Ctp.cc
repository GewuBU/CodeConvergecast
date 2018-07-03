
#include "Ctp.h"

#include <cassert>
#include <stdlib.h>
#include "NetwControlInfo.h"
#include "CtpPkt_m.h"
#include "SimTracer.h"
#include "LogNormalShadowing.h"

Define_Module(Ctp);

void Ctp::initialize(int stage)
{
    BaseNetwLayer::initialize(stage);

    if(stage == 1) {
        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 6;
        i=0;

        if(myNetwAddr!=1)
        {
            parent=-1;
            etxM=-1;
        }
        else
        {
            parent = 1;
            etxM=0;
        }
        etx1_0=0;
        etx1_1=0;
        etx1_2=0;
        etx1_3=0;
        etx1_4=0;
        etx1_5=0;
        etx1_6=0;
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

        for (i=0; i<3; i++){
            for(int j =0; j<7;j++){
                etxM_v[i][j]=-1;
            }
        }

        maxTry = par("maxTry");
        maxNetQueueSize=par("maxNetQueueSize");

        floodSeqNumber = 0;
        beaconNumber =0;
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

        ackTimer = new CtpPkt("ACKTimer", ACKTimer);

        //BGW
        //these parameters are only needed for plain flooding
        hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;
        //BGW
        hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 180.0;
        EV <<"bcMaxEntries = "<<bcMaxEntries
                <<" bcDelTime = "<<bcDelTime<<endl;

        CtpPkt* msg = new CtpPkt("BEACON", BEACON);
        scheduleAt(simTime()  +uniform(0.1, 0.05), msg);
        //scheduleAt(simTime() +1 + uniform(0.125, 0.01), msg);
    }
}

Ctp::~Ctp()
{
    cancelAndDelete(0);
}

void Ctp::handleLowerMsg(cMessage* msg)
{
    CtpPkt*           pkt        = check_and_cast<CtpPkt*>(msg);

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
                if(netQueue.front()->getSeqNum()==pkt->getSeqNum())
                {
                    netQueue.pop_front();
                    //netQueue.erase(netQueue.front());
                    txAttempts = 0;
                    cancelEvent(ackTimer);
                    tryToSend();
                    //scheduleAt(simTime() + 0.1, ackTimer);
                }
            }
        }
        delete(pkt);
        break;
    case DATA:
        if(pkt->getDestAddr()==myNetwAddr)
        {
            if(maxTry>1)
            {
                CtpPkt* ack = new CtpPkt("ACK", ACK);
                ack->setDestAddr(pkt->getRelaySrcAdd());
                ack->setSeqNum(pkt->getSeqNum());
                ack->setSrcAddr(myNetwAddr);
                setDownControlInfo(ack, LAddress::L2BROADCAST);
                sendDown(ack);
            }
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
            pkt->setTtl(pkt->getTtl()-1);
            pkt->setRelaySrcAdd(myNetwAddr);
            /*
            if(parent!=-1)//(myNetwAddr!=1
            {
                CtpPkt* coypkt = static_cast<CtpPkt*>(pkt->dup());
                coypkt->setDestAddr(parent);
                forwardControl(coypkt);
                setDownControlInfo(coypkt, LAddress::L2BROADCAST);
                EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un paqute de node: "<<coypkt->getSrcAddr()<<", et je le forward a mon parent: "<<parent <<endl;
                sendDown(coypkt);
                nbDataPacketsForwarded++;
                delete (pkt);
            }
            else
            {
             */
            if(netQueue.size()<maxNetQueueSize)
            {
                netQueue.push_back(static_cast<  CtpPkt*>(pkt));
            }
            else
            {
                netQueue.pop_front();
                //netQueue.erase(netQueue.front());
                netQueue.push_back(static_cast<  CtpPkt*>(pkt));
                txAttempts = 0;
                cancelEvent(ackTimer);
            }
            EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un paqute de node: "<<pkt->getSrcAddr()<<", et je le mettre dans la queue." <<endl;
            if(netQueue.size()==1 && parent!=-1)
            {
                tryToSend();
                //scheduleAt(simTime() + 0.1, ackTimer);
            }
            //}
            break;
        }
        //Data comme ACK
        else if(!netQueue.empty() && pkt->getSrcAddr() ==netQueue.front()->getSrcAddr()  && netQueue.front()->getSeqNum()==pkt->getSeqNum())
        {
            netQueue.pop_front();
            //netQueue.erase(netQueue.front());
            txAttempts = 0;
            cancelEvent(ackTimer);
            tryToSend();
        }
        else
            delete(pkt);
        break;
    case BEACON:
        switch (pkt->getSrcAddr())
        {
        case 0:
            contBeacon0++;
            if(contBeacon0==1)
            {
                seqBeacon0=pkt->getSeqNum();
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_0+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_0<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            else if(contBeacon0==3)
            {
                contBeacon0=0;
                int antienQb=Qb0;
                Qb0=((pkt->getSeqNum()-seqBeacon0+1)/3)*100;
                etx1_0=Qb0*0.9+antienQb*0.1;
                for(int i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {
                        etxM_v[1][i]=etx1_0+ etxM_v[0][i];
                    }
                }

                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_0+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_0<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
                // si mon parent n'a pas de parent
                int antianeParent = parent;
                for( i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {

                        if(etxM<0 || etxM> (etxM_v[1][i]+1))
                        {
                            etxM = etxM_v[1][i];
                            parent = i;
                        }
                    }
                }
                if(antianeParent!=parent)
                {
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu certains BEACONs de: "<<pkt->getSrcAddr()<<", et mon table c'est: 0: "<<etxM_v[1][0]<<", 1: "<<etxM_v[1][1]<<", 2: "<<etxM_v[1][2]<<", 3: "<<etxM_v[1][3]<<", 4: "<<etxM_v[1][4]<<", 5: "<<etxM_v[1][5]<<", 6: "<<etxM_v[1][6] <<endl;
                    EV<<"Donc, mon parent est : "<<parent<<endl;
                    if(netQueue.size()!=0 && antianeParent==-1)
                    {
                        tryToSend();
                        //scheduleAt(simTime() + 0.1, ackTimer);
                    }
                }
            }
            else
            {
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_0+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_0<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            break;
        case 1:
            contBeacon1++;
            if(contBeacon1==1)
            {
                seqBeacon1=pkt->getSeqNum();
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_1+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_1<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            else if(contBeacon1==3)
            {
                contBeacon1=0;
                int antienQb=Qb1;
                Qb1=((pkt->getSeqNum()-seqBeacon1+1)/3)*100;
                etx1_1=Qb1*0.9+antienQb*0.1;
                for(int i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {
                        etxM_v[1][i]=etx1_1+ etxM_v[0][i];
                    }
                }

                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_1+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_1<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
                // si mon parent n'a pas de parent
                int antianeParent = parent;
                for( i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {

                        if(etxM<0 || etxM> (etxM_v[1][i]+1))
                        {
                            etxM = etxM_v[1][i];
                            parent = i;
                        }
                    }
                }
                if(antianeParent!=parent)
                {
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu certains BEACONs de: "<<pkt->getSrcAddr()<<", et mon table c'est: 0: "<<etxM_v[1][0]<<", 1: "<<etxM_v[1][1]<<", 2: "<<etxM_v[1][2]<<", 3: "<<etxM_v[1][3]<<", 4: "<<etxM_v[1][4]<<", 5: "<<etxM_v[1][5]<<", 6: "<<etxM_v[1][6] <<endl;
                    EV<<"Donc, mon parent est : "<<parent<<endl;
                    if(netQueue.size()!=0 && antianeParent==-1)
                    {
                        tryToSend();
                        //scheduleAt(simTime() + 0.1, ackTimer);
                    }
                }
            }
            else
            {
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_1+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_1<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            break;
        case 2:
            contBeacon2++;
            if(contBeacon2==1)
            {
                seqBeacon2=pkt->getSeqNum();
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_2+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_2<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            else if(contBeacon2==3)
            {
                contBeacon2=0;
                int antienQb=Qb2;
                Qb2=((pkt->getSeqNum()-seqBeacon2+1)/3)*100;
                etx1_2=Qb2*0.9+antienQb*0.1;
                for(int i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {
                        etxM_v[1][i]=etx1_2+ etxM_v[0][i];
                    }
                }

                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_2+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_2<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
                // si mon parent n'a pas de parent
                int antianeParent = parent;
                for( i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {

                        if(etxM<0 || etxM> (etxM_v[1][i]+1))
                        {
                            etxM = etxM_v[1][i];
                            parent = i;
                        }
                    }
                }
                if(antianeParent!=parent)
                {
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu certains BEACONs de: "<<pkt->getSrcAddr()<<", et mon table c'est: 0: "<<etxM_v[1][0]<<", 1: "<<etxM_v[1][1]<<", 2: "<<etxM_v[1][2]<<", 3: "<<etxM_v[1][3]<<", 4: "<<etxM_v[1][4]<<", 5: "<<etxM_v[1][5]<<", 6: "<<etxM_v[1][6] <<endl;
                    EV<<"Donc, mon parent est : "<<parent<<endl;
                    if(netQueue.size()!=0 && antianeParent==-1)
                    {
                        tryToSend();
                        //scheduleAt(simTime() + 0.1, ackTimer);
                    }
                }
            }
            else
            {
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_2+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_2<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            break;
        case 3:
            contBeacon3++;
            if(contBeacon3==1)
            {
                seqBeacon3=pkt->getSeqNum();
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_3+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_3<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            else if(contBeacon3==3)
            {
                contBeacon3=0;
                int antienQb=Qb3;
                Qb3=((pkt->getSeqNum()-seqBeacon3+1)/3)*100;
                etx1_3=Qb3*0.9+antienQb*0.1;
                for(int i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {
                        etxM_v[1][i]=etx1_3+ etxM_v[0][i];
                    }
                }

                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_3+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_3<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
                // si mon parent n'a pas de parent
                int antianeParent = parent;
                for( i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {

                        if(etxM<0 || etxM> (etxM_v[1][i]+1))
                        {
                            etxM = etxM_v[1][i];
                            parent = i;
                        }
                    }
                }
                if(antianeParent!=parent)
                {
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu certains BEACONs de: "<<pkt->getSrcAddr()<<", et mon table c'est: 0: "<<etxM_v[1][0]<<", 1: "<<etxM_v[1][1]<<", 2: "<<etxM_v[1][2]<<", 3: "<<etxM_v[1][3]<<", 4: "<<etxM_v[1][4]<<", 5: "<<etxM_v[1][5]<<", 6: "<<etxM_v[1][6] <<endl;
                    EV<<"Donc, mon parent est : "<<parent<<endl;
                    if(netQueue.size()!=0 && antianeParent==-1)
                    {
                        tryToSend();
                        //scheduleAt(simTime() + 0.1, ackTimer);
                    }
                }
            }
            else
            {
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_3+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_3<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            break;
        case 4:
            contBeacon4++;
            if(contBeacon4==1)
            {
                seqBeacon4=pkt->getSeqNum();
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_4+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_4<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            else if(contBeacon4==3)
            {
                contBeacon4=0;
                int antienQb=Qb4;
                Qb4=((pkt->getSeqNum()-seqBeacon4+1)/3)*100;
                etx1_4=Qb4*0.9+antienQb*0.1;
                for(int i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {
                        etxM_v[1][i]=etx1_4+ etxM_v[0][i];
                    }
                }

                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_4+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_4<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
                // si mon parent n'a pas de parent
                int antianeParent = parent;
                for( i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {

                        if(etxM<0 || etxM> (etxM_v[1][i]+1))
                        {
                            etxM = etxM_v[1][i];
                            parent = i;
                        }
                    }
                }
                if(antianeParent!=parent)
                {
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu certains BEACONs de: "<<pkt->getSrcAddr()<<", et mon table c'est: 0: "<<etxM_v[1][0]<<", 1: "<<etxM_v[1][1]<<", 2: "<<etxM_v[1][2]<<", 3: "<<etxM_v[1][3]<<", 4: "<<etxM_v[1][4]<<", 5: "<<etxM_v[1][5]<<", 6: "<<etxM_v[1][6] <<endl;
                    EV<<"Donc, mon parent est : "<<parent<<endl;
                    if(netQueue.size()!=0 && antianeParent==-1)
                    {
                        tryToSend();
                        //scheduleAt(simTime() + 0.1, ackTimer);
                    }
                }
            }
            else
            {
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_4+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_4<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            break;
        case 5:
            contBeacon5++;
            if(contBeacon5==1)
            {
                seqBeacon5=pkt->getSeqNum();
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_5+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_5<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            else if(contBeacon5==3)
            {
                contBeacon5=0;
                int antienQb=Qb5;
                Qb5=((pkt->getSeqNum()-seqBeacon5+1)/3)*100;
                etx1_5=Qb5*0.9+antienQb*0.1;
                for(int i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {
                        etxM_v[1][i]=etx1_5+ etxM_v[0][i];
                    }
                }

                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_5+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_5<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
                // si mon parent n'a pas de parent
                int antianeParent = parent;
                for( i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {

                        if(etxM<0 || etxM> (etxM_v[1][i]+1))
                        {
                            etxM = etxM_v[1][i];
                            parent = i;
                        }
                    }
                }
                if(antianeParent!=parent)
                {
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu certains BEACONs de: "<<pkt->getSrcAddr()<<", et mon table c'est: 0: "<<etxM_v[1][0]<<", 1: "<<etxM_v[1][1]<<", 2: "<<etxM_v[1][2]<<", 3: "<<etxM_v[1][3]<<", 4: "<<etxM_v[1][4]<<", 5: "<<etxM_v[1][5]<<", 6: "<<etxM_v[1][6] <<endl;
                    EV<<"Donc, mon parent est : "<<parent<<endl;
                    if(netQueue.size()!=0 && antianeParent==-1)
                    {
                        tryToSend();
                        //scheduleAt(simTime() + 0.1, ackTimer);
                    }
                }
            }
            else
            {
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_5+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_5<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            break;
        case 6:
            contBeacon6++;
            if(contBeacon6==1)
            {
                seqBeacon6=pkt->getSeqNum();
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_6+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_6<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            else if(contBeacon6==3)
            {
                contBeacon6=0;
                int antienQb=Qb6;
                Qb6=((pkt->getSeqNum()-seqBeacon6+1)/3)*100;
                etx1_6=Qb6*0.9+antienQb*0.1;
                for(int i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {
                        etxM_v[1][i]=etx1_6+ etxM_v[0][i];
                    }
                }

                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_6+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_6<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
                // si mon parent n'a pas de parent
                int antianeParent = parent;
                for( i = 0; i<7; i++ )
                {
                    if(etxM_v[2][i]!=-1)
                    {

                        if(etxM<0 || etxM> (etxM_v[1][i]+1))
                        {
                            etxM = etxM_v[1][i];
                            parent = i;
                        }
                    }
                }
                if(antianeParent!=parent)
                {
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu certains BEACONs de: "<<pkt->getSrcAddr()<<", et mon table c'est: 0: "<<etxM_v[1][0]<<", 1: "<<etxM_v[1][1]<<", 2: "<<etxM_v[1][2]<<", 3: "<<etxM_v[1][3]<<", 4: "<<etxM_v[1][4]<<", 5: "<<etxM_v[1][5]<<", 6: "<<etxM_v[1][6] <<endl;
                    EV<<"Donc, mon parent est : "<<parent<<endl;
                    if(netQueue.size()!=0 && antianeParent==-1)
                    {
                        tryToSend();
                        //scheduleAt(simTime() + 0.1, ackTimer);
                    }
                }
            }
            else
            {
                if(pkt->getValParent()!=-1)
                {
                    etxM_v[0][pkt->getSrcAddr()]=pkt->getEtxm();
                    etxM_v[1][pkt->getSrcAddr()]=etx1_6+ pkt->getEtxm();
                    etxM_v[2][pkt->getSrcAddr()]=pkt->getValParent();
                    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un BEACON de : "<<pkt->getSrcAddr()<<" avec ETXm = "<<pkt->getEtxm()<<", et mon ETX1_"<<pkt->getSrcAddr()<< "c'est: "<<etx1_6<<endl;
                    EV<<"Donc, update table "<<pkt->getSrcAddr()<<" ="<< etxM_v[1][pkt->getSrcAddr()]<<endl;
                }
            }
            break;
        default:
            break;
        }
        delete (pkt);
        break;
        default:
            break;
    }
}

void Ctp::handleLowerControl(cMessage *msg)
{
    delete msg;
}

void Ctp::handleUpperMsg(cMessage* msg)
{
    CtpPkt*    pkt   = new CtpPkt(msg->getName(), DATA);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setRelaySrcAdd(myNetwAddr);
    pkt->setSeqNum(floodSeqNumber);
    floodSeqNumber++;
    pkt->setTtl(defaultTtl);
    /*
    if(parent!=-1)
    {
        pkt->setDestAddr(parent);
        setDownControlInfo(pkt, LAddress::L2BROADCAST);
        sendDown(pkt);
        nbDataPacketsSent++;
        EV<<"Je suis node: "<< myNetwAddr <<" , J'ai un Paquet, et je l'envois."<< " a mon parent: "<< parent  <<endl;
    }
    else
    {
     */
    if(netQueue.size()<maxNetQueueSize)
    {
        netQueue.push_back(static_cast<  CtpPkt*>(pkt));
    }
    else
    {
        netQueue.pop_front();
        //netQueue.erase(netQueue.front());
        netQueue.push_back(static_cast<  CtpPkt*>(pkt));
        txAttempts = 0;
        cancelEvent(ackTimer);
    }
    EV<<"Je suis node: "<<myNetwAddr<< ", j'ai recu un paqute de node: "<<pkt->getSrcAddr()<<", et je le mettre dans la queue." <<endl;
    if(netQueue.size()==1 && parent!=-1)
    {
        tryToSend();
        //scheduleAt(simTime() + 0.1, ackTimer);
    }
    //}
}

void Ctp::finish()
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

bool Ctp::notBroadcasted(CtpPkt* msg) {

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

void Ctp::forwardControl(CtpPkt *msg)
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

void Ctp::recuControl(CtpPkt *msg)
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


void Ctp::tryToSend()
{
    //boucle
    // while(1){
    if(netQueue.size()!=0 && parent!=-1)
    {
        if(txAttempts>=maxTry)
        {
            netQueue.pop_front();
            //netQueue.erase(netQueue.front());
            txAttempts=0;
            cancelEvent(ackTimer);
        }
        CtpPkt *msg = static_cast<CtpPkt *>(netQueue.front()->dup());

        //BGW pop=enable reTX, non-pop=disable reTX
        //netQueue.pop_front();
        msg->setDestAddr(parent);
        if(myNetwAddr!=msg->getSrcAddr())
        {
            CtpPkt* coypkt = static_cast<CtpPkt*>(msg->dup());
            forwardControl(coypkt);
            setDownControlInfo(coypkt, LAddress::L2BROADCAST);
            txAttempts++;
            EV<<"Je suis node: "<<myNetwAddr<< ", j'envoie le paqute de Queue de node: "<<coypkt->getSrcAddr()<<",  a mon parent: "<<parent <<endl;
            sendDown(coypkt);
            nbDataPacketsForwarded++;
            delete (msg);
        }
        else
        {
            CtpPkt* coypkt = static_cast<CtpPkt*>(msg->dup());
            nbDataPacketsSent++;
            setDownControlInfo(coypkt, LAddress::L2BROADCAST);
            EV<<"Je suis node: "<<myNetwAddr<< ", j'envoie le paqute de Queue de moi meme,  a mon parent: "<<parent <<endl;
            txAttempts++;
            int b = coypkt->getByteLength();
            sendDown(coypkt);
            delete (msg);
        }

        scheduleAt(simTime() + 0.1, ackTimer);
    }
    /*
    else
        break;
    }
     */
}




void Ctp::handleSelfMsg(cMessage *m)
{
    CtpPkt* msg  = check_and_cast<CtpPkt*>(m);
    switch (m->getKind())
    {
    case BEACON:
        //CtpPkt* msg  = check_and_cast<CtpPkt*>(m);
        msg->setSeqNum(beaconNumber);
        beaconNumber++;
        msg->setSrcAddr(myNetwAddr);
        msg->setValParent(parent);
        msg->setEtxm(etxM);
        scheduleAt(simTime()+0.05, msg->dup());
        setDownControlInfo(msg, LAddress::L2BROADCAST);
        sendDown(msg);
        break;
    case ACKTimer:
        //ACK timerout =0.1s
        tryToSend();
        //scheduleAt(simTime() + 0.1, ackTimer);
        break;
    default:
        break;
    }
}


cMessage* Ctp::decapsMsg(CtpPkt *msg)
{
    cMessage *m = msg->decapsulate();
    //setUpControlInfo(m, msg->getSrcAddr());
    nbHops = nbHops + msg->getNbHops();
    delete msg;
    return m;
}

