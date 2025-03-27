#ifndef _RECV_UDP_H_
#define _RECV_UDP_H_

#include <stdbool.h>


void recv_udp_init();
void recv_udp_cleanup();
bool recv_is_active();


#endif