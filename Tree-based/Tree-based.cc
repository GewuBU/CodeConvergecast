
#include "CnSmOpt.h"

#include <cassert>
#include <stdlib.h>
#include "NetwControlInfo.h"
#include "CnSmPkt_m.h"
#include "SimTracer.h"
#include "LogNormalShadowing.h"

Define_Module(CnSmOpt);

void CnSmOpt::initialize(int stage)
{
    BaseNetwLayer::initialize(stage);

    if(stage == 1) {
        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 6;

        maxNetQueueSize=par("maxNetQueueSize");
        posture=par("posture");
        appLength=par("appLength");
        headerLength=par("headerLength");
        sinkAddress =1;
        floodSeqNumber = 0;
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
        nbDesquence0 = 0;
        nbDesquence2 = 0;
        nbDesquence3 = 0;
        nbDesquence4 = 0;
        nbDesquence5 = 0;
        nbDesquence6 = 0;
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
        check=0;
        nbHopN=0;
        i=0;
        rep=0;
        k=0;
        rep1=0;
        t005 =0;
        t010=0;
        t015=0;
        t020=0;
        t025=0;
        t030=0;
        t035=0;
        t040=0;
        t045=0;
        t050=0;
        t055=0;
        t060=0;
        t065=0;
        t070=0;
        t075=0;
        t080=0;
        t085=0;
        t090=0;
        t095=0;
        t100=0;
        t105=0;
        t110=0;
        t115=0;
        t120=0;
        t125=0;
        t130=0;
        t135=0;
        t140=0;
        t145=0;
        t150=0;
        t155=0;
        t160=0;
        t165=0;
        t170=0;
        t175=0;
        t180=0;

        for (i=0; i<7; i++){
            for(int j =0; j<5;j++){
                tabmsgrec[i][j]=-1;
            }
        }

        i=0;
        if(posture == 1)
        {
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=1; tab[0][4]=5; tab[0][5]=6; tab[0][6]=0;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=1; tab[1][3]=2; tab[1][4]=4; tab[1][5]=1; tab[1][6]=2;
        }
        else if (posture == 2)
        {
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=1; tab[0][4]=5; tab[0][5]=6; tab[0][6]=1;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=3; tab[1][3]=2; tab[1][4]=1; tab[1][5]=5; tab[1][6]=2;
        }
        else if (posture == 3)
        {
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=1; tab[0][4]=5; tab[0][5]=6; tab[0][6]=0;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=1; tab[1][3]=2; tab[1][4]=7; tab[1][5]=1; tab[1][6]=1;
        }
        else if (posture == 4)
        {
            /*
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=1; tab[0][4]=5; tab[0][5]=6; tab[0][6]=3;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=1; tab[1][3]=1; tab[1][4]=2; tab[1][5]=2; tab[1][6]=1;
             */
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=1; tab[0][4]=5; tab[0][5]=6; tab[0][6]=0;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=1; tab[1][3]=1; tab[1][4]=2; tab[1][5]=1; tab[1][6]=1;
        }
        else if (posture == 5)
        {
            /*
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=2; tab[0][4]=6; tab[0][5]=6; tab[0][6]=0;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=1; tab[1][3]=2; tab[1][4]=3; tab[1][5]=2; tab[1][6]=3;
             */
            /*
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=2; tab[0][4]=6; tab[0][5]=6; tab[0][6]=3;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=1; tab[1][3]=2; tab[1][4]=3; tab[1][5]=2; tab[1][6]=1;
            */

            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=2; tab[0][4]=6; tab[0][5]=6; tab[0][6]=3;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=2; tab[1][3]=2; tab[1][4]=4; tab[1][5]=3; tab[1][6]=2;
        }
        else if (posture == 6)
        {
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=6; tab[0][4]=5; tab[0][5]=3; tab[0][6]=1;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=7; tab[1][3]=1; tab[1][4]=1; tab[1][5]=7; tab[1][6]=7;
        }
        else if (posture == 7)
        {
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=2; tab[0][4]=5; tab[0][5]=1; tab[0][6]=2;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=1; tab[1][3]=6; tab[1][4]=2; tab[1][5]=9; tab[1][6]=8;
        }
        else if (posture == 8)
        {
            tab[0][0]=1; tab[0][1]=-1; tab[0][2]=1; tab[0][3]=1; tab[0][4]=5; tab[0][5]=6; tab[0][6]=0;
            tab[1][0]=1; tab[1][1]=-1; tab[1][2]=1; tab[1][3]=3; tab[1][4]=2; tab[1][5]=2; tab[1][6]=3;
        }


        //BGW
        //these parameters are only needed for plain flooding
        hasPar("bcMaxEntries") ? bcMaxEntries = par("bcMaxEntries") : bcMaxEntries = 30;
        //BGW
        hasPar("bcDelTime") ? bcDelTime = par("bcDelTime") : bcDelTime = 180.0;
        EV <<"bcMaxEntries = "<<bcMaxEntries
                <<" bcDelTime = "<<bcDelTime<<endl;

    }
}

CnSmOpt::~CnSmOpt()
{
    cancelAndDelete(0);
}

void CnSmOpt::handleLowerMsg(cMessage* msg)
{
    CnSmPkt*           netwMsg        = check_and_cast<CnSmPkt*>(msg);
    const LAddress::L3Type& finalDestAddr  = netwMsg->getFinalDestAddr();
    //const LAddress::L3Type& finalDestAddr1  = netwMsg->getFinalDestAddr1();
    dataReceived++;
    EV<<"ttl "<<netwMsg->getTtl()<<endl;
    // if (finalDestAddr!=myNetwAddr && finalDestAddr1!=myNetwAddr) { // La erreur du code
    if (finalDestAddr!=myNetwAddr){                                                                        // Le code correct
        delete netwMsg;
    }
    else{
        const cObject* pCtrlInfo = NULL;
        CnSmPkt* msgCopy;
        msgCopy = netwMsg;
        if (netwMsg->getKind()==DATA && myNetwAddr!=1){
            if (netwMsg->getTtl()>1 && notBroadcasted(netwMsg))
            {
                msgCopy = check_and_cast<CnSmPkt*>(netwMsg->dup());
                netwMsg->setSrcAddr(myNetwAddr);

                netwMsg->setFinalDestAddr(tab[0][myNetwAddr]);


                netwMsg->setTtl( netwMsg->getTtl()-1 );
                pCtrlInfo = netwMsg->removeControlInfo();
                //BGW
                //setDownControlInfo(netwMsg, LAddress::L2BROADCAST);
                //forwardControl(netwMsg);
                //sendDown(netwMsg);
                multiSend(netwMsg);

                EV<<"je suis le noeud: "<<myNetwAddr<< ", j'ai reçu un msg de type: DATA de la part de :"<<netwMsg->getInitialSrcAddr()<<endl;
                EV<<"nbDataPacketsForwarded++; "<< nbDataPacketsForwarded << endl;
            }
            else
            {
                EV<<"delete msg because ttl "<<endl;
            }
        }
        if (netwMsg->getKind() == DATA && netwMsg->getFinalDestAddr()==1 && myNetwAddr==1) {
            const LAddress::L3Type& initSrcAdr  = netwMsg->getInitialSrcAddr();
            EV<<"initSrcAdr= "<< initSrcAdr<<endl;
            int seq =netwMsg->getSeqNum();
            if(notBroadcasted(msgCopy)){
                EV<<"je suis le noeud 1 j'ai reçu un msg de type DATA # " << seq << " de la part de :"<<netwMsg->getInitialSrcAddr()<<", par l'intermediare de : "<<netwMsg->getSrcAddr()<<endl;
                recuControl(msgCopy);
                TimeDis(msgCopy);
                sendUp(decapsMsg(msgCopy));
                nbDataPacketsReceived++;
                EV<<"nbDataPacketsReceived++; "<<nbDataPacketsReceived<<endl;
            }else{
                //EV<<"je suis le noeud 1 j'ai reçu un msg de type DATA de la part de :"<<netwMsg->getInitialSrcAddr()<<", par l'intermediare de : "<<netwMsg->getSrcAddr()<<", et je l'ai supppp"<<endl;;
                delete msgCopy;
                //for (i=0; i<7;i++){
                //EV<<"tabmsgrec[initSrcAdr] = "<<tabmsgrec[initSrcAdr]<<endl;
                //}
            }
        }else
            delete msgCopy;

        if (pCtrlInfo != NULL)
            delete pCtrlInfo;
    }
}

void CnSmOpt::handleLowerControl(cMessage *msg)
{
    delete msg;
}

void CnSmOpt::handleUpperMsg(cMessage* msg)
{
    //BGW
    const cObject* pCtrlInfo = NULL;

    EV<<"UUUUUUUUUUUUUUUUUUUUUUUUUPPPPPPPPPPPPPPPPEEEERRRRRRRRRRRR : "<<myNetwAddr<<endl;
    EV<<"ma table contient:"<<endl;
    for (i=0;i<=5; i++)
        EV<<tab[i]<<endl;
    i=0;
    CnSmPkt*    pkt   = new CnSmPkt(msg->getName(), DATA);

    pkt->setFinalDestAddr(tab[0][myNetwAddr]);
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

    //setDownControlInfo(pkt, LAddress::L2BROADCAST);
    //sendDown(pkt);
    multiSend(pkt);

    //BGW
    if (pCtrlInfo != NULL)
        delete pCtrlInfo;

}

void CnSmOpt::finish()
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
    recordScalar("nbDesquence0", nbDesquence0);
    recordScalar("nbDesquence2", nbDesquence2);
    recordScalar("nbDesquence3", nbDesquence3);
    recordScalar("nbDesquence4", nbDesquence4);
    recordScalar("nbDesquence5", nbDesquence5);
    recordScalar("nbDesquence6", nbDesquence6);
    recordScalar("NbT005", t005);
    recordScalar("NbT015", t015);
    recordScalar("NbT025", t025);
    recordScalar("NbT035", t035);
    recordScalar("NbT045", t045);
    recordScalar("NbT055", t055);
    recordScalar("NbT065", t065);
    recordScalar("NbT075", t075);
    recordScalar("NbT085", t085);
    recordScalar("NbT095", t095);
    recordScalar("NbT105", t105);
    recordScalar("NbT115", t115);
    recordScalar("NbT125", t125);
    recordScalar("NbT135", t135);
    recordScalar("NbT145", t145);
    recordScalar("NbT155", t155);
    recordScalar("NbT165", t165);
    recordScalar("NbT175", t175);
    recordScalar("NbT010", t010);
    recordScalar("NbT020", t020);
    recordScalar("NbT030", t030);
    recordScalar("NbT040", t040);
    recordScalar("NbT050", t050);
    recordScalar("NbT060", t060);
    recordScalar("NbT070", t070);
    recordScalar("NbT080", t080);
    recordScalar("NbT090", t090);
    recordScalar("NbT100", t100);
    recordScalar("NbT110", t110);
    recordScalar("NbT120", t120);
    recordScalar("NbT130", t130);
    recordScalar("NbT140", t140);
    recordScalar("NbT150", t150);
    recordScalar("NbT160", t160);
    recordScalar("NbT170", t170);
    recordScalar("NbT180", t180);


    recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);

    NetQueue::iterator it;
    for (it = netQueue.begin(); it != netQueue.end(); ++it) {
        delete (*it);
    }
}

bool CnSmOpt::notBroadcasted(CnSmPkt* msg) {

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
            nbDesquence0++;
            nbDesquence++;
        }
        Deseqence0=msg->getSeqNum();
        break;
    case 2 :
        if(msg->getSeqNum() < Deseqence2)
        {
            nbDesquence2++;
            nbDesquence++;
        }
        Deseqence2=msg->getSeqNum();
        break;
    case 3 :
        if(msg->getSeqNum() < Deseqence3)
        {
            nbDesquence3++;
            nbDesquence++;
        }
        Deseqence3=msg->getSeqNum();
        break;
    case 4 :
        if(msg->getSeqNum() < Deseqence4)
        {
            nbDesquence4++;
            nbDesquence++;
        }
        Deseqence4=msg->getSeqNum();
        break;
    case 5 :
        if(msg->getSeqNum() < Deseqence5)
        {
            nbDesquence5++;
            nbDesquence++;
        }
        Deseqence5=msg->getSeqNum();
        break;
    case 6 :
        if(msg->getSeqNum() < Deseqence6)
        {
            nbDesquence6++;
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

void CnSmOpt::forwardControl(CnSmPkt *msg)
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

void CnSmOpt::recuControl(CnSmPkt *msg)
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

void CnSmOpt::multiSend(CnSmPkt* msg)
{
    //BGW
    CnSmPkt* msgCopy;
    CnSmPkt* msgCont;
    int nbT;
    nbT= tab[1][myNetwAddr];

    msg->setByteLength(appLength);
    msgCont =msg->dup();

    nbDataPacketsForwarded++;
    forwardControl(msgCont);


    for (int i =0; i<nbT; i++)
    {
        msgCopy =msg->dup();
        setDownControlInfo(msgCopy, LAddress::L2BROADCAST);
        int b = msgCopy->getByteLength();
        sendDown(msgCopy);
        nbDataPacketsSent++;
    }
}

cMessage* CnSmOpt::decapsMsg(CnSmPkt *msg)
{
    cMessage *m = msg->decapsulate();
    setUpControlInfo(m, msg->getSrcAddr());
    nbHops = nbHops + msg->getNbHops();
    delete msg;
    return m;
}

void CnSmOpt::TimeDis(CnSmPkt *msg)
{
    simtime_t time = msg->getCreationTime();
    if (time<5)
        t005 ++;
    else if(time < 10)
        t010 ++;
    else if(time < 15)
        t015 ++;
    else if(time < 20)
        t020 ++;
    else if(time < 25)
        t025 ++;
    else if(time < 30)
        t030 ++;
    else if(time < 35)
        t035 ++;
    else if(time < 40)
        t040 ++;
    else if(time < 45)
        t045 ++;
    else if(time < 50)
        t050 ++;
    else if(time < 55)
        t055 ++;
    else if(time < 60)
        t060 ++;
    else if(time < 65)
        t065 ++;
    else if(time < 70)
        t070 ++;
    else if(time < 75)
        t075 ++;
    else if(time < 80)
        t080 ++;
    else if(time < 85)
        t085 ++;
    else if(time < 90)
        t090 ++;
    else if(time < 95)
        t095 ++;
    else if(time < 100)
        t100 ++;
    else if(time < 105)
        t105 ++;
    else if(time < 110)
        t110 ++;
    else if(time < 115)
        t115 ++;
    else if(time < 120)
        t120 ++;
    else if(time < 125)
        t125 ++;
    else if(time < 130)
        t130 ++;
    else if(time < 135)
        t135 ++;
    else if(time < 140)
        t140 ++;
    else if(time < 145)
        t145 ++;
    else if(time < 150)
        t150 ++;
    else if(time < 155)
        t155 ++;
    else if(time < 160)
        t160 ++;
    else if(time < 165)
        t165 ++;
    else if(time < 170)
        t170 ++;
    else if(time < 175)
        t175 ++;
    else if(time < 180)
        t180 ++;
}
