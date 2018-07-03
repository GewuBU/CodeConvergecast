
#include "Hybrid.h"

#include <cassert>
#include <stdlib.h>
#include "NetwControlInfo.h"
#include "CnSmPkt_m.h"
#include "SimTracer.h"

Define_Module(Hybrid);

void Hybrid::initialize(int stage)
{
    BaseNetwLayer::initialize(stage);

    if(stage == 1) {
        hasPar("defaultTtl") ? defaultTtl = par("defaultTtl") : defaultTtl = 6;
                EV<< "defaultTtl = " << defaultTtl<< endl;

        nbDataPacketsForwarded = 0;
        nbDataPacketsReceived = 0;
        nbDataPacketsSent = 0;
        dataReceived=0;
        nbHops = 0;
        pong =0;
        nbPINGsent=0;
        nbPINGrecv=0;
        nbPONGsent=0;
        nummess=0;
        nbPONGrecv=0;
        NbDiffMax=1;

    for (int s=0; s<7; s++)
            tabmsgrec[s]=-1;

// Un tableau prédéfini qui donne pour chaque noeud ses parents
        if(myNetwAddr==0){ tab[0]=1; tab[1]=-25; tab[2]=6; tab[3]=-25; tab[4]=-25; tab[5]=-25;}
        if(myNetwAddr==2){ tab[0]=1; tab[1]=-25; tab[2]=3; tab[3]=-25; tab[4]=-25; tab[5]=-25;}
        if(myNetwAddr==3){ tab[0]=1; tab[1]=-25; tab[2]=2; tab[3]=-25; tab[4]=-25; tab[5]=-25;}
        if(myNetwAddr==4){ tab[0]=5; tab[1]=-25; tab[2]=-25; tab[3]=-25; tab[4]=-25; tab[5]=-25;}
        if(myNetwAddr==5){ tab[0]=0; tab[1]=-25; tab[2]=6; tab[3]=-25; tab[4]=-25; tab[5]=-25;}
        if(myNetwAddr==6){ tab[0]=0; tab[1]=-25; tab[2]=3; tab[3]=-25; tab[4]=-25; tab[5]=-25;}

    }
}

Hybrid::~Hybrid()
{
    cancelAndDelete(0);
}

void Hybrid::handleLowerMsg(cMessage* msg)
{
    CnSmPkt*           netwMsg        = check_and_cast<CnSmPkt*>(msg);
//Je récupère les addresses de destinations
    const LAddress::L3Type& finalDestAddr  = netwMsg->getFinalDestAddr();
    const LAddress::L3Type& finalDestAddr1  = netwMsg->getFinalDestAddr1();
    EV<<" me "<<myNetwAddr<<" initialsrc "<<netwMsg->getInitialSrcAddr()<< " src "<<netwMsg->getSrcAddr()<<" finaldestAddr1 "<<finalDestAddr1<<" et finalDest "<<finalDestAddr<<endl;

    EV<<"ttl "<<netwMsg->getTtl()<<endl;

    if (netwMsg->getKind()==PING)
    {
        EV<< "Ping message de "<< netwMsg->getInitialSrcAddr()<<endl;

//Si je suis l'une des destinations de ce PING, je renvoie un message de type PONG
        if (finalDestAddr==myNetwAddr || finalDestAddr1==myNetwAddr || myNetwAddr==1)
        {
            EV<<" Le PING est à moi " <<myNetwAddr<<" de "<<netwMsg->getInitialSrcAddr() << endl;
            CnSmPkt* Pmsg= new CnSmPkt("Pong", PONG);
            Pmsg->setInitialSrcAddr(myNetwAddr);
            Pmsg->setByteLength(headerLength);
            Pmsg->setSrcAddr(myNetwAddr);
            Pmsg->setFinalDestAddr(netwMsg->getSrcAddr());
            Pmsg->setFinalDestAddr1(netwMsg->getSrcAddr());
            setDownControlInfo(Pmsg, LAddress::L2BROADCAST);
            sendDown(Pmsg);
            nbPONGsent++;
            delete netwMsg;
        }
        else
        {
            EV<<" Le PING n'est pas à moi " <<myNetwAddr<<" pour "<<finalDestAddr <<" Dest1 "<< finalDestAddr1<<endl;
            delete netwMsg;
        }

    }
    else if (netwMsg->getKind()==PONG)
    {
        EV<< "Pong message de "<< netwMsg->getInitialSrcAddr()<<endl;
        if (finalDestAddr==myNetwAddr && finalDestAddr1==myNetwAddr)
        {
            EV<<"Here pong received "<<endl;

            pong=NbDiffMax;
            if (waitMap.size()!=0)
                {
                    EV<<" enter sendmessage()"<<endl;
                    sendmessages();
                }

            nbPONGrecv++;
            delete netwMsg;
        }
        else
        {
            EV<<" Le PONG n'est pas à moi " <<myNetwAddr<<" pour est "<<finalDestAddr <<" Dest1 "<< finalDestAddr1<<endl;
            delete netwMsg;
        }

    }

    else
    {
        EV<<" Data: me "<<myNetwAddr<<" finaldestAddr1 "<<finalDestAddr1<<" et finalDest "<<finalDestAddr<<endl;

        dataReceived++;
    if (finalDestAddr!=myNetwAddr && finalDestAddr1!=myNetwAddr && finalDestAddr1!=LAddress::L3BROADCAST && myNetwAddr!=1) {
        EV<<" je supprime le message car il est ni un msg de diffusion ni à moi "<<endl;
        delete netwMsg;

    }else {

        const cObject* pCtrlInfo = NULL;
        CnSmPkt* msgCopy;
        msgCopy = check_and_cast<CnSmPkt*>(netwMsg->dup());

        if (myNetwAddr!=1)
        { // Si je ne suis pas le sink, je diffuse le message reçu à mes parents selon la table initialisée à la fonction initialize()
            if (netwMsg->getTtl()>1)
            {
               netwMsg->setFinalDestAddr(tab[0]);
               netwMsg->setFinalDestAddr1(tab[2]);
               netwMsg->setSrcAddr(myNetwAddr);
               netwMsg->setTtl( netwMsg->getTtl()-1 );
               pCtrlInfo = netwMsg->removeControlInfo();
               setDownControlInfo(netwMsg, LAddress::L2BROADCAST);
               sendDown(netwMsg);
               nbDataPacketsForwarded++;

               EV<<"je suis le noeud: "<<myNetwAddr<< ", j'ai reçu un msg de type: DATA de la part de :"<<netwMsg->getInitialSrcAddr()<<endl;

            }
            else
            {
                EV<<"delete msg because ttl = 1 "<<endl;
            }
        }
        else {
               EV<<"je suis le noeud 1 (le sink) "<<endl;
               const LAddress::L3Type& initSrcAdr  = netwMsg->getInitialSrcAddr();
               EV<<"initSrcAdr= "<< initSrcAdr<<endl;
               if(tabmsgrec[initSrcAdr]==-1){ // c'est la première fois que je reçois un message de cette source
                   EV<<"je suis le noeud 1 j'ai reçu un msg de type DATA de la part de :"<<netwMsg->getInitialSrcAddr()<<", par l'intermediare de : "<<netwMsg->getSrcAddr()<<endl;
                   sendUp(decapsMsg(msgCopy)); // Pour envoyer le message à la couche supérieure
                   nbDataPacketsReceived++;
                   tabmsgrec[initSrcAdr]=1;
                   EV<<"nbDataPacketsReceived++; "<<nbDataPacketsReceived<<endl;
               }else{
                   EV<<"je suis le noeud 1 j'ai reçu un msg de type DATA de la part de :"<<netwMsg->getInitialSrcAddr()<<", par l'intermediare de : "<<netwMsg->getSrcAddr()<<", et je l'ai supprimé"<<endl;;
                   delete msgCopy;
                       EV<<"tabmsgrec[initSrcAdr] = "<<tabmsgrec[initSrcAdr]<<endl;

               }
           }

        if (pCtrlInfo != NULL)
            delete pCtrlInfo;
    }
    } // fin netwMsg Data

}

void Hybrid::handleLowerControl(cMessage *msg)
{
    delete msg;
}


//Cette fonction gère les messages des couches supérieures
void Hybrid::handleUpperMsg(cMessage* msg)
{
    EV<<"ma table contient:"<<endl;

   // Pour connaitre mes parents
    for (i=0;i<=5; i++)
    {
        EV<<tab[i]<<endl;
    }
	////////////construction du paquet
    CnSmPkt*    pkt   = new CnSmPkt(msg->getName(), DATA);
    assert(static_cast<cPacket*>(msg));
    pkt->encapsulate(static_cast<cPacket*>(msg));
    pkt->setFinalDestAddr(tab[0]);
    pkt->setFinalDestAddr1(tab[2]);
    pkt->setInitialSrcAddr(myNetwAddr);
    pkt->setSrcAddr(myNetwAddr);
    pkt->setTtl(defaultTtl);
    setDownControlInfo(pkt, LAddress::L2BROADCAST);

//////// Insérer le message en attendant le PONG
    EV << "Insérer le message en attendant le PONG"<<endl;
    insertmessage(pkt);
    nbDataPacketsSent++;
    EV<<"nbDataPacketsSent++; "<<nbDataPacketsSent<<endl;
      ///////////


    // Pinguer le parent
        CnSmPkt* Pmsg= new CnSmPkt("Ping", PING);
        Pmsg->setByteLength(headerLength);
        Pmsg->setInitialSrcAddr(myNetwAddr);
        Pmsg->setSrcAddr(myNetwAddr);
        Pmsg->setFinalDestAddr(tab[0]);
        Pmsg->setFinalDestAddr1(tab[2]);
        Pmsg->setTtl(1);
        setDownControlInfo(Pmsg, LAddress::L2BROADCAST);
        sendDown(Pmsg);
        nbPINGsent++;

//Declencher le Timer en attendant le PONG du parent ou du sink
        cMessage *PONGTIMER = new cMessage("PONGTIMER", PONG_TIMER);
        scheduleAt(simTime() + 0.170 , PONGTIMER);
        EV<<"Fin handleUpperMessage "<<endl;



}


void Hybrid::insertmessage(CnSmPkt *msg)
{
    nummess=0;
    EV<<" waitMap size before insertmessage "<<waitMap.size()<<endl;
    pair<int, CnSmPkt*> pairToInsert = make_pair(nummess, msg);
    pair<waitingMap::iterator, bool> ret = waitMap.insert(pairToInsert);
    EV<<"I'm node  "<<myNetwAddr<<" and waitMap size after insertmessage  "<<waitMap.size()<<endl;
   // nummess = nummess+1; // si on ajoute cette ligne, on aura un grand nombre de message en attente
    // ces messages correspondent à differentes copies du message original avec évidemment différents TTL
    
}


void Hybrid::sendmessages()
{
    EV<<"I'm node  "<<myNetwAddr<<"  and I'm sending all messages in my table"<<endl;
    CnSmPkt *msgo;
    waitingMap::iterator it;

   EV<<"map size before sendmessages =  "<<waitMap.size()<<endl;

//Si je ne reçois pas un pong je diffuse le message NbDiffMax fois
   if (pong<NbDiffMax)
                {
                   for (it=waitMap.begin(); it!=waitMap.end(); it++)
                        {
               msgo = it->second;
               CnSmPkt *dMsg;
               dMsg = static_cast<CnSmPkt*>(msgo->dup());
               setDownControlInfo(dMsg, LAddress::L2BROADCAST);
               dMsg->setFinalDestAddr1(LAddress::L3BROADCAST);
               EV<<" me "<<myNetwAddr<<" dest "<<dMsg->getFinalDestAddr()<<" dest1 "<<dMsg->getFinalDestAddr1()<<endl;
               sendDown(dMsg);
               nbDataPacketsForwarded++;
               EV<<"nbDataPacketsForwarded égal à "<< nbDataPacketsForwarded << endl;
                        }
                   pong++;

                //renvoyer un ping au parents
		if (pong < NbDiffMax)
		{
                EV<<"pong égal "<<pong<<endl;
                CnSmPkt* Pmsg= new CnSmPkt("Ping", PING);
                Pmsg->setByteLength(headerLength);
                Pmsg->setInitialSrcAddr(myNetwAddr);
                Pmsg->setSrcAddr(myNetwAddr);
                Pmsg->setFinalDestAddr(tab[0]);
                Pmsg->setFinalDestAddr1(tab[2]);
                Pmsg->setTtl(1);
                setDownControlInfo(Pmsg, LAddress::L2BROADCAST);
                sendDown(Pmsg);
                nbPINGsent++;

                cMessage *PONGTIMER = new cMessage("PONGTIMER", PONG_TIMER);
                scheduleAt(simTime() + 0.170 , PONGTIMER);

                ///////////// Fin renvoyer Ping
}


       }


/// pong==NbDiffMax ça veut dire j'ai reçu un pong de mes parents
        else if (pong==NbDiffMax)
{
            for (it=waitMap.begin(); it!=waitMap.end(); it++)
                       {
              msgo = it->second;
              CnSmPkt *dMsg;
              dMsg = static_cast<CnSmPkt*>(msgo->dup());
              setDownControlInfo(dMsg, LAddress::L2BROADCAST);
              EV<<" me "<<myNetwAddr<<" dest "<<dMsg->getFinalDestAddr()<<" dest1 "<<dMsg->getFinalDestAddr1()<<endl;
              sendDown(dMsg);
              nbDataPacketsForwarded++;
              EV<<"nbDataPacketsForwarded égal à "<< nbDataPacketsForwarded << endl;
                       }
        waitMap.clear();
        nummess=0;

        EV<<"map size after sendmessages = "<<waitMap.size()<<endl;
}

}

//Cette fonction permet d'enregistrer les valeurs finales des différentes variables dans un fichier
void Hybrid::finish()
{
    //BaseNetwLayer::bcMsgs.clear();
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

        recordScalar("meanNbHops", (double) nbHops / (double) nbDataPacketsReceived);

}

cMessage* Hybrid::decapsMsg(CnSmPkt *msg)
{
    cMessage *m = msg->decapsulate();
    setUpControlInfo(m, msg->getInitialSrcAddr());
    nbHops = nbHops + msg->getNbHops();
    delete msg;
    return m;
}


// Quand le Timer expire, il appelle la fonction cette fonction
void Hybrid::handleSelfMsg(cMessage *msg)
{
   EV<<"waitMap.size() égal " <<waitMap.size()<<endl;
   if (pong<NbDiffMax)
                   {
    if (waitMap.size()!=0)
    {
        EV<<" enter sendmessage()"<<endl;
        sendmessages();
    }
                   }

}

