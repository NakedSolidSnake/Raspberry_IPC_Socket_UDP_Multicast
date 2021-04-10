#ifndef BUTTON_INTERFACE_H_
#define BUTTON_INTERFACE_H_

#include <stdbool.h>
#include <udp_multicast_sender.h>

/**
 * @brief 
 * 
 */
typedef struct 
{
    bool (*Init)(void *object);
    bool (*Read)(void *object);
    
} Button_Interface;

typedef struct 
{
    void *object;
    Button_Interface *interface;
} Button_Data;

/**
 * @brief 
 * 
 * @param object 
 * @param argv 
 * @param button 
 * @return true 
 * @return false 
 */
bool Button_Run(UDP_Sender *sender, Button_Data *button);

#endif /* BUTTON_INTERFACE_H_ */
