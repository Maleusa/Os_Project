//ftp.cc
#include "network.h"
#include "synchlist.h"
#include "transmission.h"
#include "synch.h"
#include "ftp.h"

FTP::FTP(TransmissionPost *transmissionPost_, int nBoxes_){
    transmissionPost=transmissionPost_;
    modeLock = new Lock("FTP_MODE");
    mode = FTPMODE::DOWN;

    //Server part
    boxes = new BoxState[nBoxes];
    boxesLock = new Lock("FTP_BOXES");
    nBoxes = nBoxes_;

    //Client part
    connectionLock = new Lock("FTP_CONNECTION");
    connected=false;
    //to is initialize with Connect()

    InitializeBoxes();
    ActivateListening();
}

FTP::~FTP(){
    delete boxes;
    delete boxesLock;
    delete modeLock;
    delete connectionLock;
}

void FTP::InitializeBoxes(){
    boxesLock->Acquire();
    for(int i=0; i<nBoxes; i++){
        boxes[i].free=true;
    }
    boxesLock->Release();
}

void FTP::ActivateListening(){
    //Postal Delivery Like gonna be some clean Shit

    //TODO

}

int FTP::ToServer(){
    modeLock->Acquire();
    if(mode == FTPMODE::SERVER){
        //we already are in SERVER mode
        modeLock->Release();
        return 0;
    }
    else if(mode == FTPMODE::DOWN){
        //For now I see nothing to do 
        mode = FTPMODE::SERVER;
        modeLock->Release();
        return 0;
    }
    else if(mode == FTPMODE::CLIENT){
        //We need our possible connection first
        connectionLock->Acquire();
        if(connected) Disconnect();
        ASSERT(!connected);
        connectionLock->Release();
        mode = FTPMODE::SERVER;
        modeLock->Release();
        return 0;
    }
    //Should never be here
    modeLock->Release();
    return -1;
}

int FTP::ToClient(){
    modeLock->Acquire();
    if(mode == FTPMODE::CLIENT){
        //we already are in CLIENT mode
        modeLock->Release();
        return 0;
    }
    else if(mode == FTPMODE::DOWN){
        //For now I see nothing to do 
        mode = FTPMODE::SERVER;
        modeLock->Release();
        return 0;
    }
    else if(mode == FTPMODE::SERVER){
        //We need to cut every connection first
        bool isAble = true;
        boxesLock->Acquire();
        for(int i=0; i<nBoxes; i++){
            if(!boxes[i].free) isAble = false;
        }
        boxesLock->Release();
        if(!isAble){
            modeLock->Release();
            return -1;
        }
        mode = FTPMODE::CLIENT;
        modeLock->Release();
        return 0;
    }
    //Should never be here
    return -1;
}

//Message message gonna be changed by File file
int FTP::Send(Message message){
    //Only possible in CLIENT mode
    modeLock->Acquire();
    if(mode!=FTPMODE::CLIENT){
        modeLock->Release();
        return -1;
    }
    modeLock->Release();

    //Only possible if we already are connected
    //(It make sure that to is define)
    connectionLock->Acquire();
    if(!connected){
        connectionLock->Release();
        return;
    }
    connectionLock->Release();

    //Stuff TODO to handle files
    message.destination=to;
    int rtVal = transmissionPost->Send(message);
    return rtVal;
}

//Message * message gonna be changed by (File * file, int box)
void FTP::Receive(int box, Message * message){
    //Only possible in SERVER mode
    modeLock->Acquire();
    if(mode!=FTPMODE::SERVER){
        modeLock->Release();
        return;
    }
    modeLock->Release();

    //Only possible if we already are connected
    //(It make sure that to is define)

    boxesLock->Acquire();
    if(boxes[box].free) {
        boxesLock->Release();
        return;
    }
    boxesLock->Release();
    
    //Stuff TODO to handle files
    transmissionPost->Receive(box, message);
}