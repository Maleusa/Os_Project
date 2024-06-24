#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "postTCP.h"
#include "interrupt.h"
#include "transmission.h"

#define NbMessages 6
#define Machine0 0
#define Machine1 1
#define Machine2 2


// This test only run if we use machine 0 to 1 and 1 to 0,
// and machine 2 that send messages to machine 1.
// The goal is to ensure that the machine 2's message are ignored.

// Note that if used with -l, there is a chance that a packet is definitively
// lost, that implies that the machine 1 (receiver) will be stuck in
// loop reception. This tends to be solved with FTP in the future.

void
IP3MachinesTest(int dest)
{
    printf(">>>> Test 2 de IP\n");
    printf(">>>> Not implemented yet\n");

    Contact to, from;
    int received, sent, res;

    /// Messages construction
    char const *intruder = "Devil Message";
    char const *trepasser = "Hard Message";

    char const *msg1 = "Love message";
    char const *msg2 = "Friendship message";
    char const *msg3 = "Humor message";
    char const *msg4 = "Food massage";
    char const *msg5 = "Hate message";
    char const *msg6 = "Hungry message";

    char const *messages[6] = { msg1, msg2, msg3, msg4, msg5, msg6};
    Message msg;
    msg.destination=to;
    msg.sender=from;
    /// End of Messages construction

    /// Test Protocol
    switch (dest){ // Current thread is :
        case Machine0: // Receiver Machine
            /// Loop of reception
            received = 0;
            while (1) {
                transmissionPost->Receive(1, &msg); // Loop Reception on current message
                printf("\nGot \"%s\" from %d\n", msg.data, msg.sender.netAddr);
                fflush(stdout);
                received++;
                printf(">>> %d messages completely received\n", received);
                if(msg.id==NbMessages)break;
            }
            /// End of loop
            printf("All %d messages completely received", received);
            break;
        case Machine1: // Sender Machine
            /// Contacts
            to.netAddr=dest;
            to.boxAddr=1;
            from.boxAddr=1;

            /// Message
            msg.destination=to;
            msg.sender=from;

            /// Sending of NbMessages
            sent = 0;
            for (int i = 0; i < NbMessages ; i++)
            {
                printf(">>Sending message...\n");
                fflush(stdout);
                msg.data = messages[i];
                msg.id=i;
                msg.Print();
                printf("\n");
                fflush(stdout);
                res = transmissionPost->Send(msg);
                if (res == 0) {
                    printf(">>Message %d sent\n", msg.id);
                    sent++;
                } else {
                    printf(">>Message %d not sent\n", msg.id);
                }
                Delay(1);
            }
            printf(">>> %d/%d messages sent\n", sent, NbMessages);
            /// End of sending

            break;
        case Machine2: // Other Sender Machine
            /// Contacts
            to.netAddr=dest;
            to.boxAddr=1;
            from.boxAddr=1;

            /// Message
            msg.destination=to;
            msg.sender=from;

            /// First Message
            msg.data = intruder;
            msg.id = 1;
            res = transmissionPost->Send(msg);
            if (res == 0) {
                printf(">>Message %d inlaid\n", msg.id);
            } else {
                printf(">>Message %d not inlaid\n", msg.id);
            }

            /// Second Message
            msg.data = trepasser;
            msg.id = 2;
            res = transmissionPost->Send(msg);
            if (res == 0) {
                printf(">>Message %d inlaid\n", msg.id);
            } else {
                printf(">>Message %d not inlaid\n", msg.id);
            }
            break;
        default:
            printf("Destination machine must be <0,1,2>");
            break;
    }
    /// End of Test Protocol

    interrupt->Halt();
}
