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

// reference: https://friendlyuser.github.io/posts/tech/cpp/Using_FFmpeg_in_C++_A_Comprehensive_Guide/

class VidStreamer
{
public:
    VidStreamer(std::string ip_addr, std::string filename); // constructor

    ~VidStreamer();



private:
    bool openFile();
    bool initInputContext(const char *filename);
    bool findVideoStream();
    bool initCodecContext();

    bool setupStream();
    bool initOutputContext();
    bool initOutputStream();
    bool initEncoder();
    bool initPacket();
    bool initFrame();

    void startStream();

    std::string send_addr;

    std::string filename;

    AVFrame *frame = NULL;
    AVPacket *inputPacket = NULL;
    AVPacket *outputPacket = NULL;

    AVFormatContext *inputFormatContext = NULL;
    AVFormatContext *outputFormatContext = NULL;

    const AVCodec *encoder = NULL;
    AVCodecContext *encoderContext = NULL;

    AVCodecContext *codecContext = NULL;
    const AVCodec *codec = NULL;
    int streamIdx = -1;

    AVStream *outputStream;

    AVDictionary *dict = NULL;



    void decode(AVCodecContext *decContext, AVFrame *frame, AVPacket *pkt, std::ofstream *outFile);
    bool decodeVideo();
    void pgmSave(const char *buf, int wrap, int xSize, int ySize, std::ofstream &outFile);
    bool initFilePointers();


    std::string outfilename;
    std::ofstream outFile;
    std::ifstream inFile;






};


#endif