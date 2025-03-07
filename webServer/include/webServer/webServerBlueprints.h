#ifndef _WEB_SERVER_BLUEPRINTS_H_
#define _WEB_SERVER_BLUEPRINTS_H_
#include "crow_all.h"

void add_routes(crow::SimpleApp& app);
void send_video_websocket_sample(crow::websocket::connection& conn);;

#endif