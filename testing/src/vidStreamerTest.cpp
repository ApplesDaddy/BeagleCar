#include "../../vidStreamer/include/vidStreamer/vidStreamer.h"
#include <iostream>
#include <string>
#include <thread>

void create_vid_streamer(std::string addr, std::string filename) { VidStreamer streamer(addr, filename); }

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "<input_file>" << std::endl;
        return 1;
    }

    std::string ip_addr = "udp://192.168.7.2";
    std::string web_port = "12345";
    std::string lcd_port = "12346";

    std::string web_addr = ip_addr + ":" + web_port;
    std::string lcd_addr = ip_addr + ":" + lcd_port;
    std::string filename = argv[1];

    // std::cout << "You have entered: " << filename << std::endl;

    std::thread web_thread(create_vid_streamer, web_addr, filename);
    std::thread lcd_thread(create_vid_streamer, lcd_addr, filename);

    web_thread.join();
    lcd_thread.join();

    return 0;
}