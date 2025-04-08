#include "receiver.h"
#include "udp_constants.h"

#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <hal/motor.h>


static pthread_t udp_thread;
static int sock;

static bool initialized = false;
static bool stop_thread = false;     // set true to stop

static int encoder_val = -1;
static int max_ack_timeout = 5;     // re-send ack on n-th duplicate message
static int curr_ack_timeout = 0;    // need n more duplicate messages before resend


// ================================= private function declarations =================================
static inline void process_cmd(char* cmd, struct sockaddr_in* remote);
static void* listen_udp(void* args);


// ========================================= public functions ======================================
void recv_udp_init()
{
    printf("RECEIVER INITIALIZED\n");
    initialized = true;
    pthread_create(&udp_thread, NULL, &listen_udp, NULL);
}
void recv_udp_cleanup()
{
    stop_thread = true;
    pthread_join(udp_thread, NULL);

    // tell sender to close
    struct sockaddr_in remote;
    char msg_rx[MAX_UDP_LEN];
    snprintf(msg_rx, MAX_UDP_LEN, "%d", CODE_STOP);
    sendto(sock, msg_rx, strlen(msg_rx), 0, (struct sockaddr*)&remote, sizeof(remote));

    // clean up
    close(sock);
    initialized = false;
}

bool recv_is_active()
{ return !stop_thread; }


// ======================================== private functions ======================================
// command format: <int>:<args>
// valid commands:  0 <x-val> <y-val>   or  0:<angle> <radius>      joystick input
//                  1 <encoder val>                                 encodeer input
//                  2                                               encoder push
//                  3                                               joystick push
//                  -1                                              stop
static inline void process_cmd(char* cmd, struct sockaddr_in* remote)
{
    if(!cmd || strlen(cmd) <= 0) { return; }

    // trim newline
    int new_line = strlen(cmd) - 1;
    if (cmd[new_line] == '\n')
    { cmd[new_line] = '\0'; }

    char* code = strtok(cmd, " ");
    if(!code)
    { printf("empty udp message\n"); return; }

    int int_code = atoi(code);
    switch(int_code)
    {
        case CODE_JOYSTICK_VAL:
        {
            char* param = strtok(NULL, " ");
            if(!param)
            { printf("Missing x-val\n"); return; }

            float x_val = atof(param);

            param = strtok(NULL, " ");
            if(!param)
            { printf("Missing y-val\n"); return; }

            float y_val = atof(param);

            // TODO: do something with this input
            printf("x: %f   y: %f\n", x_val, y_val);
            int speed = (x_val) * 100;
            if (speed > 100){
                speed = 99; 
            }
            if (speed < 0){
                speed =1;
            }
            printf("speed: %d\n", speed);
            motor_set_speed(speed, false);
        } break;
        case CODE_ENCODER_VAL:
        {
            char* param = strtok(NULL, " ");
            if(!param)
            { printf("Missing encoder val\n"); return; }

            int val = atoi(param);

            // TODO: do something with this input
            printf("encoder: %d\n", val);

            // ack
            if(val != encoder_val || curr_ack_timeout <= 0)
            {
                char msg_rx[MAX_UDP_LEN];
                snprintf(msg_rx, MAX_UDP_LEN, "%d", val);
                sendto(sock, msg_rx, strlen(msg_rx), 0, (struct sockaddr*)remote, sizeof(*remote));

                curr_ack_timeout = max_ack_timeout;
            }
            else { curr_ack_timeout--; }

        } break;
        case CODE_ENCODER_PUSH:
        {
            // TODO: add encoder push behavior if desired
            printf("encoder push\n");
        } break;
        case CODE_JOYSTICK_PUSH:
        {
            // TODO: add joystick push behavior if desired
            printf("joystick push\n");
        } break;
        case CODE_STOP:
        {
            stop_thread = true;
            printf("stopping\n");
        } break;
        default: // unknown command
        {
            printf("Unknown udp code\n");
        }
    }
}

static void* listen_udp(void* args)
{
    (void)args; // silence warning

    // set up
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(UDP_PORT);

    // bind
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    bind(sock, (struct sockaddr*)&sin, sizeof(sin));

    // set timeout so we dont block on shutdown
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = LISTEN_TIMEOUT_MS * 1000;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    { perror("Failed to set socket timeout"); }

    // listen
    struct sockaddr_in remote;
    unsigned int sin_len = sizeof(remote);
    do
    {
        char msg_rx[MAX_UDP_LEN];
        int bytes_rx = recvfrom(sock, msg_rx, MAX_UDP_LEN - 1, 0, (struct sockaddr*)&remote, &sin_len);
        if(bytes_rx <= 0 || bytes_rx >= MAX_UDP_LEN)
        {  continue; }

        // process input
        msg_rx[bytes_rx] = 0;
        printf("got msg: %s\n", msg_rx);
        process_cmd(msg_rx, &remote);
    }
    while(!stop_thread);

    return NULL;
}
