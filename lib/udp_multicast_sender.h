#ifndef UDP_MULTICAST_SENDER_H_
#define UDP_MULTICAST_SENDER_H_

#include <stdbool.h>
#include <stdlib.h>

typedef struct 
{
    int socket;
    const char *hostname;
    const char *port;
} UDP_Sender;

bool UDP_Multicast_Sender_Init(UDP_Sender *sender);

bool UDP_Multicast_Sender_Send(UDP_Sender *sender, const char *message, size_t message_size);




#endif /* UDP_MULTICAST_SENDER_H_ */
