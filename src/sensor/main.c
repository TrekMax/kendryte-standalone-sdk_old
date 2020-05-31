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
#include <string.h>

#include "unistd.h"
#include "dvp.h"
#include "fpioa.h"
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "iomem.h"

#include "syslog.h"

#include "lcd.h"
#include "ov5640.h"
#include "ov2640.h"

#include "board_config.h"

static const char *TAG = "main";

static uint32_t *g_lcd_gram0;
static uint32_t *g_lcd_gram1;

volatile uint8_t g_dvp_finish_flag;
volatile uint8_t g_ram_mux;

static int on_irq_dvp(void* ctx)
{
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        /* switch gram */
        dvp_set_display_addr(g_ram_mux ? (uint32_t)g_lcd_gram0 : (uint32_t)g_lcd_gram1);

        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else
    {
        if(g_dvp_finish_flag == 0)
            dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }

    return 0;
}

static void io_mux_init(void)
{
    /* Init DVP IO map and function settings */
    fpioa_set_function(42, FUNC_CMOS_RST);
    fpioa_set_function(44, FUNC_CMOS_PWDN);
    fpioa_set_function(46, FUNC_CMOS_XCLK);
    fpioa_set_function(43, FUNC_CMOS_VSYNC);
    fpioa_set_function(45, FUNC_CMOS_HREF);
    fpioa_set_function(47, FUNC_CMOS_PCLK);
    fpioa_set_function(41, FUNC_SCCB_SCLK);
    fpioa_set_function(40, FUNC_SCCB_SDA);
}

static void lcd_io_mux_init(void)
{
    fpioa_set_function(LCD_RST_PIN_NUM, FUNC_GPIOHS0 + RST_GPIO_HS_NUM);
    fpioa_set_function(LCD_DCX_PIN_NUM, FUNC_GPIOHS0 + DCX_GPIO_HS_NUM);
    fpioa_set_function(LCD_CS_PIN_NUM,  FUNC_SPI0_SS0 + SPI_SLAVE_SELECT);
    fpioa_set_function(LCD_CLK_PIN_NUM, FUNC_SPI0_SCLK);
    sysctl_set_spi0_dvp_data(1);
}

static void io_set_power(void)
{
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

void dvp_sensor_init(void)
{

    g_lcd_gram0 = (uint32_t *)iomem_malloc(320*240*2);
    g_lcd_gram1 = (uint32_t *)iomem_malloc(320*240*2);
    /* DVP init */
    LOGI(TAG, "DVP init");
#if (CAMERA == CAMERA_OV5640)
    dvp_init(16);
    dvp_set_xclk_rate(50000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);
    ov5640_init();

#if 0
    // OV5640_Focus_Init();
    OV5640_Light_Mode(2);      //set auto
    OV5640_Color_Saturation(6); //default
    OV5640_Brightness(8);   //default
    OV5640_Contrast(3);     //default
    // OV5640_Sharpness(33);   //set auto
    // OV5640_Auto_Focus();
#endif
    #elif (CAMERA == CAMERA_OV2640)
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);

    dvp_set_image_size(320, 240);
    ov2640_init();

    #else
        LOGE(TAG, "Undefined camera model!");
        return;
    #endif

    dvp_set_ai_addr((uint32_t)0x40600000, (uint32_t)0x40612C00, (uint32_t)0x40625800);
    dvp_set_display_addr((uint32_t)g_lcd_gram0);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();

    /* DVP interrupt config */
    LOGI(TAG, "DVP interrupt config");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_irq_dvp, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* enable global interrupt */
    sysctl_enable_irq();

    /* system start */
    LOGI(TAG, "system start");
    g_ram_mux = 0;
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    while (1)
    {
        /* ai cal finish*/
        while (g_dvp_finish_flag == 0)
            ;
        g_dvp_finish_flag = 0;
        /* display pic*/
        g_ram_mux ^= 0x01;
        lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? g_lcd_gram0 : g_lcd_gram1);
    }
    iomem_free(g_lcd_gram0);
    iomem_free(g_lcd_gram1);
}

int main(void)
{
    uint32_t freq_ppl0 = 0;
    /* Set CPU and dvp clk */
    freq_ppl0 = sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uarths_init();

    printk(LOG_COLOR_W "-------------------------------\r\n");
    printk(LOG_COLOR_W "Sipeed@QinYUN575\r\n");
    printk(LOG_COLOR_W "Compile@ %s %s\r\n", __DATE__, __TIME__);
    printk(LOG_COLOR_W "Board: " LOG_COLOR_E BOARD_NAME "\r\n");
    printk(LOG_COLOR_W "PLL0 Freq: %dHz\r\n", freq_ppl0);
    printk(LOG_COLOR_W "-------------------------------\r\n");

    io_mux_init();
    lcd_io_mux_init();
    io_set_power();
    plic_init();

    /* LCD init */
    LOGI(TAG, "LCD init");
    lcd_init();
    
    #if (BOARD == BOARD_MAIX_DUINO)
    lcd_set_direction(DIR_YX_RLDU);     /* left up 0,0 */
#else
    lcd_set_direction(DIR_YX_RLUD);     /* left up 0,0 */
#endif

    lcd_clear(BLACK);

    dvp_sensor_init();

    printk(LOG_COLOR_W "-------------END---------------\r\n");
    while (1);

    return 0;
}
