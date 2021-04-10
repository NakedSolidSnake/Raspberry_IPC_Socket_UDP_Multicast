#ifndef LED_INTERFACE_H_
#define LED_INTERFACE_H_

#include <stdbool.h>
#include <stdint.h>
#include <udp_multicast_receiver.h>

/**
 * @brief 
 * 
 */
typedef struct 
{
    bool (*Init)(void *object);
    bool (*Set)(void *object, uint8_t state);
} LED_Interface;

typedef struct 
{
    void *object;
    LED_Interface *interface;
} LED_Data;

/**
 * @brief 
 * 
 * @param object 
 * @param argv 
 * @param led 
 * @return true 
 * @return false 
 */
bool LED_Run(UDP_Receiver *receiver, LED_Data *led);

#endif /* LED_INTERFACE_H_ */
