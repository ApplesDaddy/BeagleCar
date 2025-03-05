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

    //TODO: Websocket example


}
