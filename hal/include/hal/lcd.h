#ifndef _LCD_H_
#define _LCD_H_

#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
void lcd_init();
void lcd_cleanup();

void lcd_update_screen(char* message);
void lcd_show_bmp(char* filename);
void lcd_show_frame(AVFrame * frame);

#endif