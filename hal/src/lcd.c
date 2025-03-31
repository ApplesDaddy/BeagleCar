#include "hal/lcd.h"

#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"

#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <signal.h>     //signal()
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <assert.h>
#include <pthread.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>


static UWORD *s_fb;
static bool isInitialized = false;

static struct SwsContext *resize;
static int context_height;
static AVFrame* queued_frame;
static AVFrame* buffered_frame;
static atomic_bool dirty_frame = false;

static pthread_mutex_t frame_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t frame_cond = PTHREAD_COND_INITIALIZER;
static bool frame_thread_active = false;
static atomic_bool stop_thread = false;
static pthread_t frame_thread;

#define CLIP(x) ( (x) > 255 ? 255 : (x) < 0 ? 0 : (x) )
void* frame_thread_func(void* args)
{
    (void)args;

    do
    {
        // wait for frame to be given
        pthread_mutex_lock(&frame_lock);
        while(!dirty_frame && !stop_thread)
        { pthread_cond_wait(&frame_cond, &frame_lock); }

        if(!dirty_frame) // should only be if we're quitting
        { pthread_mutex_unlock(&frame_lock); continue; }

        // copy frame so main thread can queue a different frame
        // scale down to fit and store locally
        sws_scale(resize, buffered_frame->data, buffered_frame->linesize, 0, context_height,
                  queued_frame->data, queued_frame->linesize);
        dirty_frame = false;

        pthread_mutex_unlock(&frame_lock);


        // display frame
        Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
        Paint_Clear(WHITE);

        assert(isInitialized);

        int h = queued_frame->height < LCD_1IN54_HEIGHT? queued_frame->height : LCD_1IN54_HEIGHT;
        int w = queued_frame->width < LCD_1IN54_WIDTH? queued_frame->width : LCD_1IN54_WIDTH;
        for(int row = 0; row < h; row++)
        {
            for(int col = 0; col < w; col++)
            {
                // source: https://stackoverflow.com/questions/23761786/using-ffplay-or-ffmpeg-how-can-i-get-a-pixels-rgb-value-in-a-frame
                // get yuv value at pixel
                int y = queued_frame->data[0][queued_frame->linesize[0] * row + col];
                int u = queued_frame->data[1][(int)(queued_frame->linesize[1] * (row/2.0) + (col/2.0))];
                int v = queued_frame->data[2][(int)(queued_frame->linesize[2] * (row/2.0) + (col/2.0))];

                // convert to rgb
                int r = y + 1.402 * (v - 128);
                int g = y - 0.344 * (u - 128) - 0.714 * (v - 128);
                int b = y + 1.772 * (u - 128);

                // prevent overflow/underflow
                r = CLIP(r);
                g = CLIP(g);
                b = CLIP(b);

                int data=RGB((r), (g), (b));
                Paint_SetPixel(col, row, data);
            }
        }
        LCD_1IN54_Display(s_fb);
    }
    while(!stop_thread);
    pthread_mutex_unlock(&frame_lock);

    return NULL;
}


// source: cmake_lcdstarter from prof Brian Fraser
void lcd_init()
{
    assert(!isInitialized);

    // Exception handling:ctrl + c
    // signal(SIGINT, Handler_1IN54_LCD);

    // Module Init
	if(DEV_ModuleInit() != 0)
    {
        DEV_ModuleExit();
        exit(0);
    }

    // LCD Init
    DEV_Delay_ms(2000);
	LCD_1IN54_Init(HORIZONTAL);
	LCD_1IN54_Clear(WHITE);
	LCD_SetBacklight(1023);

    UDOUBLE Imagesize = LCD_1IN54_HEIGHT*LCD_1IN54_WIDTH*2;
    if((s_fb = (UWORD *)malloc(Imagesize)) == NULL)
    {
        perror("Failed to apply for black memory");
        exit(0);
    }

    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);

    isInitialized = true;
}

void lcd_video_init(AVCodecContext *context)
{
    assert(isInitialized);

    // setup to resize image to fit lcd screen
    resize = sws_getContext(context->width, context->height, context->pix_fmt, LCD_1IN54_WIDTH,
                            LCD_1IN54_HEIGHT, context->pix_fmt, SWS_SINC, NULL, NULL, NULL);

    // second frame for scaled image
    queued_frame = av_frame_alloc();
    queued_frame->width = LCD_1IN54_WIDTH;
    queued_frame->height = LCD_1IN54_HEIGHT;
    queued_frame->format = context->pix_fmt;
    int ret = av_frame_get_buffer(queued_frame, 0);
    ret = av_frame_make_writable(queued_frame);

    context_height = context->height;

    stop_thread = false;
    frame_thread_active = true;
    pthread_create(&frame_thread, NULL, frame_thread_func, NULL);
}

// source: cmake_lcdstarter from prof Brian Fraser
void lcd_cleanup()
{
    assert(isInitialized);

    if(frame_thread_active)
    {
        stop_thread = true;
        pthread_cond_signal(&frame_cond);
        pthread_join(frame_thread, NULL);
    }
    av_frame_free(&queued_frame);
    av_frame_free(&buffered_frame);

    // Module Exit
    free(s_fb);
    s_fb = NULL;
	DEV_ModuleExit();
    isInitialized = false;
}

// source: cmake_lcdstarter from prof Brian Fraser
void lcd_show_bmp(char* filename)
{
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

	GUI_ReadBmp(filename);
    LCD_1IN54_Display(s_fb);
    DEV_Delay_ms(2000);
}

// source: cmake_lcdstarter from prof Brian Fraser
void lcd_show_message(char* message)
{
    assert(isInitialized);

    const int x = 5;
    const int y = 70;

    // Initialize the RAM frame buffer to be blank (white)
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    // Draw into the RAM frame buffer
    // WARNING: Don't print strings with `\n`; will crash!
    Paint_DrawString_EN(x, y, message, &Font16, WHITE, BLACK);

    // Send the RAM frame buffer to the LCD (actually display it)
    // Option 2) Update just a small window (~15 updates / second)
    //           Assume font height <= 20
    LCD_1IN54_DisplayWindows(x, y, LCD_1IN54_WIDTH, y+20, s_fb);
}

void lcd_show_frame(AVFrame * frame)
{
    assert(isInitialized);

    pthread_mutex_lock(&frame_lock);

    buffered_frame = av_frame_clone(frame);    // increment ref counter, not deep copy
    dirty_frame = true;
    pthread_cond_signal(&frame_cond);

    pthread_mutex_unlock(&frame_lock);
}
