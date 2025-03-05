#include "crow_all.h"
#include "webServer/webServerBlueprints.h"

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
    CROW_WEBSOCKET_ROUTE(app, "/ws")
    .onopen([&](crow::websocket::connection& conn){
            CROW_LOG_INFO << "new websocket connection";
            CROW_LOG_INFO << "ip address of new remote connection: " <<  conn.get_remote_ip();
            })
    .onclose([&](crow::websocket::connection& conn, const std::string& reason, uint16_t){
            CROW_LOG_INFO << "websocket connection closed with the following reason: " << reason;
            CROW_LOG_INFO << "ip address of closinng remote connection: " <<  conn.get_remote_ip();
            })
    .onmessage([&](crow::websocket::connection& conn, const std::string& data, bool is_binary)){
                if (is_binary){
                    CROW_LOG_INFO << "received binary message: " << data;
                    conn.send_binary(data);
                } else {
                    CROW_LOG_INFO << "received message: " << data;
                    conn.send_text(data);
                }
            };


}
