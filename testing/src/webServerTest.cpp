#include "webServer/webServerBlueprints.h"
#include "crow_all.h"


int main()
{
    crow::SimpleApp app; //define your crow application

    add_routes(app); //add the routes to the app

    //set the port, set the app to run on multiple threads, and run the app
    app.port(8080).multithreaded().run();
}