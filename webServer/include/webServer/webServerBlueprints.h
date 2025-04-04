#ifndef _WEB_SERVER_BLUEPRINTS_H_
#define _WEB_SERVER_BLUEPRINTS_H_
#include "crow_all.h"

void add_routes(crow::SimpleApp& app);
void send_video_websocket_sample(crow::websocket::connection& conn);
void send_video_websocket(crow::websocket::connection& conn, int pipe_fd);

#endif