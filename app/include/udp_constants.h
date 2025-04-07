#ifndef _UDP_CONSTANTS_H_
#define _UDP_CONSTANTS_H_


#define UDP_PORT 12345
#define WEBSERVER_UDP_PORT "12346"
#define LCD_UDP_PORT "12347"

#define MAX_UDP_LEN 1500

#define CONTROLLER_IP "192.168.7.2"
#define RECV_IP "192.168.7.1"

#define JOYSTICK_ANGLE  // undef to send raw x, y values ([-1.0, 1.0]) instead of angle + radius

#define SEND_INTERVAL_MS 100    // send hardware (joystick) inputs every n ms
#define LISTEN_TIMEOUT_MS 100   // used to prevent recv_from() from blocking shutdown


enum UDP_MSG_CODES
{
    CODE_STOP = -1,

    CODE_JOYSTICK_VAL = 0,
    CODE_ENCODER_VAL,

    CODE_ENCODER_PUSH,
    CODE_JOYSTICK_PUSH,

    UDP_MSG_CODES_COUNT
};

#endif