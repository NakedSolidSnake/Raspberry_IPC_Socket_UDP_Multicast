#include <udp_multicast_receiver.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

bool UDP_Multicast_Receiver_Init(UDP_Receiver *receiver)
{
    bool status = false;
    int yes = 1;
    struct sockaddr_in server_addr;
    struct ip_mreq multicast;


    do
    {
        if(!receiver || !receiver->buffer || !receiver->buffer_size)
            break;

        receiver->socket = socket(AF_INET, SOCK_DGRAM, 0);
        if(receiver->socket < 0)
            break;

        memset(&server_addr, 0, sizeof(server_addr));

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(receiver->port);        
        
        if (setsockopt(receiver->socket, SOL_SOCKET, SO_REUSEADDR, (void*)&yes, sizeof(yes)) < 0)
            break;

        if (bind(receiver->socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
            break;

        multicast.imr_multiaddr.s_addr = inet_addr(receiver->multicast_group);
        multicast.imr_interface.s_addr = htonl(INADDR_ANY);

        if(setsockopt(receiver->socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&multicast, sizeof(multicast)) < 0)
            break;

        status = true;
        
    } while(false);

    return status;
}

bool UDP_Multicast_Receiver_Run(UDP_Receiver *receiver, void *user_data)
{
    bool status = false;
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    size_t read_size;

    if(receiver->socket > 0)
    {
        read_size = recvfrom(receiver->socket, receiver->buffer, receiver->buffer_size, MSG_WAITALL,
                                    (struct sockaddr *)&client_addr, &len); 
        receiver->buffer[read_size] = 0;
        receiver->on_receive_message(receiver->buffer, read_size, user_data);
        memset(receiver->buffer, 0, receiver->buffer_size);
        status = true;
    }

    return status;
}