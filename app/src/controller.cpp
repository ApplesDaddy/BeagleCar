/*
TODO: In parallel,
    1. Start+configure webserver+lcd modules to receive video over udp
        a. Can listen on udp://@:port
            - port number must be unique
    2. Initialize joystick, rotary encoder, udp sender modules
        - Need receiver's IP address

    ?. Configure as AP?
*/
#include "lcd_stream_recv.h"
#include "webServer/webServerBlueprints.h"

#define WEBSERVER_PORT 8080

int main()
{
    // Start LCD thread
    lcdStreamRecv lcd;

    // Start webserver thread
    crow::SimpleApp app; //define your crow application
    add_routes(app); //add the routes to the app
    //set the port, set the app to run on multiple threads, and run the app
    app.port(WEBSERVER_PORT).multithreaded().run();

    


    return 0;
}