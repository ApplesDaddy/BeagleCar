#include "../../vidStreamer/include/vidStreamer/vidStreamer.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cerr << "Usage: " << argv[0] << "<input_file>" << std::endl;
        return 1;
    }

    std::string ip_addr = "udp://192.168.7.2:12345";
    std::string filename = argv[1];

    // std::cout << "You have entered: " << filename << std::endl;

    VidStreamer streamer(ip_addr, argv[1]);


    return 0;
}