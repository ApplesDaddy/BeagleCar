// Main program to build the application
// Has main(); does initialization and cleanup and perhaps some basic logic.

#include <stdio.h>
#include <stdbool.h>
#include <libavcodec/avcodec.h>

#include "hal/lcd.h"

#define INBUF_SIZE 4096


// source: https://ffmpeg.org/doxygen/4.2/decode_video_8c-example.html
static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt)
{
    int ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error sending a packet for decoding\n");
        exit(1);
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
    }
}

int main()
{
    printf("Hello world with LCD!\n");

    // Initialize all modules; HAL modules first
    lcd_init();


    // open file
    char* filename = "pic.mp4";
    AVPacket *pkt = av_packet_alloc();
    if (!pkt)
    { exit(1); }

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

    /* find the MPEG-4 decoder */
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
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

    AVCodecContext *c = avcodec_alloc_context3(codec);
    if (!c)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */
    c->height = 540;
    c->width = 960;

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    AVFrame *frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    while (!feof(f))
    {
        /* read raw data from the input file */
        size_t data_size = fread(inbuf, 1, INBUF_SIZE, f);
        if (!data_size)
        { break; }

        /* use the parser to split the data into frames */
        uint8_t *data = inbuf;
        while (data_size > 0)
        {
            int ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size, data, data_size,
                                        AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0)
            {
                fprintf(stderr, "Error while parsing\n");
                exit(1);
            }
            data += ret;
            data_size -= ret;

            // parse into AVFrame
            decode(c, frame, pkt);
            lcd_show_frame(frame);
        }
    }

    /* flush the decoder */
    decode(c, frame, NULL);

    fclose(f);

    av_parser_close(parser);
    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);


    // Cleanup all modules (HAL modules last)
    lcd_cleanup();

    printf("!!! DONE !!!\n");
}