#ifndef _VIDSTREAMER_H_
#define _VIDSTREAMER_H_


extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/opt.h>
    // #include <libavutil/imgutils.h>
    // #include <libavutil/avutil.h>
    // #include <libswscale/swscale.h>
    }
#include <fstream>
#include <string>

class VidStreamer
{
public:
    VidStreamer(std::string ip_addr, std::string filename); // constructor

    ~VidStreamer();



private:
    bool openFile();
    bool initInputContext(const char *filename);
    bool findInputStream();
    bool initDecoder();

    bool setupOutputStream();
    bool initOutputContext();
    bool initOutputStream();
    bool initEncoder();
    bool initPackets();
    bool initInputFrame();
    bool initInputFile();

    void startOutputStream();

    std::string send_addr;

    std::string filename;
    std::ifstream inFile;

    AVFrame *inputFrame = NULL;

    AVPacket *inputPacket = NULL;
    AVPacket *outputPacket = NULL;

    AVFormatContext *inputFormatContext = NULL;
    AVFormatContext *outputFormatContext = NULL;

    const AVCodec *encoder = NULL;
    AVCodecContext *encoderContext = NULL;

    const AVCodec *decoder = NULL;
    AVCodecContext *decoderContext = NULL;

    int inputStreamIdx = -1;
    AVStream *outputStream;

    AVDictionary *dict = NULL;


    bool decodeVideo();

};


#endif