/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>
#include <unistd.h>
#include "lcd.h"
#include "nt35310.h"
#include "font.h"
#include "sleep.h"
#include "board_config.h"

static lcd_ctl_t lcd_ctl;

void lcd_polling_enable(void)
{
    lcd_ctl.mode = 0;
}

void lcd_interrupt_enable(void)
{
    lcd_ctl.mode = 1;
}

void tft_write_onebyte(uint8_t data)
{
    tft_write_byte(&data, 1);
}

void lcd_init(void)
{
    uint8_t data = 0;
    tft_hard_init();
    /*soft reset*/
    tft_write_command(SOFTWARE_RESET);
    usleep(100000);

#if (BOARD == BOARD_MAIX_NEW_GO)
    uint8_t t[15];

    tft_write_command(0XF1); /* Unk */
    t[0] = (0x36);
    t[1] = (0x04);
    t[2] = (0x00);
    t[3] = (0x3C);
    t[4] = (0X0F);
    t[5] = (0x8F);
    tft_write_byte(t, 6);

    tft_write_command(0XF2); /* Unk */
    t[0] = (0x18);
    t[1] = (0xA3);
    t[2] = (0x12);
    t[3] = (0x02);
    t[4] = (0XB2);
    t[5] = (0x12);
    t[6] = (0xFF);
    t[7] = (0x10);
    t[8] = (0x00);
    tft_write_byte(t, 9);

    tft_write_command(0XF8); /* Unk */
    t[0] = (0x21);
    t[1] = (0x04);
    tft_write_byte(t, 2);

    tft_write_command(0XF9); /* Unk */
    t[0] = (0x00);
    t[1] = (0x08);
    tft_write_byte(t, 2);

    tft_write_command(0x36); /* Memory Access Control */
    t[0] = (0x28);
    tft_write_byte(t, 1);

    tft_write_command(0xB4); /* Display Inversion Control */
    t[0] = (0x00);
    tft_write_byte(t, 1);

    tft_write_command(0xB6); /* Display Function Control */
    t[0] = (0x02);
    // t[1] = (0x22);
    tft_write_byte(t, 1);

    tft_write_command(0xC1); /* Power Control 2 */
    t[0] = (0x41);
    tft_write_byte(t, 1);

    tft_write_command(0xC5); /* Vcom Control */
    t[0] = (0x00);
    t[1] = (0x18);
    tft_write_byte(t, 2);

    tft_write_command(0xE0); /* Positive Gamma Control */
    t[0] = (0x0F);
    t[1] = (0x1F);
    t[2] = (0x1C);
    t[3] = (0x0C);
    t[4] = (0x0F);
    t[5] = (0x08);
    t[6] = (0x48);
    t[7] = (0x98);
    t[8] = (0x37);
    t[9] = (0x0A);
    t[10] = (0x13);
    t[11] = (0x04);
    t[12] = (0x11);
    t[13] = (0x0D);
    t[14] = (0x00);
    tft_write_byte(t, 15);

    tft_write_command(0xE1); /* Negative Gamma Control */
    t[0] = (0x0F);
    t[1] = (0x32);
    t[2] = (0x2E);
    t[3] = (0x0B);
    t[4] = (0x0D);
    t[5] = (0x05);
    t[6] = (0x47);
    t[7] = (0x75);
    t[8] = (0x37);
    t[9] = (0x06);
    t[10] = (0x10);
    t[11] = (0x03);
    t[12] = (0x24);
    t[13] = (0x20);
    t[14] = (0x00);
    tft_write_byte(t, 15);

    tft_write_command(0x3A); /* Interface Pixel Format */
    t[0] = (0x55);
    tft_write_byte(t, 1);

    tft_write_command(0x11); /* Sleep OUT */
    msleep(120);
    tft_write_command(0x29); /* Display ON */

#endif
    /*soft reset*/
    tft_write_command(SOFTWARE_RESET);
    usleep(100000);

    /*exit sleep*/
    tft_write_command(SLEEP_OFF);
    usleep(100000);
    /*pixel format*/
    tft_write_command(PIXEL_FORMAT_SET);
    data = 0x55;
    tft_write_byte(&data, 1);
    lcd_set_direction(DIR_XY_LRUD);

    /*display on*/
    tft_write_command(DISPALY_ON);

    lcd_polling_enable();
}

void lcd_set_direction(lcd_dir_t dir)
{
    lcd_ctl.dir = dir;
    if (dir & DIR_XY_MASK)
    {
        lcd_ctl.width = LCD_Y_MAX - 1;
        lcd_ctl.height = LCD_X_MAX - 1;
    }
    else
    {
        lcd_ctl.width = LCD_X_MAX - 1;
        lcd_ctl.height = LCD_Y_MAX - 1;
    }

    tft_write_command(MEMORY_ACCESS_CTL);
    tft_write_byte((uint8_t *)&dir, 1);
}

void lcd_set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4] = {0};

    data[0] = (uint8_t)(x1 >> 8);
    data[1] = (uint8_t)(x1);
    data[2] = (uint8_t)(x2 >> 8);
    data[3] = (uint8_t)(x2);
    tft_write_command(HORIZONTAL_ADDRESS_SET);
    tft_write_byte(data, 4);

    data[0] = (uint8_t)(y1 >> 8);
    data[1] = (uint8_t)(y1);
    data[2] = (uint8_t)(y2 >> 8);
    data[3] = (uint8_t)(y2);
    tft_write_command(VERTICAL_ADDRESS_SET);
    tft_write_byte(data, 4);

    tft_write_command(MEMORY_WRITE);
}

void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_area(x, y, x, y);
    tft_write_half(&color, 1);
}

void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t color)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t data = 0;

    for (i = 0; i < 16; i++)
    {
        data = ascii0816[c * 16 + i];
        for (j = 0; j < 8; j++)
        {
            if (data & 0x80)
                lcd_draw_point(x + j, y, color);
            data <<= 1;
        }
        y++;
    }
}

void lcd_draw_string(uint16_t x, uint16_t y, char *str, uint16_t color)
{
    while (*str)
    {
        lcd_draw_char(x, y, *str, color);
        str++;
        x += 8;
    }
}

void lcd_ram_draw_string(char *str, uint32_t *ptr, uint16_t font_color, uint16_t bg_color)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t data = 0;
    uint8_t *pdata = NULL;
    uint16_t width = 0;
    uint32_t *pixel = NULL;

    width = 4 * strlen(str);
    while (*str)
    {
        pdata = (uint8_t *)&ascii0816[(*str) * 16];
        for (i = 0; i < 16; i++)
        {
            data = *pdata++;
            pixel = ptr + i * width;
            for (j = 0; j < 4; j++)
            {
                switch (data >> 6)
                {
                    case 0:
                        *pixel = ((uint32_t)bg_color << 16) | bg_color;
                        break;
                    case 1:
                        *pixel = ((uint32_t)bg_color << 16) | font_color;
                        break;
                    case 2:
                        *pixel = ((uint32_t)font_color << 16) | bg_color;
                        break;
                    case 3:
                        *pixel = ((uint32_t)font_color << 16) | font_color;
                        break;
                    default:
                        *pixel = 0;
                        break;
                }
                data <<= 2;
                pixel++;
            }
        }
        str++;
        ptr += 4;
    }
}

void lcd_clear(uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;

    lcd_set_area(0, 0, lcd_ctl.width, lcd_ctl.height);
    tft_fill_data(&data, LCD_X_MAX * LCD_Y_MAX / 2);
}

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color)
{
    uint32_t data_buf[640] = {0};
    uint32_t *p = data_buf;
    uint32_t data = color;
    uint32_t index = 0;

    data = (data << 16) | data;
    for (index = 0; index < 160 * width; index++)
        *p++ = data;

    lcd_set_area(x1, y1, x2, y1 + width - 1);
    tft_write_word(data_buf, ((x2 - x1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x1, y2 - width + 1, x2, y2);
    tft_write_word(data_buf, ((x2 - x1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x1, y1, x1 + width - 1, y2);
    tft_write_word(data_buf, ((y2 - y1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x2 - width + 1, y1, x2, y2);
    tft_write_word(data_buf, ((y2 - y1 + 1) * width + 1) / 2, 0);
}

void lcd_draw_picture(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint32_t *ptr)
{
    lcd_set_area(x1, y1, x1 + width - 1, y1 + height - 1);
    tft_write_word(ptr, width * height / 2, lcd_ctl.mode ? 2 : 0);
}
