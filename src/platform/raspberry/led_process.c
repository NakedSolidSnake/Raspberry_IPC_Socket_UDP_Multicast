#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <led.h>
#include <led_interface.h>

#define BUFFER_SIZE 1024

void on_receive_message(const char *buffer, size_t buffer_size, void *data);

bool Init(void *object);
bool Set(void *object, uint8_t state);

int main(int argc, char *argv[])
{   
    char server_buffer[BUFFER_SIZE];

    LED_t led =
    {
        .gpio.pin = 0,
        .gpio.eMode = eModeOutput
    };

    LED_Interface led_interface = 
    {
        .Init = Init,
        .Set = Set
    };

    UDP_Receiver receiver = 
    {
        .buffer = server_buffer,
        .buffer_size = BUFFER_SIZE,
        .port = 1234,
        .on_receive_message = on_receive_message,
        .multicast_group = "232.1.1.1"
    };

    LED_Data data = 
    {
        .object = &led,
        .interface = &led_interface
    };

    LED_Run(&receiver, &data);
    
    return 0;
}

bool Init(void *object)
{
    LED_t *led = (LED_t *)object; 
    return LED_init(led) == EXIT_SUCCESS ? true : false;   
}

bool Set(void *object, uint8_t state)
{
    LED_t *led = (LED_t *)object;
    return LED_set(led, (eState_t)state) == EXIT_SUCCESS ? true : false;
}

void on_receive_message(const char *buffer, size_t buffer_size, void *data)
{
    LED_Data *led = (LED_Data *)data;

    if(strncmp("LED ON", buffer, strlen("LED ON")) == 0)
        led->interface->Set(led->object, 1);
    else if(strncmp("LED OFF", buffer, strlen("LED OFF")) == 0)
        led->interface->Set(led->object, 0);
}