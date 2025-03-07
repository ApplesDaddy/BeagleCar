#include "../include/vidStreamer/vidStreamer.h"
#include <iostream>

VidStreamer::VidStreamer(std::string ip_addr) : ip_addr(ip_addr) {}

VidStreamer::~VidStreamer() {}

void openFile(std::string filename)
{
    std::cout << "Opening file: " << filename << "...\n";
    return;
}


void startStream()
{
    std::cout << "Starting stream...\n";
    return;
}


void vidStreamer_init(std::string filename)
{
    std::string f = filename;
    std::cout << "Starting vidStreamer threads...\n";
    return;
}


void vidStreamer_cleanup()
{
    std::cout << "Joining vidStreamer threads...\n";
    return;
}