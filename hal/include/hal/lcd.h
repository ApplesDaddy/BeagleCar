/**
 * part of the HAL layer
 * lcd.h
 *
 * Methods to interact with the LCD.
 */
#ifndef _LCD_H_
#define _LCD_H_

#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

void lcd_init();
void lcd_video_init(AVCodecContext *context);
void lcd_cleanup();

void lcd_show_message(char *message);
void lcd_show_bmp(char *filename);
void lcd_show_frame(AVFrame *frame); // make sure frame is in AV_PIX_FMT_YUYV422 format

#endif