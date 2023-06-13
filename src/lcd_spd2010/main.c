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
#include <bsp.h>
#include <stdio.h>
#include <sysctl.h>
#include "board_config.h"
#include "syslog.h"
#include "pic.h"
#include "pic2.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "uarths.h"
#include "ips_lcd.h"

static const char *TAG = "main";

int main(void)
{
    uint32_t freq = 0;
    freq = sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uint64_t core = current_coreid();
    printk(LOG_COLOR_W "-------------------------------\r\n");
    printk(LOG_COLOR_W "Sipeed@QinYUN575\r\n");
    printk(LOG_COLOR_W "Compile@ %s %s\r\n", __DATE__, __TIME__);
    printk(LOG_COLOR_W "Board: " LOG_COLOR_E BOARD_NAME "\r\n");
    printk(LOG_COLOR_W "pll freq: %dhz\r\n", freq);
    printk(LOG_COLOR_W "-------------------------------\r\n");

    ips_lcd_init();
    ips_lcd_color_bar_test();
    // LCD_ShowPicture(10, 20, 40, 40, gImage_1);
    LCD_ShowPicture(0, 0, LCD_W, LCD_H, gImage_nanke);
    LCD_DrawRectangle(0, 0, LCD_W-1, LCD_H-1, RED);


    printk(LOG_COLOR_W "-------------END---------------\r\n");
    return 0;
}
