#ifndef _VIDSTREAMER_H_
#define _VIDSTREAMER_H_


extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    // #include <libavutil/imgutils.h>
    // #include <libavutil/avutil.h>
    // #include <libswscale/swscale.h>
    }
#include <fstream>
#include <string>

// reference: https://friendlyuser.github.io/posts/tech/cpp/Using_FFmpeg_in_C++_A_Comprehensive_Guide/

class VidStreamer
{
public:
    VidStreamer(std::string ip_addr, std::string filename); // constructor

    ~VidStreamer();



private:
    void openFile();
    void initFormatContext(const char *filename);
    void findVideoStream();
    void initCodecContext();
    void startStream();

    std::string send_addr;
    std::string filename;

    AVFormatContext *formatContext = NULL;
    AVCodecContext *codecContext = NULL;
    const AVCodec *codec = NULL;
    int streamIdx = -1;
};


#endif