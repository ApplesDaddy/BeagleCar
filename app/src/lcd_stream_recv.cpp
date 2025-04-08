#include <stdio.h>
#include <stdbool.h>
#include "udp_constants.h"
#include "lcd_stream_recv.h"
#include <thread>
#include <iostream>

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavutil/avutil.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include <libavutil/opt.h>
    #include "LCD_1in54.h"
    #include "hal/lcd.h"
}

#define INBUF_SIZE 4096
// #define IP_ADDR "192.168.7.2"
// #define PORT "12346"
#define INPUT_URL_ARGS "?fifo_size=5000000&overrun_nonfatal=1"
#define INPUT_URL "udp://" CONTROLLER_IP ":" LCD_UDP_PORT INPUT_URL_ARGS

std::thread lcd_thread;

// source: https://ffmpeg.org/doxygen/4.2/decode_video_8c-example.html
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt, int stream_idx)
{
    if (!pkt || pkt->stream_index != stream_idx){ return; }

    int ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error sending a packet for decoding %d\n", ret);
        return;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        { return; }

        if (ret < 0)
        {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        lcd_show_frame(frame);
    }
}

void recv_video()
{
    std::cout << "RECV LCD\n";
    // open file
    // char* filename = "pic.mp4";
    // const char* filename = "udp://192.168.7.2:12346?fifo=size5000000&overrun_nonfatal=1";

    // open file
    AVFormatContext * format = NULL;
    if ( avformat_open_input( & format, INPUT_URL, NULL, NULL ) != 0 )
    {
        fprintf(stderr, "Could not get format context\n");
        exit(1);
    }

    if ( avformat_find_stream_info( format, NULL ) < 0)
    {
        fprintf(stderr, "Could not get stream info\n");
        exit(1);
    }

    // get index of video stream (since mp4 has audio and video streams)
    int video_stream_index = av_find_best_stream( format, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0 );
    if ( video_stream_index < 0 )
    {
        fprintf(stderr, "Could not get stream index\n");
        exit(1);
    }
    AVStream* videoStream = format->streams[ video_stream_index ];
    AVCodecParameters *codecpar = videoStream->codecpar;
    // get frame rate to limit writing to lcd
    // double fps = (double)videoStream->r_frame_rate.num / (double)videoStream->r_frame_rate.den;

    const AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec)
    {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    AVCodecParserContext *parser = av_parser_init(codec->id);
    if (!parser)
    {
        fprintf(stderr, "parser not found\n");
        exit(1);
    }
    parser->flags = PARSER_FLAG_COMPLETE_FRAMES;


    AVCodecContext *c = avcodec_alloc_context3(codec);
    if (!c)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    c->flags |= AV_CODEC_FLAG_LOW_DELAY;
    c->flags2 |= AV_CODEC_FLAG2_FAST;

    if(avcodec_parameters_to_context(c, codecpar) < 0){
        fprintf(stderr, "Failed to copy codec parameters to codec context\n");
        exit(1);
    }


    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    // uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    // memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);


    // TODO: look into include error
    AVDictionary *opt = NULL;
    av_dict_set(&opt, "preset", "ultrafast", 0);
    av_dict_set(&opt, "tune", "zerolatency", 0);

    /* open it */
    if (avcodec_open2(c, codec, &opt) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    AVPacket *pkt = av_packet_alloc();
    if (!pkt) { 
        fprintf(stderr, "Could not allocate packet\n");
        exit(1); 
    }

    AVFrame *frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
    MUST be initialized there because this information is not
    available in the bitstream. */
    frame->width = c->width;
    frame->height = c->height;


    lcd_video_init(c);

    // process file
    while (av_read_frame(format, pkt) >= 0)
    {
        decode(c, frame, pkt, video_stream_index);
        av_packet_unref(pkt);
    }

    /* flush the decoder */
    decode(c, frame, NULL, video_stream_index);


    // cleanup
    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);
}

lcdStreamRecv::lcdStreamRecv()
{
    // Initialize all modules; HAL modules first
    std::cout << "CONSTRUCT LCD\n";
    lcd_init();
    lcd_thread = std::thread(recv_video);
}

lcdStreamRecv::~lcdStreamRecv()
{
    std::cout << "DESTROY LCD\n";
    lcd_thread.join();
    lcd_cleanup();
}