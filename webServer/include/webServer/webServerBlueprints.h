#ifndef _WEB_SERVER_BLUEPRINTS_H_
#define _WEB_SERVER_BLUEPRINTS_H_
#include "crow_all.h"

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    // #include <libavutil/imgutils.h>
    // #include <libavutil/avutil.h>
    // #include <libswscale/swscale.h>
}

void add_routes(crow::SimpleApp& app);
void send_video_websocket_sample(crow::websocket::connection& conn);
void send_video_websocket(crow::websocket::connection& conn, int pipe_fd);

#endif