#include "sender.h"
#include "udp_constants.h"
#include "util/common_funcs.h"
#include "hal/joystick.h"
#include "hal/rotary_encoder.h"

#include <pthread.h>
#include <stdatomic.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


static pthread_t udp_save_thread;
static pthread_t udp_listen_thread;
static bool stop_thread = false;     // set true to stop listen/send threads

static int sock;
static struct sockaddr_in remote;
static atomic_int acked_encoder_val = 0;        // receiver currently has this value saved

static bool using_terminal = true;
static bool initialized = false;


// ================================= private function declarations =================================
static inline void setup_socket();
static inline void send_joy_push();
static inline void send_encoder_push();
static inline void send_joy_vals();
static inline void send_encoder_val();
void* send_udp(void* args);

static inline void process_cmd(char* cmd);
static void* listen_udp(void* args);

void send_terminal_input(char input, int* curr_encoder);


// ======================================== public functions ======================================
void send_udp_init(bool use_terminal)
{
    initialized = true;
    setup_socket();

    // enable sending data thru terminal instead of hardware
    if(!use_terminal)
    { using_terminal = false; pthread_create(&udp_save_thread, NULL, &send_udp, NULL); }

    pthread_create(&udp_listen_thread, NULL, &listen_udp, NULL);
}
void send_udp_cleanup()
{
    stop_thread = true;
    if(!using_terminal) { pthread_join(udp_save_thread, NULL); }
    pthread_join(udp_listen_thread, NULL);

    // tell receiver to close
    char msg_rx[MAX_UDP_LEN];
    snprintf(msg_rx, MAX_UDP_LEN, "%d", CODE_STOP);
    sendto(sock, msg_rx, strlen(msg_rx), 0, (struct sockaddr*)&remote, sizeof(remote));

    // clean up
    close(sock);

    initialized = false;
    using_terminal = true;
}


// ======================================== private functions ======================================
static inline void setup_socket()
{
    memset(&remote, 0, sizeof(remote));
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr(RECV_IP);
    remote.sin_port = htons(UDP_PORT);

    // bind
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    bind(sock, (struct sockaddr*)&remote, sizeof(remote));

    // set timeout so we dont block on shutdown
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = LISTEN_TIMEOUT_MS * 1000;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error");
    }
}

static inline void send_joy_push()
{
    char msg_rx[MAX_UDP_LEN];
    snprintf(msg_rx, MAX_UDP_LEN, "%d", CODE_JOYSTICK_PUSH);
    sendto(sock, msg_rx, strlen(msg_rx), 0, (struct sockaddr*)&remote, sizeof(remote));
}
static inline void send_encoder_push()
{
    char msg_rx[MAX_UDP_LEN];
    snprintf(msg_rx, MAX_UDP_LEN, "%d", CODE_ENCODER_PUSH);
    sendto(sock, msg_rx, strlen(msg_rx), 0, (struct sockaddr*)&remote, sizeof(remote));
}
static inline void send_joy_vals()
{
    char msg_rx[MAX_UDP_LEN];
    #ifdef JOYSTICK_ANGLE
        int angle = joystick_get_angle();
        float radius = joystick_get_radius();
        snprintf(msg_rx, MAX_UDP_LEN, "%d %d %f", CODE_JOYSTICK_VAL, angle, radius);
    #else
        float x = joystick_get_x_val();
        float y = joystick_get_y_val();
        snprintf(msg_rx, MAX_UDP_LEN, "%d %f %f", CODE_JOYSTICK_VAL, x, y);
    #endif
    sendto(sock, msg_rx, strlen(msg_rx), 0, (struct sockaddr*)&remote, sizeof(remote));
}
static inline void send_encoder_val()
{
    int val = rot_encoder_get_val();
    if(val == acked_encoder_val) // receiver already updated, so dont bother
    { return; }
    // printf("sending encoder %d\n", val);

    char msg_rx[MAX_UDP_LEN];
    snprintf(msg_rx, MAX_UDP_LEN, "%d %d", CODE_ENCODER_VAL, val);
    sendto(sock, msg_rx, strlen(msg_rx), 0, (struct sockaddr*)&remote, sizeof(remote));
}

void* send_udp(void* args)
{
    (void)args; // silence warning

    do
    {
        // send joystick vals
        if(joystick_is_pressed())
        { send_joy_push(); }
        send_joy_vals();

        // send encoder vals
        if(rot_encoder_is_pressed())
        { send_encoder_push(); }
        send_encoder_val();

        sleep_ms(SEND_INTERVAL_MS);
    }
    while(!stop_thread);

    return NULL;
}

static inline void process_cmd(char* cmd)
{
    if(!cmd || strlen(cmd) <= 0) { return; }

    // trim newline
    int new_line = strlen(cmd) - 1;
    if (cmd[new_line] == '\n')
    { cmd[new_line] = '\0'; }

    acked_encoder_val = atoi(cmd);

    // fine to check directly since encoder should never give -1
    if(acked_encoder_val == CODE_STOP)
    { stop_thread = true; }
}
static void* listen_udp(void* args)
{
    (void)args; // silence warning

    // listen
    struct sockaddr_in remote1;
    unsigned int sin_len = sizeof(remote1);
    do
    {
        char msg_rx[MAX_UDP_LEN];
        int bytes_rx = recvfrom(sock, msg_rx, MAX_UDP_LEN - 1, 0, (struct sockaddr*)&remote1, &sin_len);
        if(bytes_rx <= 0 || bytes_rx >= MAX_UDP_LEN)
        {  continue; }

        // process input (should only be encoder ack)
        msg_rx[bytes_rx] = 0;
        process_cmd(msg_rx);
    }
    while(!stop_thread);

    return NULL;
}

// for debugging/using without the beaglebone
void send_terminal_input(char input, int* curr_encoder)
{
    char msg_rx[MAX_UDP_LEN];
    switch(input)
    {
        // joystick
        case 'w':
        {
            #ifdef JOYSTICK_ANGLE
                snprintf(msg_rx, MAX_UDP_LEN, "%d %d %d", CODE_JOYSTICK_VAL, 0, 1);
            #else
                snprintf(msg_rx, MAX_UDP_LEN, "%d %f %f", CODE_JOYSTICK_VAL, 0, 1);
            #endif
        } break;
        case 's':
        {
            #ifdef JOYSTICK_ANGLE
                snprintf(msg_rx, MAX_UDP_LEN, "%d %d %d", CODE_JOYSTICK_VAL, 180, 1);
            #else
                snprintf(msg_rx, MAX_UDP_LEN, "%d %f %f", CODE_JOYSTICK_VAL, 0, -1);
            #endif
        } break;
        case 'd':
        {
            #ifdef JOYSTICK_ANGLE
                snprintf(msg_rx, MAX_UDP_LEN, "%d %d %d", CODE_JOYSTICK_VAL, 90, 1);
            #else
                snprintf(msg_rx, MAX_UDP_LEN, "%d %f %f", CODE_JOYSTICK_VAL, 1, 0);
            #endif
        } break;
        case 'a':
        {
            #ifdef JOYSTICK_ANGLE
                snprintf(msg_rx, MAX_UDP_LEN, "%d %d %d", CODE_JOYSTICK_VAL, 270, 1);
            #else
                snprintf(msg_rx, MAX_UDP_LEN, "%d %f %f", CODE_JOYSTICK_VAL, -1, 0);
            #endif
        } break;
        // encoder
        case 'l':
        {
            *curr_encoder += 1;
            snprintf(msg_rx, MAX_UDP_LEN, "%d %d", CODE_ENCODER_VAL, *curr_encoder);
        } break;
        case 'j':
        {
            *curr_encoder -= 1;
            *curr_encoder = *curr_encoder < 0? 0 : *curr_encoder;
            snprintf(msg_rx, MAX_UDP_LEN, "%d %d", CODE_ENCODER_VAL, *curr_encoder);
        } break;
        // push
        case ' ':
        {
            snprintf(msg_rx, MAX_UDP_LEN, "%d", CODE_JOYSTICK_PUSH);
        } break;
        case 'e':
        {
            snprintf(msg_rx, MAX_UDP_LEN, "%d", CODE_ENCODER_PUSH);
        } break;

        default:
        break;
    }
    sendto(sock, msg_rx, strlen(msg_rx), 0, (struct sockaddr*)&remote, sizeof(remote));
}
