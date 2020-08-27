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
#include "gpiohs.h"
#include "sysctl.h"

#include <pwm.h>
#include <timer.h>

#include "board_config.h"

#include "board_config.h"
#include "image.h"
#include "lcd.h"
#include "nt35310.h"
#include "sleep.h"
#include "syslog.h"

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
    fpioa_set_function(LCD_CS_PIN_NUM, FUNC_SPI0_SS0 + SPI_SLAVE_SELECT);
    fpioa_set_function(LCD_CLK_PIN_NUM, FUNC_SPI0_SCLK);
    sysctl_set_spi0_dvp_data(1);

#if LCD_BL_CONTROL
    gpiohs_set_drive_mode(LCD_BL_GPIONUM, GPIO_DM_OUTPUT);
    gpio_pin_value_t value = GPIO_PV_LOW;
    gpiohs_set_pin(LCD_BL_GPIONUM, value);
#endif
}

#if LCD_BL_CONTROL

#define TIMER_NOR 0
#define TIMER_CHN 0
#define TIMER_PWM 1
#define TIMER_PWM_CHN 0

int timer_callback(void *ctx)
{
    static double cnt = 0.01;
    static int flag = 0;

    pwm_set_frequency(TIMER_PWM, TIMER_PWM_CHN, 200 * 1000, cnt);

    flag ? (cnt -= 0.01) : (cnt += 0.01);
    if(cnt > 1.0)
    {
        cnt = 1.0;
        flag = 1;
    } else if(cnt < 0.2)
    {
        cnt = 0.2;
        flag = 0;
    }
    return 0;
}

void lcd_bl_control()
{
    /* Init FPIOA pin mapping for PWM*/
    fpioa_set_function(LCD_BL_GPIONUM, FUNC_TIMER1_TOGGLE1);
    /* Init Platform-Level Interrupt Controller(PLIC) */
    plic_init();
    /* Enable global interrupt for machine mode of RISC-V */
    sysctl_enable_irq();
    /* Init timer */
    timer_init(TIMER_NOR);
    /* Set timer interval to 100ms (1e8ns) */
    timer_set_interval(TIMER_NOR, TIMER_CHN, 1000 * 1000 * 100);
    /* Set timer callback function with repeat method */
    timer_irq_register(TIMER_NOR, TIMER_CHN, 0, 1, timer_callback, NULL);
    /* Enable timer */
    timer_set_enable(TIMER_NOR, TIMER_CHN, 1);
    /* Init PWM */
    pwm_init(TIMER_PWM);
    /* Set PWM to 200000Hz */
    pwm_set_frequency(TIMER_PWM, TIMER_PWM_CHN, 200000, 0.5);
    /* Enable PWM */
    pwm_set_enable(TIMER_PWM, TIMER_PWM_CHN, 1);
}

#endif
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

#if(BOARD == BOARD_MAIX_DUINO)
    lcd_set_direction(DIR_YX_RLDU); /* left up 0,0 */
#elif(BOARD == BOARD_MAIX_CUBE)
    lcd_set_direction(DIR_XY_RLUD);
    lcd_set_direction(DIR_YX_RLDU);
    tft_write_command(INVERSION_DISPALY_ON);
#elif(BOARD == BOARD_MAIX_NEW_GO)
    lcd_set_direction(DIR_YX_RLUD | 0x08); /* left up 0,0 */
#else
    lcd_set_direction(DIR_YX_RLUD | 0x08); /* left up 0,0 */
#endif
#if LCD_BL_CONTROL
    lcd_bl_control();
#endif

    lcd_clear(BLACK);
    lcd_draw_picture(0, 0, 320, 240, rgb_image);
    // lcd_set_direction(DIR_XY_RLUD);

    lcd_draw_string(0, 0, "Hello World", RED);
    lcd_draw_string(0, 20, "Hello World", GREEN);
    lcd_draw_string(0, 40, "Hello World", BLUE);
    printk(LOG_COLOR_W "-------------END---------------\r\n");
    while(1)
        ;
}
