#include "hal/lcd.h"

#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"

#include <stdio.h>		//printf()
#include <stdlib.h>		//exit()
#include <signal.h>     //signal()
#include <stdbool.h>
#include <assert.h>


static UWORD *s_fb;
static bool isInitialized = false;


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

    // avcodec_register_all();

    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    isInitialized = true;
}
// source: cmake_lcdstarter from prof Brian Fraser
void lcd_cleanup()
{
    assert(isInitialized);

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
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
    Paint_Clear(WHITE);

    assert(isInitialized);

    int h = frame->height < LCD_1IN54_HEIGHT? frame->height : LCD_1IN54_HEIGHT;
    int w = frame->width < LCD_1IN54_WIDTH? frame->width : LCD_1IN54_WIDTH;
	for(int row = 0; row < h; row++)
	{
		for(int col = 0; col < w; col++)
		{
            // source: https://stackoverflow.com/questions/23761786/using-ffplay-or-ffmpeg-how-can-i-get-a-pixels-rgb-value-in-a-frame
            // get yuv value at pixel
            unsigned char y = frame->data[0][frame->linesize[0] * row + col];
            unsigned char u = frame->data[1][(int)(frame->linesize[1] * (row/2.0) + (col/2.0))];
            unsigned char v = frame->data[2][(int)(frame->linesize[2] * (row/2.0) + (col/2.0))];

            // convert to rgb
            const unsigned char r = y + 1.402 * (v-128);
            const unsigned char g = y - 0.344 * (u-128) - 0.714 * (v-128);
            const unsigned char b = y + 1.772 * (u-128);

            int data=RGB((r), (g), (b));
            Paint_SetPixel(col, row, data);
        }
	}
    LCD_1IN54_Display(s_fb);
}
