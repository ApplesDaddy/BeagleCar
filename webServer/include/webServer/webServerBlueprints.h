#ifndef _WEB_SERVER_BLUEPRINTS_H_
#define _WEB_SERVER_BLUEPRINTS_H_
#include "crow_all.h"
// #include "udp_constants.h"

#define CONTROLLER_IP "192.168.7.2"
#define WEBSERVER_UDP_PORT "12346"
#define FUSER_CMD "fuser"
#define CLOSE_PORT_CMD "-k " WEBSERVER_UDP_PORT "/udp" 

void add_routes(crow::SimpleApp& app);
void send_video_websocket_sample(crow::websocket::connection& conn);
void send_video_websocket(crow::websocket::connection& conn, int pipe_fd);
void signal_handler(int i);

#endif