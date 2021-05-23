#ifndef UDP_MULTICAST_RECEIVER_H_
#define UDP_MULTICAST_RECEIVER_H_

#include <stdbool.h>
#include <stdlib.h>

/**
 * @brief 
 * 
 */
typedef void (*Event)(const char *buffer, size_t buffer_size, void *data);

/**
 * @brief 
 * 
 */
typedef struct 
{
    int socket;
    int port;
    char *buffer;
    size_t buffer_size;
    Event on_receive_message;
    const char *multicast_group;
} UDP_Receiver;



bool UDP_Multicast_Receiver_Init(UDP_Receiver *receiver);

bool UDP_Multicast_Receiver_Run(UDP_Receiver *receiver, void *user_data);


#endif /* UDP_MULTICAST_RECEIVER_H_ */
