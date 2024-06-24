//transmission.cc
#include "network.h"
#include "synchlist.h"
#include "postTCP.h"
#include "transmission.h"

#define MaxIPSize 	(MaxTCPSize - sizeof(IPHeader))  //For the '\0'


TransmissionPost::TransmissionPost(TCPPostOffice *tcpPostOffice1)
{
// First, initialize the tcp post office which take care of all the transmission part
    tcpPostOffice = tcpPostOffice1;
}

TransmissionPost::~TransmissionPost()
{
}

int
TransmissionPost::Send(Message message)
{
    int rtValue;
    unsigned int dataLen = strlen(message.data); 
    int nbTCPMail = dataLen/MaxIPSize;
    if(dataLen%MaxIPSize!=0) {
        nbTCPMail++;
    }


    DEBUG('w',"Nb Messages : %d car MAXIPSIZE : %d et nb char = %d\n",nbTCPMail, MaxIPSize,dataLen);
    fflush(stdout);

    IPHeader ipHdr;
    TCPHeader tcpHdr;
    MailHeader mailHdr;
    PacketHeader pktHdr;

    //We create our Headers 
    ipHdr.id = message.id;
    ipHdr.size_data = dataLen;
    ipHdr.num_total_pkt = nbTCPMail;

    tcpHdr.type=TYPE::MESSAGE;

    mailHdr.to=message.destination.boxAddr;
    mailHdr.from=message.sender.boxAddr;

    pktHdr.to=message.destination.netAddr;
    for(int i=0; i<nbTCPMail; i++){
        ipHdr.num_current_pkt=i+1;
        if (DebugIsEnabled('w'))
            {
                DEBUG('w',"I send the %d packet\n", ipHdr.num_current_pkt);
                fflush(stdout);
                DEBUG('w',"The fucking i :%d\n",i);
                fflush(stdout);
                DEBUG('w',"The fucking size of IPHeader :%d\n",sizeof(IPHeader));
                fflush(stdout);
            }
        //We handle the division and put our 
        //part of the data in the buffer concateneted
        //with the IPHeader
        char buffer[MaxTCPSize];
        bcopy(&ipHdr, buffer, sizeof(IPHeader));
        if(i==nbTCPMail-1 && dataLen%MaxIPSize!=0){
            bcopy(message.data+(i*MaxIPSize),buffer+sizeof(IPHeader),dataLen%MaxIPSize);
            // buffer[dataLen%MaxIPSize - 1] = '\0';
        }else{
            bcopy(message.data+(i*MaxIPSize),buffer+sizeof(IPHeader),MaxIPSize);
            // buffer[MaxTCPSize - 1] = '\0';
        }


        tcpHdr.length=MaxTCPSize;

        mailHdr.length = tcpHdr.length + sizeof(TCPHeader);
            if (DebugIsEnabled('w'))
            {
                DEBUG('w',"Data are : %s\n", buffer+sizeof(IPHeader));
                fflush(stdout);
            }
        rtValue = tcpPostOffice->Send(tcpHdr,pktHdr,mailHdr,buffer);
        //On transmets l'intégralité de nos messages et si on a un echec on 
        //retourne -1 instant (on arrête l'envoi total)
        if(rtValue == -1) break;
    }
    //On retourne 0 si tout a été envoyé avec succès
    return rtValue;
}


//Bloquant jusqu'à avoir un message intégral
void
TransmissionPost::Receive(int box, Message* message)
{
    IPHeader inIPHdr;
    TCPHeader inTCPHdr;
    MailHeader inMailHdr;
    PacketHeader inPktHdr;

    char buffer[MaxTCPSize];
    tcpPostOffice->Receive(box,&inTCPHdr,&inPktHdr,&inMailHdr,buffer);
    bcopy(buffer, &inIPHdr, sizeof(IPHeader));
    //For now our buffer still contains our IPHeader bits at his
    //Front

    //We remove every message (deleting them) until
    //we got a first IPmessage (with numCurrent = 0)
    while (inIPHdr.num_current_pkt!=1)
    {
        //A voir peut être edge case ???
        tcpPostOffice->Receive(box,&inTCPHdr,&inPktHdr,&inMailHdr,buffer);
        bcopy(buffer, &inIPHdr, sizeof(IPHeader));
        if (DebugIsEnabled('w'))
        {
            DEBUG('w',"Damn it ! there is something wrong with that stuff !\n");
            fflush(stdout);
        }
    }
    
    //Here our current information are those of the first IPMessage

    int currentMessageID = inIPHdr.id;
    //We create our global buffer which will contains all our data
    //While we did not have our last IPMessage (numCurrent == numTotal)
    //We had our data (unless "doublons", which are ignored

    char* globalBuffer = new char[inIPHdr.size_data+1];

    unsigned int currentNum = 1;
    while(inIPHdr.num_current_pkt<inIPHdr.num_total_pkt)
    {
        if (DebugIsEnabled('w'))
        {
            DEBUG('w',"We received the %d packet !\n",inIPHdr.num_current_pkt);
            fflush(stdout);
            DEBUG('w',"The num expected is %d\n",currentNum);
            fflush(stdout);
            DEBUG('w',"The id expected is %d and we received the id %d\n",currentMessageID,inIPHdr.id);
            fflush(stdout);
        }
        //Test if we are still with the same IPMessage
        //For now we delete every other message that are none those 
        //that we want so WATCHOUT !!!!!!
        if(inIPHdr.id==currentMessageID && inIPHdr.num_current_pkt==currentNum){
            bcopy(buffer+sizeof(IPHeader),globalBuffer + (inIPHdr.num_current_pkt-1)*MaxIPSize,MaxIPSize);
            currentNum++;
        }
        tcpPostOffice->Receive(box,&inTCPHdr,&inPktHdr,&inMailHdr,buffer);
        bcopy(buffer, &inIPHdr, sizeof(IPHeader));
        if(inIPHdr.id!=currentMessageID && inIPHdr.num_current_pkt==1) {
            globalBuffer = new char[inIPHdr.size_data];
            currentMessageID = inIPHdr.id;
            currentNum=1;
        };
        DEBUG('w', "the num is : %d\n", inIPHdr.num_current_pkt);
        fflush(stdout);
    }
    DEBUG('w', "the data are : %s with length : %d at position : %d\n", buffer+sizeof(IPHeader), inIPHdr.size_data%MaxIPSize, (inIPHdr.num_total_pkt-1)*MaxIPSize);
    fflush(stdout);
    //We don't have a -1 because we still copy the '\0'
    if (inIPHdr.size_data % MaxIPSize != 0)
    {
        bcopy(buffer + sizeof(IPHeader), globalBuffer + (inIPHdr.num_total_pkt - 1) * MaxIPSize,
              inIPHdr.size_data % MaxIPSize);
    }
    else
    {
        bcopy(buffer + sizeof(IPHeader), globalBuffer + (inIPHdr.num_total_pkt - 1) * MaxIPSize,
              MaxIPSize);
    }
    globalBuffer[inIPHdr.size_data] = '\0';
    //Here our globalBuffer contains all the data

    //We recreate our message
    message->destination.boxAddr=inMailHdr.to;
    message->destination.netAddr=inPktHdr.to;

    message->sender.boxAddr=inMailHdr.from;
    message->sender.netAddr=inPktHdr.from;

    message->id=inIPHdr.id;
    message->data=globalBuffer;

}

void Message::Print() {
    printf("Message : From (%d, %d) to (%d, %d) id : %d, data : %s",
           sender.netAddr, sender.boxAddr, destination.netAddr, destination.boxAddr, id, data);
}