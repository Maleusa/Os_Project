// transmission.h 
//	Data structures for providing the abstraction of reliable,
//	ordered, dynamic-sized message delivery to (directly connected) 
//  other machines.  
//  
//  Message are composed of : Destination, Sender, name, data. 
//  Message can't be corrupted, only lost.
//
#include "copyright.h"

#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include "network.h"
#include "synchlist.h"
#include "postTCP.h"
class Contact{
    public:
    NetworkAddress netAddr;		// machine ID
    MailBoxAddress boxAddr;		// mail box
};

class Message{
    public:
    //Information about the destination remote machine : addr network, box, etc
    //In the receive case, the destination informations represents ourself
    Contact destination;

    //Information about the sender remote machine : addr network, box, etc
    //In the receive case, the sender informations represents the other machine
    Contact sender;

    //ID of the message (like object of a email)
    int id;

    //Data sended
    const char* data;

    //Print method
    void Print();
};

class IPHeader{
    public:
        int size_data; //Total size of the data
        unsigned int num_total_pkt; 
        unsigned int num_current_pkt;
        int id; //It's the same for each IP packet of a single message

};

// The following class defines a "transmission post". It is a synchronization 
// object that provides two main operations: Send -- send a
// message to a other (directly connected) machine 
// and Receive -- wait until a message is in one of our mailBox, 
// then remove and return it.
//

class TransmissionPost {
  public:
    TransmissionPost(TCPPostOffice *tcpPostOffice);
    ~TransmissionPost();
    
    int Send(Message message);
    				// Send a message to a TransmissionPost of a remote machine
                    // Return -1 if the transmission failed
                    // Need a ack after the recreation of the  !!!!!!

    void Receive(int box, Message* message);
    				// Retrieve a message from a "box".  Wait if
				// there is no message in the box.
  private:
    // Have a PostOffice wich take car of the send part
    TCPPostOffice *tcpPostOffice;
    NetworkAddress netAddr;

    //Some methods for deconstruction and reconstruction of messages
};
#endif