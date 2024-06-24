// ftp.h 
// Handle everythig about the connexion stuff
//
// For now a Client can only send messages (connect
// and disconnect too)
// A Server can only receive messages
//
#include "copyright.h"

#ifndef FTP_H
#define FTP_H

#include "network.h"
#include "synchlist.h"
#include "transmission.h"
#include "synch.h"

enum class FTPMODE{
    SERVER,
    CLIENT,
    DOWN
};

typedef struct BoxState_t{
    bool free;
    Contact connected;
}BoxState;

typedef int ConnectionValue;

class ConnectionHeader{
    public:
        ConnectionValue connectionValue;
};


class File{
    public:
        //MetaData :
        char * name;
        int length;
        char * data;
};

class FTP{
    public:
        FTP(TransmissionPost *transmissionPost, int nBoxes);
        //The number of available boxes is nBoxes less 2 in reality :
        //0 is for error and 1 is for connection
        //At first the FTP is neither in mode SERVER nor in mode CLIENT
        ~FTP();
        //We assume it is called when every connection are stopped

        int ToClient();
            //Check that no one are connected before change
            //May creat EdgeCase.
        int ToServer();
            //Disconnect the possible connexion before switching
            //Maximum one

        int Connect(NetworkAddress farAddr);
            //Ask for a connection to the Machine with farAddr ID
            //Can be connected to only one server at a time
            //Return 0 if successfully connected
            //Only in CLIENT mode

            //This method is only available when we are
            //in CLIENT mode
        int Send(Message message);

        void Disconnect();
            //Need to be connected and being in CLIENT mode

        
        void Receive(int box, Message * message);
            //Only on SERVER mode
    private:
    
        void ActivateListening();
        void InitializeBoxes();

        TransmissionPost *transmissionPost;
        Lock * modeLock;
        FTPMODE mode;

        //Only useable in SERVER mode
        BoxState * boxes;
        Lock * boxesLock;
        int nBoxes;
        
        //Only useable when connected to the machine 'to'
        Lock * connectionLock;
        bool connected;
        Contact to; 
};

#endif