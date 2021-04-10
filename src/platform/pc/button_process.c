#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 
#include <button_interface.h>
#include <sys/stat.h> 
#include <sys/types.h>

static bool Init(void *object);
static bool Read(void *object);
static const char * myfifo = "/tmp/multicast_fifo";

static int fd;

int main(int argc, char *argv[])
{    

    Button_Interface button_interface = 
    {
        .Init = Init,
        .Read = Read
    };

    Button_Data button = 
    {
        .object = NULL,
        .interface = &button_interface
    };

    UDP_Sender sender = 
    {
        .hostname = "232.1.1.1",
        .port  = "1234"
    };

    Button_Run(&sender, &button);
        
    return 0;
}

static bool Init(void *object)
{    
    (void)object;
    remove(myfifo);
    int ret = mkfifo(myfifo, 0666);
    return (ret == -1 ? false : true);
}

static bool Read(void *object)
{
    (void)object;
    int state;
    char buffer[2];

    fd = open(myfifo,O_RDONLY);
    read(fd, buffer, 2);	
    state = atoi(buffer);
    return state ? true : false;
}