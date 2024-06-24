// postTCP.h 
//	Data structures for providing the abstraction of reliable,
//	ordered, fixed-size message delivery to mailboxes on other 
//	(directly connected) machines.  Messages can be dropped by
//	the network, but they are never corrupted.
//  
//  For each message send we wait for a respond, there is three
//  differents messages we can receive : ACK, MESSAGE, TIMERSIGNAL
//  If we receive a ACK, the next instructions are made, if we receive
//  a MESSAGE type we ACK then wait again for our ACK.
//
#include "copyright.h"

#ifndef POSTTCP_H
#define POSTTCP_H

#include "network.h"
#include "synchlist.h"
#include "post.h"

#define TEMPO 5000
#define MAXEMISSION 5


#define MaxTCPSize 	(MaxMailSize - sizeof(TCPHeader))


typedef int AlternedBit;
enum class TYPE {
  MESSAGE,
  ACK,
  TIMERSIGNAL
};

class TCPHeader {
  public:
    TYPE type;
    unsigned length;		// Bytes of message data (excluding the
    // tcp header)
    int id = -1; //For the timer
};

// The following class defines a "TCP Post Office", or a collection of 
// TCPmailboxes.  The TCP Post Office is a synchronization object that provides
// two main operations: Send -- send a message to a TCPmailbox on a remote 
// machine, and Receive -- wait until a message is in the mailbox, 
// then remove and return it.
//
// Incoming messages are put by the TCPPostOffice into the 
// appropriate TCPmailbox, waking up any threads waiting on Receive.

class TCPPostOffice {
  public:
    TCPPostOffice(PostOffice *postOffice);
    ~TCPPostOffice();
    
    int Send(TCPHeader tcpHdr, PacketHeader pktHdr, MailHeader mailHdr, const char *data);
    				// Send a message to a mailbox on a remote 
				// machine.  The fromBox in the MailHeader is 
				// the return box for ack's.
        // Wait for the Ack or resend if the timer bips
    
    void Receive(int box, TCPHeader *tcpHdr, PacketHeader *pktHdr, MailHeader *mailHdr, char *data);
    				// Retrieve a message from "box".  Wait if
				// there is no message in the box.

    void PostalDelivery();	// Wait for incoming messages,
				// and then put them in the correct mailbox

    void SendAck(Mail *mail);

  private:
    // Have a PostOffice wich take car of the send part
    PostOffice *postOffice;

    void ReceiveAck(TCPHeader *tcpH, PacketHeader *pktHdr, MailHeader *mailHdr, char *data);
    void ScheduleReSend(int id);
};

#endif
