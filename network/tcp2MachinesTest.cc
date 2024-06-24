#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "postTCP.h"
#include "interrupt.h"

#define NbMessages 10

// This test only run if we use machine 0 to 1 and 1 to 0.

void
TCP2MachinesTest(int dest)
{
    printf(">>>> Test 2 de TCP\n");
    if (dest == 1)  // this is currently machine 0
    {
        // Number of messages sent
        int sent = 0;
        /// Message information's
        // Message
        const char *message = "Hello World";
        // Packet
        PacketHeader outPktHdr;
        outPktHdr.to = dest;
        // Mail
        MailHeader outMailHdr;
        outMailHdr.to = 1;      // Not 0
        outMailHdr.from = 1;    // since it is reserved
        outMailHdr.length = strlen(message) + 1 + sizeof(TCPHeader);   // Since length represent data + TCPHeader concatenated
        // TCP
        TCPHeader outTCPHeader;
        outTCPHeader.type = TYPE::MESSAGE;
        outTCPHeader.length = strlen(message) + 1;
        /// End of Message information's

        /// Sending of Message NbMessage times
        for (int i = 0; i < NbMessages ; i++)
        {
            printf(">>Sending message...\n");
            fflush(stdout);
            int res = tcpPostOffice->Send(outTCPHeader, outPktHdr, outMailHdr, message);
            if (res == 0) {
                printf(">>Message sent\n");
                sent++;
            } else {
                printf(">>Message not sent\n");
            }
            Delay(0.5);
        }
        printf(">>> %d/%d messages sent\n", sent, NbMessages);
        /// End of sending
    }
    else            // this is currently machine 1
    {
        // Number of messages received
        int received = 0;
        /// Message to be received
        PacketHeader inPktHdr;
        TCPHeader inTCPHeader;
        MailHeader inMailHdr;
        char buffer[MaxMailSize];
        /// End of Message

        /// Endless loop of reception
        printf(">> Waiting for messages...\n");
        while (1) {
            tcpPostOffice->Receive(1, &inTCPHeader, &inPktHdr, &inMailHdr, buffer);
            printf("Got \"%s\" from %d\n", buffer, inPktHdr.from);
            fflush(stdout);
            received++;
            printf(">>> %d/%d messages received\n", received, NbMessages);
        }
        /// End of loop

    }
    // Then we're done!
    interrupt->Halt();
}
