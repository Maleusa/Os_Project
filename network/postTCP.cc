// postTCP.cc 


#include "network.h"
#include "synchlist.h"
#include "post.h"
#include "postTCP.h"
#include "system.h"

#include <strings.h> /* for bzero */

struct serial {
    int id;
    PostOffice *p;
};

//----------------------------------------------------------------------
// TCPPostOffice::TCPPostOffice
// 	Initialize a tcp post office as a collection of mailboxes.
//	Also initialize the network device, to allow tcp post offices
//	on different machines to deliver messages to one another.
//
//      We use a separate thread "the postal worker" to wait for messages 
//	to arrive, and deliver them to the correct TCPmailbox.  Note that
//	delivering messages to the TCPmailboxes can't be done directly
//	by the interrupt handlers, because it requires a Lock.
//
//	"addr" is this machine's network ID 
//	"reliability" is the probability that a network packet will
//	  be delivered (e.g., reliability = 1 means the network never
//	  drops any packets; reliability = 0 means the network never
//	  delivers any packets)
//	"nBoxes" is the number of mail boxes in this Post Office
//----------------------------------------------------------------------

TCPPostOffice::TCPPostOffice(PostOffice *po)
{
// First, initialize the post office which take care of all the transmission part
    postOffice = po;
    postOffice->ActivateAutoAck(); // We activate autoAck
}

TCPPostOffice::~TCPPostOffice()
{
}

//----------------------------------------------------------------------
// TCPPostOffice::Send
// 	Concatenate the TCPHeader to the front of the data, and pass
//	the result to the PostOffice for delivery to the Network.
//
//	Note that the TCPHeader + data looks just like normal payload
//	data to the PostOffice.
//
//  "tcpHdr" -- type, alterned bit tcp
//	"pktHdr" -- source, destination machine ID's
//	"mailHdr" -- source, destination mailbox ID's
//	"data" -- payload message data
//----------------------------------------------------------------------

int
TCPPostOffice::Send(TCPHeader tcpHdr, PacketHeader pktHdr, MailHeader mailHdr, const char* data)
{
    char* buffer = new char[MaxPacketSize];	// space to hold concatenated
						// TCPHdr + data

    ASSERT(tcpHdr.length <= MaxTCPSize);
    if (tcpHdr.type == TYPE::MESSAGE) // box 0 is reserved to TCP
        ASSERT(mailHdr.to > 0);

    // concatenate TCPHeader and data
    bcopy(&tcpHdr, buffer, sizeof(TCPHeader));
    bcopy(data, buffer + sizeof(TCPHeader), tcpHdr.length);

    int returnvalue = -1;
    int nbEmission = 0;
    int id;
    char* entry = new char[MaxPacketSize];
    PacketHeader outPktHdr;
    MailHeader outMailHdr;
    TCPHeader outtcpHeader;

    while (nbEmission < MAXEMISSION){
        // Attempt to send message
        postOffice->Send(pktHdr, mailHdr, buffer);
        nbEmission++;
        // Schedule a resend
        id = stats->totalTicks;
        ScheduleReSend(id);
        // Reception of ACK or TIMERSIGNAL
        ReceiveAck(&outtcpHeader, &outPktHdr, &outMailHdr, entry);
        switch (outtcpHeader.type) {
            case TYPE::ACK:
                // ACK received so need to wait for last timer scheduled
                ReceiveAck(&outtcpHeader, &outPktHdr, &outMailHdr, entry);
                ASSERT(outtcpHeader.type == TYPE::TIMERSIGNAL);
                // Then the loop is ended
                returnvalue = 0;
                nbEmission = MAXEMISSION;
                break;
            case TYPE::TIMERSIGNAL:
                // TIMERSIGNAL received so need to send again
                // Then continue loop
                break;
            default:
                ASSERT(false);
                break;
        }

    }

    delete [] buffer;			// we've sent the message, so
					// we can delete our buffer
    return returnvalue;
}

// Function used by the timer
static void do_Notify(int arg){
    struct serial *s = (struct serial *) arg ;
    (s->p)->NotifyOurself(s->id);
}

// Function to schedule a resend of message
void TCPPostOffice::ScheduleReSend(int id){

    if (DebugIsEnabled('w'))
    {
        printf("Schedule to resend message (%d)\n", id);
        fflush(stdout);
    }
    struct serial *s = new serial;
    s->id = id;
    s->p = postOffice;
    interrupt->Schedule(do_Notify,(int) s,TEMPO, TimerInt);
}

//----------------------------------------------------------------------
// TCPPostOffice::Receive
// 	Retrieve a message from a specific box if one is available, 
//	otherwise wait for a message to arrive in the box.
//
//	Note that the TCPHeader + data looks just like normal payload
//	data to the PostOffice.
//
//
//	"box" -- mailbox ID in which to look for message
//  "tcpHdr" -- type, alterned bit tcp
//	"pktHdr" -- address to put: source, destination machine ID's
//	"mailHdr" -- address to put: source, destination mailbox ID's
//	"data" -- address to put: payload message data
//----------------------------------------------------------------------

void
TCPPostOffice::Receive(int box, TCPHeader *tcpHdr, PacketHeader *pktHdr,
				MailHeader *mailHdr, char* data)
{
    ASSERT(box > 0); // box 0 reserved

    char* buffer = new char[MaxPacketSize];
    postOffice->Receive(box, pktHdr, mailHdr, buffer);

    bcopy(buffer, tcpHdr, sizeof(TCPHeader));
    bcopy( buffer + sizeof(TCPHeader), data, tcpHdr->length);
    // copy the message data into
    // the caller's buffer

    ASSERT(mailHdr->length <= MaxMailSize);
}

//----------------------------------------------------------------------
// TCPPostOffice::ReceiveAck
// 	Wait for reception of mail in box 0.
//
//  "tcpHdr" -- type, alterned bit tcp
//	"pktHdr" -- address to put: source, destination machine ID's
//	"mailHdr" -- address to put: source, destination mailbox ID's
//	"data" -- address to put: payload message data
//----------------------------------------------------------------------
void
TCPPostOffice::ReceiveAck(TCPHeader *tcpHdr, PacketHeader *pktHdr, MailHeader *mailHdr, char *data) {
    char * buffer = new char[MaxPacketSize];
    postOffice->Receive(0, pktHdr, mailHdr, buffer);

    bcopy(buffer, tcpHdr, sizeof(TCPHeader));
    bcopy( buffer + sizeof(TCPHeader), data, tcpHdr->length);
    // copy the message data into
    // the caller's buffer

    delete [] buffer;
}
