#ifndef __IPS_LCD_H__
#define __IPS_LCD_H__

void ips_lcd_init(void);

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[]);
void LCD_Fill(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint16_t color);

#endif
