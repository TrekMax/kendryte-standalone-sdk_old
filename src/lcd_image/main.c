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
#include <stdio.h>
#include "unistd.h"

#include "fpioa.h"
#include "sysctl.h"

#include "board_config.h"

#include "nt35310.h"
#include "image.h"
#include "lcd.h"
#include "syslog.h"
#include "sleep.h"
#include "board_config.h"

static const char *TAG = "main";


static void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

static void io_mux_init(void)
{
    fpioa_set_function(LCD_RST_PIN_NUM, FUNC_GPIOHS0 + RST_GPIO_HS_NUM);
    fpioa_set_function(LCD_DCX_PIN_NUM, FUNC_GPIOHS0 + DCX_GPIO_HS_NUM);
    fpioa_set_function(LCD_CS_PIN_NUM,  FUNC_SPI0_SS0 + SPI_SLAVE_SELECT);
    fpioa_set_function(LCD_CLK_PIN_NUM, FUNC_SPI0_SCLK);
    sysctl_set_spi0_dvp_data(1);
}


int main(void)
{
    uint32_t freq = 0;
    freq = sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    
    printk(LOG_COLOR_W "-------------------------------\r\n");
    printk(LOG_COLOR_W "Sipeed@QinYUN575\r\n");
    printk(LOG_COLOR_W "Compile@ %s %s\r\n", __DATE__, __TIME__);
    printk(LOG_COLOR_W "Board: " LOG_COLOR_E BOARD_NAME "\r\n");
    printk(LOG_COLOR_W "pll freq: %dhz\r\n", freq);
    printk(LOG_COLOR_W "-------------------------------\r\n");

    io_mux_init();
    io_set_power();
    lcd_init();

#if (BOARD == BOARD_MAIX_DUINO)
    lcd_set_direction(DIR_YX_RLDU);     /* left up 0,0 */
#else
    lcd_set_direction(DIR_YX_RLUD);     /* left up 0,0 */
#endif

    lcd_clear(BLACK);

    lcd_draw_string(0, 0, "Hello World", RED);
    lcd_draw_string(0, 20, "Hello World", GREEN);
    lcd_draw_string(0, 40, "Hello World", BLUE);
    printk(LOG_COLOR_W "-------------END---------------\r\n");
    while (1);
}
