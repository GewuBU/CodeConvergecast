
#include "CnSm1.h"

#include <cassert>
#include <stdlib.h>
#include "NetwControlInfo.h"
#include "CnSm1Pkt_m.h"
#include "SimTracer.h"
#include "LogNormalShadowing.h"

Define_Module(CnSm1);

void CnSm1::initialize(int stage)
{
    BaseNetwLayer::initialize(stage);

    if(stage == 1) {
        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 6;
        optiEnable=par("optiEnable");
        posture = par("posture");
        sinkAddress =1;
        floodSeqNumber = 0;
        nbDataPacketsForwarded = 0;
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
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
        nbHops = 0;
        check=0;
        nbHopN=0;
        i=0;
        rep=0;
        k=0;
        rep1=0;
        rnd=0;
        for (i=0; i<6; i++)
            tab[i]=-1;
        for (i=0; i<7; i++){
            for(int j =0; j<5;j++){
                tabmsgrec[i][j]=-1;
            }
        }

        i=0;
        if(myNetwAddr==0){ tab[0]=1; tab[1]=0; tab[2]=6; tab[3]=2; tab[4]=-1; tab[5]=-1;}
        if(myNetwAddr==2){ tab[0]=1; tab[1]=0; tab[2]=3; tab[3]=1; tab[4]=-1; tab[5]=-1;}
        if(myNetwAddr==3){ tab[0]=1; tab[1]=0; tab[2]=2; tab[3]=1; tab[4]=-1; tab[5]=-1;}
        if(myNetwAddr==4){ tab[0]=5; tab[1]=2; tab[2]=-1; tab[3]=-1; tab[4]=-1; tab[5]=-1;}
        if(myNetwAddr==5){ tab[0]=0; tab[1]=1; tab[2]=6; tab[3]=2; tab[4]=-1; tab[5]=-1;}
        if(myNetwAddr==6){ tab[0]=0; tab[1]=1; tab[2]=3; tab[3]=1; tab[4]=-1; tab[5]=-1;}

        //BGW
        //these parameters are only needed for plain flooding
        hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;
        //BGW
        hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 180.0;
        EV <<"bcMaxEntries = "<<bcMaxEntries
                <<" bcDelTime = "<<bcDelTime<<endl;

    }
}

CnSm1::~CnSm1()
{
    cancelAndDelete(0);
}

void CnSm1::handleLowerMsg(cMessage* msg)
{
    EV<<"LLLLLLLLLLOOOOOOOOOWWWWWWWWWWWWEEEEEEEEEEEERRRRRRRRRRRRRRRRRRR : "<<myNetwAddr<< endl;
    CnSm1Pkt*           netwMsg        = check_and_cast<CnSm1Pkt*>(msg);
    const LAddress::L3Type& finalDestAddr  = netwMsg->getFinalDestAddr();
    const LAddress::L3Type& finalDestAddr1  = netwMsg->getFinalDestAddr1();
    dataReceived++;

    if (finalDestAddr!=myNetwAddr && finalDestAddr1!=myNetwAddr) {
        delete netwMsg;
    }else{
        const cObject* pCtrlInfo = NULL;
        CnSm1Pkt* msgCopy;
        msgCopy = netwMsg;
        if (netwMsg->getKind()==DATA && myNetwAddr!=1){
            if (netwMsg->getTtl()>1)
            {
                msgCopy = check_and_cast<CnSm1Pkt*>(netwMsg->dup());
                EV<<", par l'intermediare de : "<<netwMsg->getSrcAddr()<<endl;
                netwMsg->setSrcAddr(myNetwAddr);
                netwMsg->setTtl( netwMsg->getTtl()-1 );
                rnd=rand() % 2;
                i=0;
                while (i<=3){
                    if(tab[i]==1){
                        rep1=1;
                        i=4;
                    }
                    i=i+2;
                }
                i=0;
                if (rep1==1){
                    netwMsg->setFinalDestAddr(1);
                    netwMsg->setFinalDestAddr1(2540);
                    check=1;
                }else{
                    if (rnd==0)
                        netwMsg->setFinalDestAddr(tab[i]);
                    else
                        netwMsg->setFinalDestAddr(tab[i+2]);
                }
                netwMsg->setFinalDestAddr1(2540);
                rep1=0;
                pCtrlInfo = netwMsg->removeControlInfo();
                if(optiEnable==1)
                {
                    multiSend(netwMsg);
                }
                else
                {
                    setDownControlInfo(netwMsg, LAddress::L2BROADCAST);
                    forwardControl(netwMsg);
                    sendDown(netwMsg);
                    nbDataPacketsForwarded++;
                }
                EV<<"je suis le noeud: "<<myNetwAddr<< ", j'ai reçu un msg de type: DATA de la part de :"<<netwMsg->getInitialSrcAddr()<<endl;

                EV<<"nbDataPacketsForwarded++; "<< nbDataPacketsForwarded << endl;

            }
            else
                EV<<" delete msg because ttl"<<endl;
        }
        if (netwMsg->getKind() == DATA && netwMsg->getFinalDestAddr()==1 && myNetwAddr==1 ) {
            const LAddress::L3Type& initSrcAdr  = netwMsg->getInitialSrcAddr();
            EV<<"initSrcAdr= "<< initSrcAdr<<endl;
            int seq =netwMsg->getSeqNum();
            if(notBroadcasted(msgCopy)){
                EV<<"je suis le noeud 1 j'ai reçu un msg de type DATA # " << seq << " de la part de :"<<netwMsg->getInitialSrcAddr()<<", par l'intermediare de : "<<netwMsg->getSrcAddr()<<endl;
                sendUp(decapsMsg(msgCopy));
                nbDataPacketsReceived++;
                EV<<"nbDataPacketsReceived++; "<<nbDataPacketsReceived<<endl;
            }else{
                // EV<<"je suis le noeud 1 j'ai reçu un msg de type DATA de la part de :"<<netwMsg->getInitialSrcAddr()<<", par l'intermediare de : "<<netwMsg->getSrcAddr()<<", et je l'ai supppp"<<endl;;
                delete msgCopy;
            }
        }else
            delete msgCopy;

        if (pCtrlInfo != NULL)
            delete pCtrlInfo;
    }
}

void CnSm1::handleLowerControl(cMessage *msg)
{
    delete msg;
}

void CnSm1::handleUpperMsg(cMessage* msg)
{
    //BGW
    const cObject* pCtrlInfo = NULL;

    EV<<"UUUUUUUUUUUUUUUUUUUUUUUUUPPPPPPPPPPPPPPPPEEEERRRRRRRRRRRR : "<<myNetwAddr<<endl;
    EV<<"ma table contient:"<<endl;
    for (i=0;i<=5; i++)
        EV<<tab[i]<<endl;
    i=0;
    CnSm1Pkt*    pkt   = new CnSm1Pkt(msg->getName(), DATA);
    pkt->setFinalDestAddr(tab[i]);
    pkt->setFinalDestAddr1(tab[i+2]);
    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setNbHops(0);
    pkt->setSeqNum(floodSeqNumber);
    pkt->setTtl(defaultTtl);
    floodSeqNumber++;
    //BGW
    pCtrlInfo = msg->removeControlInfo();


    assert(static_cast<cPacket*>(msg));
    pkt->encapsulate(static_cast<cPacket*>(msg));
    if(optiEnable==1)
    {
        multiSend(pkt);
    }
    else
    {
        setDownControlInfo(pkt, LAddress::L2BROADCAST);
        sendDown(pkt);
        nbDataPacketsSent++;
    }
    EV<<"nbDataPacketsSent++; "<<nbDataPacketsSent<<endl;

    //BGW
    if (pCtrlInfo != NULL)
        delete pCtrlInfo;
}

void CnSm1::finish()
{

    bcMsgs.clear();
    cOwnedObject *Del=NULL;
    int OwnedSize=this->defaultListSize();
    for(int i=0;i<OwnedSize;i++){
        Del=this->defaultListGet(0);
        this->drop(Del);
        delete Del;
    }

    recordScalar("nbDataPacketsForwarded", nbDataPacketsForwarded);
    recordScalar("nbDataPacketsReceived", nbDataPacketsReceived);
    recordScalar("nbDataPacketsSent", nbDataPacketsSent);
    recordScalar("dataReceived", dataReceived);
    recordScalar("nbDesquence", nbDesquence);
    recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);
}

bool CnSm1::notBroadcasted(CnSm1Pkt* msg) {

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
        if ((it->srcAddr == msg->getInitialSrcAddr()) && (it->seqNum
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
    switch (msg->getInitialSrcAddr())
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
    bcMsgs.push_back(Bcast(msg->getSeqNum(), msg->getInitialSrcAddr(), simTime() +bcDelTime));
    return true;
}

void CnSm1::forwardControl(CnSm1Pkt *msg)
{
    int x=msg->getInitialSrcAddr();
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

void CnSm1::recuControl(CnSm1Pkt *msg)
{
    int x=msg->getInitialSrcAddr();
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

void CnSm1::multiSend(CnSm1Pkt* msg)
{
    //BGW
    CnSm1Pkt* msgCopy;
    int nbT=0;
    LogNormalShadowing LS1, LS2;
    LS1.sender=(int)(myNetwAddr);
    LS1.receiver=msg->getFinalDestAddr();
    LS1.posture=posture;
    int nbT1=LS1.nbForTrying();

    if((msg->getFinalDestAddr1()!=2540) && (msg->getFinalDestAddr1()!=-1))
    {
        LS2.sender=(int)(myNetwAddr);
        LS2.receiver=msg->getFinalDestAddr1();
        LS2.posture=posture;
        int nbT2=LS2.nbForTrying();

        if(nbT2>nbT1)
            nbT = nbT1;
        else
            nbT = nbT2;
    }
    else
        nbT= nbT1;

    for (int i =0; i<nbT; i++)
    {
        msgCopy =msg->dup();
        if(myNetwAddr != msgCopy->getInitialSrcAddr())
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

cMessage* CnSm1::decapsMsg(CnSm1Pkt *msg)
{
    cMessage *m = msg->decapsulate();
    setUpControlInfo(m, msg->getSrcAddr());
    nbHops = nbHops + msg->getNbHops();
    delete msg;
    return m;
}

