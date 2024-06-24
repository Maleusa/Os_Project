#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "postTCP.h"
#include "interrupt.h"
#include "transmission.h"

#define NbMessages 6

// This test aims to test IP by sending diverses messages in a row.
// The machine 1 just stay in reception state while machine 0 send
// messages.

void
transmissionSimpleTest(int farAddr)
{
    printf(">>>> Test 1 de IP\n");
    /// Contact
    Contact to, from;
    to.netAddr=farAddr;
    to.boxAddr=1;
    from.boxAddr=1;

    /// Messages construction
    char const *msg1 = "Hello there! How are you doing ? Good I hope so!";
    char const *msg2 = "A beautiful message for you my love...";
    char const *msg3 = "Attention passengers ! The plane is burning.";
    char const *msg4 = "Have a great day buddy !";
    char const *msg5 = "I am a very beautiful potato that everybody loves since I am the kindest vegetable."
                       " PLEASE ADOPT ME I AM SO ALONE RIGHT NOW. HEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEELP !!!";
    char const *msg6 = "This is a really long message, so long that no one will want to read it but just"
                       " look if it makes any sense because I wrote something logical but unbearably long"
                       " and boring to read to check that IP can transfer messages of any size and finally"
                       " finish this part of the network and move on to FTP which also requires more and"
                       " more testing.";

    char const *messages[6] = { msg1, msg2, msg3, msg4, msg5, msg6};
    Message msg;
    msg.destination=to;
    msg.sender=from;
    /// End of Messages construction
    
   if (farAddr == 1)  // this is currently machine 0
    {
        // Number of messages sent
        int sent = 0;

        /// Sending of Message NbMessage times
        for (int i = 0; i < NbMessages ; i++)
        {
            printf(">>Sending message...\n");
            fflush(stdout);
            msg.data = messages[i];
            msg.id=i;
            msg.Print();
            printf("\n");
            fflush(stdout);
            int res = transmissionPost->Send(msg);
            if (res == 0) {
                printf(">>Message sent\n");
                sent++;
            } else {
                printf(">>Message not sent\n");
            }
            Delay(1);
        }
        printf(">>> %d/%d messages sent\n", sent, NbMessages);
        /// End of sending
    }
    else            // this is currently machine 1
    {
        // Number of messages received
        int received = 0;
       

        /// Endless loop of reception
        while (1) {
            transmissionPost->Receive(1, &msg);
//            msg.Print();
            printf("\nGot \"%s\" from %d\n", msg.data, msg.sender.netAddr);
            fflush(stdout);
            received++;
            printf(">>> %d/%d messages received\n", received, NbMessages);
            if(msg.id==5)break;
        }
        /// End of loop
    }

    interrupt->Halt();
}
