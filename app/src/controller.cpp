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

int main(int argc, char* argv[])
{
    if(argc < 2){
        std::cout << "Usage: " << argv[0] << "<ip_address>" << std::endl;
        return 1;
    }

    std::cout << "argc " << argc << std::endl;
    // init hardware
    gpio_init();
    joystick_init();
    rot_encoder_init();
    rot_encoder_set_min_max(0, 100);
    rot_encoder_set_step(5);

    // init udp sender
    char *ip = argv[1];
    send_udp_init(false, ip);

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