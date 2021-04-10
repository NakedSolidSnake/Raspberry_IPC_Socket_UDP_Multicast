#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h>
#include <syslog.h>
#include <led_interface.h>

#define BUFFER_SIZE 1024

void on_receive_message(const char *buffer, size_t buffer_size, void *data);

bool Init(void *object);
bool Set(void *object, uint8_t state);

int main(int argc, char *argv[])
{   
    char server_buffer[BUFFER_SIZE];

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

    LED_Data led = 
    {
        .object = NULL,
        .interface = &led_interface
    };

    LED_Run(&receiver, &led);
    
    return 0;
}

bool Init(void *object)
{
    (void)object; 
    return true;
}

bool Set(void *object, uint8_t state)
{
    (void)object;    
    openlog("LED UDP", LOG_PID | LOG_CONS , LOG_USER);
    syslog(LOG_INFO, "LED Status: %s", state ? "On": "Off");
    closelog(); 
    return true;
}

void on_receive_message(const char *buffer, size_t buffer_size, void *data)
{
    LED_Data *led = (LED_Data *)data;

    if(strncmp("LED ON", buffer, strlen("LED ON")) == 0)
        led->interface->Set(led->object, 1);
    else if(strncmp("LED OFF", buffer, strlen("LED OFF")) == 0)
        led->interface->Set(led->object, 0);
}