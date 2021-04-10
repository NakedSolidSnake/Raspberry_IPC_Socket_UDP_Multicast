#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <led_interface.h>


#define _1ms    1000

bool LED_Run(UDP_Receiver *receiver, LED_Data *led)
{

	if(led->interface->Init(led->object) == false)
		return false;

	if(UDP_Receiver_Init(receiver) == false) 
		return false;


	while(true)
	{
		UDP_Receiver_Run(receiver, led);
	}

	return false;	
}
