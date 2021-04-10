#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{

    int pid_button, pid_led;
    int button_status, led_status;

    pid_button = fork();

    if(pid_button == 0)
    {
        //start button process
        char *args[] = {"./button_process", NULL};
        button_status = execvp(args[0], args);
        printf("Error to start button process, status = %d\n", button_status);
        abort();
    }   

    pid_led = fork();

    if(pid_led == 0)
    {
        //Start led process
        char *args[] = {"./led_process", NULL};
        led_status = execvp(args[0], args);
        printf("Error to start led process, status = %d\n", led_status);
        abort();
    }

    return EXIT_SUCCESS;
}