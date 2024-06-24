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
IPCrashTest(int dest)
{
    printf(">>>> Test intercaler de IP\n");

    Contact to, from;
    int received, res;

    /// Messages construction
    char const *intruder = "I'm gonna try to break down that IP post, in fact, "
                            "maybe if may message is long, long, I mean very long,"
                            " maybe there will be time where our two ID crash together."
                            " Let's go Rick. Because I was meaning a long message I just"
                            " continue writing stuff, hoping this shit gonna crash. If it"
                            " does'nt crash it means that we need to fine where did it "
                            "became sequential. AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                            "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA!";

    char const *msg1 = "I just wanna tell you how I'm feeling, Gotta make you"
                        "understand, Never gonna give you up, Never gonna let "
                        "you down, Never gonna run around and desert you, Never"
                        " gonna make you cry. BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
                        "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB";

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
            to.netAddr=1;
            to.boxAddr=1;
            from.boxAddr=1;

            /// Message
            msg.destination=to;
            msg.sender=from;

            /// Sending of NbMessages
            printf(">>Sending message...\n");
            fflush(stdout);
            msg.data = msg1;
            msg.id=1;
            printf("\n");
            fflush(stdout);
            res = transmissionPost->Send(msg);
            if (res == 0) {
                printf(">>Message sent\n");
            } else {
                printf(">>Message not sent\n");
            }
            Delay(1);
            /// End of sending

            break;
        case Machine2: // Other Sender Machine
            /// Contacts
            to.netAddr=1;
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
                printf(">>Message inlaid\n");
            } else {
                printf(">>Message not inlaid\n");
            }
            break;
        default:
            printf("Destination machine must be <0,1,2>");
            break;
    }
    /// End of Test Protocol

    interrupt->Halt();
}
