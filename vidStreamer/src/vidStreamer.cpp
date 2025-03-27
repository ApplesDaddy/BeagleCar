#include "../include/vidStreamer/vidStreamer.h"
#include <iostream>

VidStreamer::VidStreamer(std::string ip_addr, std::string filename)
{
    this->send_addr = ip_addr;
    this->file = filename;

    openFile(this->file);
    startStream();
}

VidStreamer::~VidStreamer() 
{
    std::cout << "Destructor" << std::endl;
}

// Credit: https://community.gumlet.com/t/how-can-i-use-ffmpeg-or-ffplay-as-a-library-instead-of-calling-exe-in-c-c/1586/2
// Credit: https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
void VidStreamer::openFile(std::string filename)
{
    std::cout << "Opening file: " << filename << "...\n";
    AVFormatContext *formatContext = avformat_alloc_context();
    if(avformat_open_input(&formatContext, filename.c_str(), NULL, NULL) < 0) {
        std::cout << "Error: Could not open file\n";
    }
    avformat_free_context(formatContext);
    return;
}


void VidStreamer::startStream()
{
    std::cout << "Starting stream...\n";
    return;
}
