#ifndef _SEND_UDP_H_
#define _SEND_UDP_H_

#include <stdbool.h>


void send_udp_init(bool use_terminal);
void send_udp_cleanup();
void send_terminal_input(char input, int* curr_encoder);

#endif