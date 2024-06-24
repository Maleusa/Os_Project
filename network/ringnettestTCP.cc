#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"


// This test file test interaction with more than one machine
// By using -l X option of nachos-mynetwork, you can test resending
// without block the program since you'll avec X^MaxEmissions chances
// to loose a packet and don't resend it.

// The machine 0 begins by sending a message to machine 1, which
// send then this message to machine 2 etc...
// The last machine send the message to machine 0 who then stop
// the test.

void
TCPTestRing(int dest)
{
    printf(">>>> Test 1 de TCP\n");
    PacketHeader outPktHdr, inPktHdr;
    TCPHeader outTCPHeader, inTCPHeader;
    MailHeader outMailHdr, inMailHdr;
    const char *data = "I'm a the big packet";
    char buffer[MaxTCPSize];

    outPktHdr.to = dest;
    outMailHdr.to = 1;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1 + sizeof(TCPHeader);
    outTCPHeader.type = TYPE::MESSAGE;
    outTCPHeader.length = strlen(data) + 1;

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = dest;		
    outMailHdr.to = 1;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1 + sizeof(TCPHeader);

    // Send the first message by machine dest-1
    if (dest-1==0){

        tcpPostOffice->Send(outTCPHeader,outPktHdr, outMailHdr, data);
        printf("I sent first message: \"%s\" to machine %d\n",data, dest);
        fflush(stdout);

        tcpPostOffice->Receive(1, &inTCPHeader,&inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);
    }

    else{
        Delay(2);
        data = NULL;
        // Wait for the first message from the other machine
        tcpPostOffice->Receive(1, &inTCPHeader, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        // Send acknowledgement to the other machine (using "reply to" mailbox
        // in the message that just arrived
        outPktHdr.to = dest;
        outMailHdr.to = 1;
        outMailHdr.length = inMailHdr.length;
        tcpPostOffice->Send(inTCPHeader, outPktHdr, outMailHdr, buffer);
        printf("I sent it to my next neighbor machine %d\n", dest);
        fflush(stdout);
    }

    // Then we're done!
    interrupt->Halt();
}
