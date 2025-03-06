#include "crow_all.h"
#include "webServer/webServerBlueprints.h"
#include <stdlib.h>

// Example from: https://crowcpp.org/master/getting_started/your_first_application/
void add_routes(crow::SimpleApp& app){
    
    //define your endpoint at the root directory
    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });


    //Return a template
    CROW_ROUTE(app, "/template/<int>")([](int count){
        auto page = crow::mustache::compile("The value is {{value}}");
        crow::mustache::context ctx;
        ctx["value"] = count;

        return page.render(ctx);
    });


    //Return a template from a file with values
    CROW_ROUTE(app, "/template_file/<int>")([](int count){
        auto page = crow::mustache::load("template_page.html");
        crow::mustache::context ctx;
        ctx["value"] = count;

        return page.render(ctx);
    });

    //Return a template from a file
    CROW_ROUTE(app, "/load_file")([](){
        auto page = crow::mustache::load_text("template_page.html");
        
        return page;
    });

    // From: https://crowcpp.org/master/guides/websockets/
    // Only prints out to the console for now. To get a 
    // connection go to /static/websocket_page.html
    CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([&](crow::websocket::connection& conn){
            CROW_LOG_INFO << "new websocket connection";
            CROW_LOG_INFO << "ip address of new remote connection: " <<  conn.get_remote_ip();
            })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t){
            CROW_LOG_INFO << "websocket connection closed with the following reason: " << reason;
            CROW_LOG_INFO << "ip address of closinng remote connection: " <<  conn.get_remote_ip();
            })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary){
                if (is_binary){
                    CROW_LOG_INFO << "received binary message: " << data;
                    conn.send_binary(data);
                } else {
                    CROW_LOG_INFO << "received message: " << data;
                    conn.send_text(data);
                }
            });

    CROW_WEBSOCKET_ROUTE(app, "/ws_video")
    .onopen([&](crow::websocket::connection& conn){
            CROW_LOG_INFO << "new websocket connection";
            CROW_LOG_INFO << "ip address of new remote connection: " <<  conn.get_remote_ip();


            // Start a thread to send the video data 
            std::thread t(send_video_websocket_sample, std::ref(conn));
            t.detach();

            })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t){
            CROW_LOG_INFO << "websocket connection closed with the following reason: " << reason;
            CROW_LOG_INFO << "ip address of closinng remote connection: " <<  conn.get_remote_ip();
            })

}

void send_video_websocket_sample(crow::websocket::connection& conn){
    for (int i = 0; i < 16; i++){
        char file_name[100];
        sprintf(file_name, "video/output%03d.mp4", i);

        FILE* file = fopen(file_name, "rb");
        if (file == NULL){
            CROW_LOG_INFO << "Could not open file: " << file_name;
            return;
        }

        char data[400 * 1024]; // 400 KB - since the largest 2 seocond file seems to be about 300 KB
        size_t read_size = fread(data, 1, sizeof(data), file);
        if (read_size == 0){
            CROW_LOG_INFO << "Could not read file: " << file_name;
            return;
        }

        conn.send_binary(std::string(data, read_size));
        fclose(file);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}
