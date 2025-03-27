#include "../include/vidStreamer/vidStreamer.h"
#include <iostream>

/**
references:
- https://github.com/loupus/ffmpeg_tutorial/
- https://www.youtube.com/playlist?list=PL4_PZmmnAGij12Uk3hifiEV9ESncwRfdI
 - ffmpeg official documentation and examples
**/


// TODO: better error handling? 
// refer to: https://stackoverflow.com/questions/43791772/proper-way-to-cleanup-in-a-function-with-multiple-return-points
VidStreamer::VidStreamer(std::string ip_addr, std::string filename)
{
    std::cout << "Initializing video streamer...\n";
    this->send_addr = ip_addr;
    this->filename = filename;
    this->outfilename = "sampleOut.yuv";

    if(!openFile()){
        return;
    }
    if(!decodeVideo()){
        return;
    }
    // startStream();
}

VidStreamer::~VidStreamer() 
{
    std::cout << "Destructor...\n";

    if(formatContext){
        std::cout << "Freeing AVFormatContext...\n";
        avformat_close_input(&formatContext);
    }

    if(codecContext){
        std::cout << "Freeing AVCodecContext...\n";
        avcodec_free_context(&codecContext);
    }

    if(frame){
        std::cout << "Freeing AV Frame...\n";
        av_frame_free(&frame);
    }

    if(packet){
        std::cout << "Freeing AV Packet...\n";
        av_packet_free(&packet);
    }

    if(inFile.is_open()){
        std::cout << "Closing input file...\n";
        inFile.close();
    }
    if(outFile.is_open()){
        std::cout << "Closing output file...\n";
        outFile.close();
    }
}



bool VidStreamer::initFormatContext(const char *filename)
{
    // allocate memory for format
    formatContext = avformat_alloc_context();

    // Open input stream and read header
    if(avformat_open_input(&formatContext, filename, NULL, NULL) < 0) {
        std::cout << "Error: Could not open file\n";
        return false;
    }
    // get stream info from format context
    if(avformat_find_stream_info(formatContext, NULL) < 0){
        std::cout << "Error: Could not find stream info\n";
        return false;
    }

    return true;    
}


bool VidStreamer::findVideoStream()
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
        return false;
    }

    // print metadata
    // av_dump_format(formatContext, streamIdx, filename.c_str(), 0); // 0 = input, 1 = output
    return true;
}


bool VidStreamer::initCodecContext()
{
    // Allocate memory for codec
    codecContext = avcodec_alloc_context3(NULL);

    // fill codec context
    if(avcodec_parameters_to_context(codecContext, formatContext->streams[streamIdx]->codecpar) < 0){
        std::cout << "Error: Cannot get codec parameters\n";
        return false;
    }

    // find decoding codec
    codec = avcodec_find_decoder(codecContext->codec_id);

    // Initialize the AVCodecContext to use the given AVCodec.
    // TODO: not threadsafe
    if(avcodec_open2(codecContext, codec, NULL) < 0){
        std::cout << "Error: Cannot open video decoder\n";
        return false;
    }

    std::cout << "Decoding codec is: " << codec->name << std::endl;

    return true;
}


bool VidStreamer::initPacket()
{
    packet = av_packet_alloc();
    if(!packet){
        std::cout << "Error: Could not initialize packet\n";
        return false;
    }

    std::cout << "Packet successfully initialized\n";
    return true;
}


bool VidStreamer::initFrame()
{
    frame = av_frame_alloc();
    if(!frame){
        std::cout << "Error: Could not initialize frame\n";
        return false;
    }
    std::cout << "Frame successfully initialized\n";
    return true;
}


bool VidStreamer::initFilePointers()
{
    this->inFile.open(this->filename, std::ifstream::in);
    if(!this->inFile){
        std::cout << "Error: Could not open input file\n";
        return false;
    } else{
        std::cout << "Successfully opened input file\n";
    }

    this->outFile.open(this->outfilename, std::ofstream::out | std::ofstream::binary);
    if(!this->outFile.is_open()){
        std::cout << "Error: Could not open output file\n";
        return false;
    } else {
        std::cout << "Successfully opened output file\n";
    }
    return true;
}


// Credit: https://community.gumlet.com/t/how-can-i-use-ffmpeg-or-ffplay-as-a-library-instead-of-calling-exe-in-c-c/1586/2
// Credit: https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
bool VidStreamer::openFile()
{
    const char *filename = this->filename.c_str();
    std::cout << "Opening file: " << this->filename << "...\n";

    // Demuxer (read media file and split into packets)
    if(!initFormatContext(filename)){
        return false;
    }
    if(!findVideoStream()){
        return false;
    }
    if(!initCodecContext()){
        return false;
    }

    return true;
}


/*
    The following function is adapted from: https://ffmpeg.org/doxygen/4.1/decode_video_8c-example.html
    with the help of https://www.youtube.com/watch?v=b_mYOAT1Q-M

    Dump video frames to .yuv file
    use: `ffplay -video_size 1920x1080 -i sampleOut.yuv` 
    to view
*/ 
void VidStreamer::pgmSave(const char *buf, int wrap, int xSize, int ySize, std::ofstream &outFile)
{
    outFile << "P5\n" << xSize << " " << ySize << "\n255\n";
    for (int i = 0; i < ySize; i++) {
        outFile.write(buf + i*wrap, xSize);
    }
}

/*
    The following function is adapted from: https://ffmpeg.org/doxygen/4.1/decode_video_8c-example.html
    with the help of https://www.youtube.com/watch?v=b_mYOAT1Q-M
*/ 
// TODO: error handling
void VidStreamer::decode(AVCodecContext *decContext, AVFrame *frame, AVPacket *pkt, std::ofstream *outFile)
{
    // char buf[1024];
    int ret = avcodec_send_packet(decContext, pkt);
    if(ret < 0){
        std::cout << "Error sending a packet for decoding\n";
        return;
    }
    while(ret >= 0){
        ret = avcodec_receive_frame(decContext, frame);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR(EOF)){
            return;
        } else if (ret < 0) {
            std::cout << "Error during decoding\n";
            return;
        }
        std::cout << "Saving frame " << decContext->frame_number << std::endl;
        fflush(stdout);
        // credit: https://stackoverflow.com/questions/54880043/argument-of-type-unsigned-char-is-incompatible-with-parameter-of-type-const
        pgmSave((const char*) frame->data[0], frame->linesize[0], frame->width, frame->height, *outFile);
    }
}


bool VidStreamer::decodeVideo()
{
    std::cout << "Decoding video...\n";
    if(!initPacket()){
        return false;
    }
    if(!initFrame()){
        return false;
    }
    if(!initFilePointers()){
        return false;
    }

    while(!inFile.eof()){
        // read raw data
        if(av_read_frame(formatContext, packet) < 0){
            std::cout << "Could not read frame\n";
            break;
        }
        if(packet->stream_index == streamIdx){
            decode(codecContext, frame, packet, &outFile);
        }
        av_packet_unref(packet);
    }

    // flush decoder
    decode(codecContext, frame, NULL, &outFile);

    return true;
}



void VidStreamer::startStream()
{
    std::cout << "Starting stream...\n";
    return;
}
