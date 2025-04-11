#include "../include/vidStreamer/vidStreamer.h"
#include <iostream>

#define FORMAT_NAME "mpegts"

/**
references:
    - https://github.com/loupus/ffmpeg_tutorial/
    - https://www.youtube.com/playlist?list=PL4_PZmmnAGij12Uk3hifiEV9ESncwRfdI
    - ffmpeg official documentation and examples
    - https://stackoverflow.com/questions/32254897/streaming-h-264-over-udp-using-ffmpeg-and-dimensions-not-set-error
    - https://github.com/leandromoreira/ffmpeg-libav-tutorial
    - https://stackoverflow.com/questions/40825300/ffmpeg-create-rtp-stream
    - https://trac.ffmpeg.org/wiki/StreamingGuide
    - https://friendlyuser.github.io/posts/tech/cpp/Using_FFmpeg_in_C++_A_Comprehensive_Guide/
**/

// TODO: threading

// TODO: better error handling?
// refer to:
// https://stackoverflow.com/questions/43791772/proper-way-to-cleanup-in-a-function-with-multiple-return-points
VidStreamer::VidStreamer(std::string ip_addr, std::string filename) {
    std::cout << "Initializing video streamer...\n";
    this->send_addr = ip_addr;
    this->filename = filename;

    if (!openFile()) {
        return;
    }
    if (!setupOutputStream()) {
        return;
    }
    // TODO: error checking
    startOutputStream();
}

VidStreamer::~VidStreamer() {
    std::cout << "Destructor...\n";

    if (inputFormatContext) {
        std::cout << "Freeing input format context...\n";
        avformat_close_input(&inputFormatContext);
    }

    if (decoderContext) {
        std::cout << "Freeing AVCodecContext...\n";
        avcodec_free_context(&decoderContext);
    }

    if (inputFrame) {
        std::cout << "Freeing AV Frame...\n";
        av_frame_free(&inputFrame);
    }

    if (inputPacket) {
        std::cout << "Freeing Input Packet...\n";
        av_packet_free(&inputPacket);
    }

    if (outputPacket) {
        std::cout << "Freeing Output Packet...\n";
        av_packet_free(&outputPacket);
    }

    if (inFile.is_open()) {
        std::cout << "Closing input file...\n";
        inFile.close();
    }

    if (encoderContext) {
        std::cout << "Freeing encoder context...\n";
        avcodec_free_context(&encoderContext);
    }

    if (outputFormatContext) {
        std::cout << "Freeing output format context...\n";
        avformat_free_context(outputFormatContext);
    }

    // TODO: not sure why but dictionary does not get freed...
    if (dict) {
        std::cout << "Freeing dictionary...\n";
        av_dict_free(&dict);
    }
}

bool VidStreamer::initInputContext(const char *filename) {
    // allocate memory for format
    inputFormatContext = avformat_alloc_context();

    // Open input stream and read header
    if (avformat_open_input(&inputFormatContext, filename, NULL, NULL) < 0) {
        std::cout << "Error: Could not open file\n";
        return false;
    }
    // get stream info from format context
    if (avformat_find_stream_info(inputFormatContext, NULL) < 0) {
        std::cout << "Error: Could not find stream info\n";
        return false;
    }

    return true;
}

// set up input stream
bool VidStreamer::findInputStream() {
    // get stream index
    for (uint i = 0; i < inputFormatContext->nb_streams; i++) {
        // check if stream codec is a video
        if (inputFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            inputStreamIdx = i;
            break;
        }
    }

    if (inputStreamIdx < 0) {
        std::cout << "Error: Stream codec is not a video type\n";
        return false;
    }

    // print metadata
    // av_dump_format(inputFormatContext, inputStreamIdx, filename.c_str(), 0); // 0 = input, 1 = output
    return true;
}

bool VidStreamer::initDecoder() {
    // Allocate memory for decoder
    decoderContext = avcodec_alloc_context3(NULL);

    // fill decoder context
    if (avcodec_parameters_to_context(decoderContext, inputFormatContext->streams[inputStreamIdx]->codecpar) < 0) {
        std::cout << "Error: Could not get input stream decoder parameters\n";
        return false;
    }

    // find decoding codec
    decoder = avcodec_find_decoder(decoderContext->codec_id);

    // Initialize the AVCodecContext to use the given AVCodec.
    // TODO: not threadsafe
    if (avcodec_open2(decoderContext, decoder, NULL) < 0) {
        std::cout << "Error: Cannot open video decoder\n";
        return false;
    }

    std::cout << "Decoding codec is: " << decoder->name << std::endl;

    return true;
}

bool VidStreamer::initPackets() {
    inputPacket = av_packet_alloc();
    if (!inputPacket) {
        std::cout << "Error: Could not initialize inputPacket\n";
        return false;
    }

    outputPacket = av_packet_alloc();
    if (!outputPacket) {
        std::cout << "Error: Could not initialize outputPacket\n";
        return false;
    }

    std::cout << "Packet successfully initialized\n";
    return true;
}

bool VidStreamer::initInputFrame() {
    inputFrame = av_frame_alloc();
    if (!inputFrame) {
        std::cout << "Error: Could not initialize inputFrame\n";
        return false;
    }
    std::cout << "Frame successfully initialized\n";
    return true;
}

bool VidStreamer::initInputFile() {
    this->inFile.open(this->filename, std::ifstream::in);
    if (!this->inFile) {
        std::cout << "Error: Could not open input file\n";
        return false;
    } else {
        std::cout << "Successfully opened input file\n";
    }

    return true;
}

// Credit:
// https://community.gumlet.com/t/how-can-i-use-ffmpeg-or-ffplay-as-a-library-instead-of-calling-exe-in-c-c/1586/2
// Credit: https://stackoverflow.com/questions/347949/how-to-convert-a-stdstring-to-const-char-or-char
bool VidStreamer::openFile() {
    const char *filename = this->filename.c_str();
    std::cout << "Opening file: " << this->filename << "...\n";

    // Demuxer (read media file and split into packets)
    if (!initInputContext(filename)) {
        return false;
    }
    if (!findInputStream()) {
        return false;
    }
    // used for decoder
    if (!initDecoder()) {
        return false;
    }

    return true;
}

// TODO: may be used later to break up startOutputStream() function
bool VidStreamer::decodeVideo() {
    std::cout << "Decoding video...\n";
    if (!initPackets()) {
        return false;
    }
    if (!initInputFrame()) {
        return false;
    }
    if (!initInputFile()) {
        return false;
    }

    while (!inFile.eof()) {
        // read raw data
        if (av_read_frame(inputFormatContext, inputPacket) < 0) {
            std::cout << "Could not read inputFrame\n";
            break;
        }
        if (inputPacket->stream_index == inputStreamIdx) {
            // TODO: send packet to encoder
        }
        av_packet_unref(inputPacket);
    }

    return true;
}

bool VidStreamer::setupOutputStream() {
    std::cout << "Setting up stream...\n";
    if (!initOutputContext()) {
        return false;
    }
    if (!initEncoder()) {
        return false;
    }
    if (!initOutputStream()) {
        return false;
    }
    if (!initPackets()) {
        return false;
    }
    if (!initInputFrame()) {
        return false;
    }
    if (!initInputFile()) {
        return false;
    }

    return true;
}

bool VidStreamer::initOutputContext() {
    avformat_alloc_output_context2(&outputFormatContext, NULL, FORMAT_NAME, send_addr.c_str());
    if (!outputFormatContext) {
        std::cout << "Error: Could not create output context\n";
        return false;
    }

    if (avio_open(&outputFormatContext->pb, send_addr.c_str(), AVIO_FLAG_WRITE) < 0) {
        std::cout << "Error: Could not access resource indicated by URL\n";
        return false;
    }

    std::cout << "Output format: " << outputFormatContext->oformat->name << std::endl;
    std::cout << "Sending stream to " << send_addr << std::endl;
    return true;
}

bool VidStreamer::initOutputStream() {
    outputStream = avformat_new_stream(outputFormatContext, NULL);
    if (!outputStream) {
        std::cout << "Error: Could not create output stream\n";
        return false;
    }

    if (avcodec_parameters_from_context(outputStream->codecpar, encoderContext) < 0) {
        std::cout << "Error: Could not set output stream codec parameters";
        return false;
    }
    return true;
}

bool VidStreamer::initEncoder() {
    encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!encoder) {
        std::cout << "Error: Could not find encoder\n";
        return false;
    }
    std::cout << "Encoder found\n";

    encoderContext = avcodec_alloc_context3(encoder);
    // encoderContext codec options
    //  credit: https://stackoverflow.com/questions/40825300/ffmpeg-create-rtp-stream
    encoderContext->bit_rate = decoderContext->bit_rate;
    // same dimensions as input file
    encoderContext->width = decoderContext->width;
    encoderContext->height = decoderContext->height;
    encoderContext->pix_fmt = AV_PIX_FMT_YUV420P;
    encoderContext->time_base.num = 1;
    encoderContext->time_base.den = 25;

    // set following presets for real-time streaming
    av_dict_set(&dict, "tune", "zerolatency", 0);
    av_dict_set(&dict, "preset", "ultrafast", 0);

    if (avcodec_open2(encoderContext, encoder, &dict) < 0) {
        std::cout << "Failed to open encoder\n";
        return false;
    }

    return true;
}

/*
    The following function is adapted from the following sources:
        - https://ffmpeg.org/doxygen/4.1/decode_video_8c-example.html
        - https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/3_transcoding.c
        - https://stackoverflow.com/questions/40825300/ffmpeg-create-rtp-stream
*/
// TODO: break up function into smaller chunks
void VidStreamer::startOutputStream() {
    // Write stream header to output media file
    if (avformat_write_header(outputFormatContext, NULL) < 0) {
        std::cout << "Failed to write header\n";
        return;
    }
    std::cout << "Starting stream...\n";

    while (!inFile.eof()) {

        // read input data
        if (av_read_frame(inputFormatContext, inputPacket) < 0) {
            std::cout << "Could not read inputFrame\n";
            break;
        }

        // decode packet
        if (inputPacket->stream_index == inputStreamIdx) {
            int ret = avcodec_send_packet(decoderContext, inputPacket);
            if (ret < 0) {
                std::cout << "Error: Could not send packet for decoding\n";
                continue;
            }

            while (ret >= 0) {
                ret = avcodec_receive_frame(decoderContext, inputFrame);
                if (ret == AVERROR(EAGAIN)) {
                    // std::cout << "No more data for decoder to receive\n";
                    break;
                } else if (ret == AVERROR(EOF)) {
                    std::cout << "Decoder reached end of file\n";
                    break;
                } else if (ret < 0) {
                    std::cout << "Error during decoding\n";
                    break;
                    ;
                }

                // encode packet
                int response = avcodec_send_frame(encoderContext, inputFrame);
                if (response < 0) {
                    std::cout << "Error sending inputFrame to encoder\n";
                    break;
                }

                while (response >= 0) {
                    response = avcodec_receive_packet(encoderContext, outputPacket);
                    if (response == AVERROR(EAGAIN)) {
                        // std::cout << "No more data for encoder to receive\n";
                        break;
                    } else if (response == AVERROR(EOF)) {
                        std::cout << "Encoder reached end of file\n";
                        break;
                    } else if (response < 0) {
                        std::cout << "Error receiving packet from encoder\n";
                        break;
                    }

                    av_packet_rescale_ts(outputPacket, encoderContext->time_base, outputStream->time_base);
                    response = av_interleaved_write_frame(outputFormatContext, outputPacket);
                    if (response != 0) {
                        std::cout << "Error while receiving packet from decoder\n";
                        break;
                    }
                }
                av_packet_unref(outputPacket);
            }
        }
        av_frame_unref(inputFrame);
        av_packet_unref(inputPacket);
    }
    av_write_trailer(outputFormatContext);
}