#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <button_interface.h>

#define _1ms    1000

typedef enum 
{
    LED_ON, 
    LED_OFF
} LED_Commands;

const char *led_commands[] = 
{
    "LED ON",
    "LED OFF"
};

bool Button_Run(UDP_Sender *sender, Button_Data *button)
{
    int state = 0;

    if(button->interface->Init(button->object) == false)
        return false;

    if(UDP_Multicast_Sender_Init(sender) == false)
        return false;

    while (true)
    {
        while (1)
        {
            if (!button->interface->Read(button->object))
            {
                usleep(_1ms * 100);
                state ^= 0x01;
                break;
            }
            else
            {
                usleep(_1ms);
            }
        }

        UDP_Multicast_Sender_Send(sender, led_commands[state], strlen(led_commands[state]));
    }

    return false;
}
