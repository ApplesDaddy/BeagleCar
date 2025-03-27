#include "receiver.h"
#include "sender.h"
#include "hal/joystick.h"
#include "hal/rotary_encoder.h"
#include "hal/gpio.h"
#include "util/common_funcs.h"

#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>


static bool is_sender = false;
static bool is_terminal_sender = false;


static inline void handle_cmd_args(int argc, char* argv[])
{
    if(strcmp(argv[1], "--sender") == 0)
    { is_sender = true; }
    if(strcmp(argv[1], "--terminal") == 0)
    { is_terminal_sender = true; }

    if(argc >= 3 && strcmp(argv[2], "--sender") == 0)
    { is_sender = true; }
    if(argc >= 3 && strcmp(argv[2], "--terminal") == 0)
    { is_terminal_sender = true; }

    if(strcmp(argv[1], "--help") == 0)
    {
        printf("Options:\n");
        printf("\t--sender\t");
        printf("configure program to send joystick/encoder values to receiver\n");
        printf("\t--terminal\t");
        printf("send terminal input instead of hardware. (press one key at a time)\n");
        printf("wasd for joystick, j/l for encoder, e for encoder push, space for joystick push\n");
        printf("enter q to stop\n");
        exit(EXIT_SUCCESS);
    }
}


int main(int argc, char* argv[])
{
    if(argc > 1)
    { handle_cmd_args(argc, argv); }

    // initialize hardware only if there's hardware
    if(is_sender)
    {
        if(!is_terminal_sender)
        {
            gpio_init();
            joystick_init();
            rot_encoder_init();
        }

        send_udp_init(is_terminal_sender);
    }
    else
    { recv_udp_init(); }


    // listen for keypresses if sender, else sleep
    initscr();
    timeout(1000);
    int curr_encoder = 0;
    while(is_sender || recv_is_active())
    {
        if(is_sender)
        {
            // char input = getchar(); // alternative to ncurses (comment out ncurses part in CMakeLists)
            char input = getch();
            if(input == 'q') // quit
            { break; }

            if(is_terminal_sender)
            { send_terminal_input(input, &curr_encoder); }
        }
        else
        { sleep_ms(500); }
    }
    endwin();


    // cleanup hardware only if there's hardware
    if(is_sender)
    {
        send_udp_cleanup();

        if(!is_terminal_sender)
        {
            rot_encoder_cleanup();
            joystick_cleanup();
            gpio_cleanup();
        }
    }
    else
    { recv_udp_cleanup(); }

    return 0;
}