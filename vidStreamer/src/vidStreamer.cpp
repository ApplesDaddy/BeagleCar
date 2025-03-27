#include "../include/vidStreamer/vidStreamer.h"
#include <iostream>

/**
references:
- https://github.com/loupus/ffmpeg_tutorial/
- https://www.youtube.com/playlist?list=PL4_PZmmnAGij12Uk3hifiEV9ESncwRfdI
 
**/



VidStreamer::VidStreamer(std::string ip_addr, std::string filename)
{
    std::cout << "Initializing video streamer...\n";
    this->send_addr = ip_addr;
    this->filename = filename;

    openFile();
    // startStream();
}

VidStreamer::~VidStreamer() 
{
    std::cout << "Destructor...\n";

    std::cout << "Freeing AVFormatContext...\n";
    avformat_close_input(&formatContext);
    avcodec_free_context(&codecContext);
}

// Credit: https://community.gumlet.com/t/how-can-i-use-ffmpeg-or-ffplay-as-a-library-instead-of-calling-exe-in-c-c/1586/2
// Credit: https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
void VidStreamer::openFile()
{
    const char *filename = this->filename.c_str();
    std::cout << "Opening file: " << this->filename << "...\n";

    // Demuxer (read media file and split into packets)
    initFormatContext(filename);
    findVideoStream();
    initCodecContext();

    return;
}


void VidStreamer::initFormatContext(const char *filename)
{
    // allocate memory for format
    formatContext = avformat_alloc_context();

    // Open input stream and read header
    if(avformat_open_input(&formatContext, filename, NULL, NULL) < 0) {
        std::cout << "Error: Could not open file\n";
        return;
    }
    // get stream info from format context
    if(avformat_find_stream_info(formatContext, NULL) < 0){
        std::cout << "Error: Could not find stream info\n";
    }

    return;    
}


void VidStreamer::findVideoStream()
{
    // get stream index
    for(uint i = 0; i < formatContext->nb_streams; i++){
        // check if stream codec is a video
        if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            streamIdx = i;
            break;
        }
    }

    if(streamIdx < 0){
        std::cout << "Error: Stream codec is not a video type\n";
        return;
    }

    // print metadata
    // av_dump_format(formatContext, streamIdx, filename.c_str(), 0); // 0 = input, 1 = output
    return;
}


void VidStreamer::initCodecContext()
{
    // Allocate memory for codec
    codecContext = avcodec_alloc_context3(NULL);

    // fill codec context
    if(avcodec_parameters_to_context(codecContext, formatContext->streams[streamIdx]->codecpar) < 0){
        std::cout << "Error: Cannot get codec parameters\n";
        return;
    }

    // find decoding codec
    codec = avcodec_find_decoder(codecContext->codec_id);

    // Initialize the AVCodecContext to use the given AVCodec.
    // TODO: not threadsafe
    if(avcodec_open2(codecContext, codec, NULL) < 0){
        std::cout << "Error: Cannot open video decoder\n";
        return;
    }

    std::cout << "Decoding codec is: " << codec->name << std::endl;

    return;
}


void VidStreamer::startStream()
{
    std::cout << "Starting stream...\n";
    return;
}
