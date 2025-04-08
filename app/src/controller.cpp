/*
TODO: 
    2. Initialize joystick, rotary encoder, udp sender modules
        - Need receiver's IP address

    ?. Configure as AP?
*/
#include "lcd_stream_recv.h"
#include "webServer/webServerBlueprints.h"

extern "C"{
    #include "sender.h"
    #include "hal/rotary_encoder.h"
    #include "hal/joystick.h"
    #include "hal/gpio.h"
}

#define WEBSERVER_PORT 8080

int main()
{
    // init hardware
    gpio_init();
    joystick_init();
    rot_encoder_init();

    // init udp sender
    send_udp_init(false);

    // Start LCD thread
    lcdStreamRecv lcd;

    // Start webserver thread
    crow::SimpleApp app; //define your crow application
    add_routes(app); //add the routes to the app
    //set the port, set the app to run on multiple threads, and run the app
    app.port(WEBSERVER_PORT).multithreaded().run();


    send_udp_cleanup();
    rot_encoder_cleanup();
    joystick_cleanup();
    gpio_cleanup();


    return 0;
}