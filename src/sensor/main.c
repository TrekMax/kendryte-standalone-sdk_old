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
#include "dmac.h"
#include "dvp.h"
#include "fpioa.h"
#include "iomem.h"
#include "plic.h"
#include "rgb2bmp.h"
#include "sleep.h"
#include "sysctl.h"
#include "syslog.h"
#include "uarths.h"
#include "unistd.h"

#include "ff.h"
#include "sdcard.h"

#include "gc0328.h"
#include "gc2145.h"
#include "lcd.h"
#include "ov2640.h"
#include "ov5640.h"

#include "board_config.h"


const int resolution[][2] = {
    {0, 0},
    // C/SIF Resolutions
    {88, 72},   /* QQCIF     */
    {176, 144}, /* QCIF      */
    {352, 288}, /* CIF       */
    {88, 60},   /* QQSIF     */
    {176, 120}, /* QSIF      */
    {352, 240}, /* SIF       */
    // VGA Resolutions
    {40, 30},   /* QQQQVGA   */
    {80, 60},   /* QQQVGA    */
    {160, 120}, /* QQVGA     */
    {320, 240}, /* QVGA      */
    {640, 480}, /* VGA       */
    {60, 40},   /* HQQQVGA   */
    {120, 80},  /* HQQVGA    */
    {240, 160}, /* HQVGA     */
    // FFT Resolutions
    {64, 32},   /* 64x32     */
    {64, 64},   /* 64x64     */
    {128, 64},  /* 128x64    */
    {128, 128}, /* 128x128    */
    {240, 240}, /* 240x240    */
    // Other
    {128, 160},   /* LCD       */
    {128, 160},   /* QQVGA2    */
    {720, 480},   /* WVGA      */
    {752, 480},   /* WVGA2     */
    {800, 600},   /* SVGA      */
    {1280, 1024}, /* SXGA      */
    {1600, 1200}, /* UXGA      */
};

static const char *TAG = "main";

static uint32_t *g_lcd_gram0;
static uint32_t *g_lcd_gram1;

volatile uint8_t g_dvp_finish_flag;
volatile uint8_t g_ram_mux;

static int on_irq_dvp(void *ctx)
{
    if(dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        /* switch gram */
        dvp_set_display_addr(g_ram_mux ? (uint32_t)g_lcd_gram0 : (uint32_t)g_lcd_gram1);

        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    } else
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
    fpioa_set_function(LCD_CS_PIN_NUM, FUNC_SPI0_SS0 + SPI_SLAVE_SELECT);
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

    /* DVP init */
    LOGI(TAG, "DVP init");
#if(CAMERA == CAMERA_OV5640)
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
#elif(CAMERA == CAMERA_OV2640)
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);

    dvp_set_image_size(320, 240);
    ov2640_init();
#elif(CAMERA == CAMERA_GC0328)
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);

    dvp_set_image_size(320, 240);
    gc0328_init();
#elif(CAMERA == CAMERA_GC2145)
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    // dvp_set_xclk_rate(20000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    // dvp_set_image_format(DVP_CFG_YUV_FORMAT);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);

    // dvp_set_image_size(160, 120);//QQVGA
    dvp_set_image_size(320, 240);//QVGA
    // dvp_set_image_size(640, 480);//VGA
    gc2145_init();
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
    msleep(500);
    char name[25];
    uint16_t count = 0;
    while(1)
    {
        /* ai cal finish*/
        while(g_dvp_finish_flag == 0)
            ;
        g_dvp_finish_flag = 0;
        /* display pic*/
        g_ram_mux ^= 0x01;
        lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? g_lcd_gram0 : g_lcd_gram1);
        // if(g_dvp_finish_flag)
        // {
        // sprintf(name, "0:photo_%03d.bmp", count++);
        // LOGI(TAG, "rgb565tobmp:%s",name);
        // rgb565tobmp((uint8_t *)(g_ram_mux ? g_lcd_gram0 : g_lcd_gram1), 320, 240, _T(name));
        //     // break;
        //     g_dvp_finish_flag = 0;
        // }

    }
    iomem_free(g_lcd_gram0);
    iomem_free(g_lcd_gram1);
}

extern const unsigned char gImage_image[] __attribute__((aligned(128)));

static uint16_t lcd_gram[320 * 240] __attribute__((aligned(32)));
void rgb888_to_lcd(uint8_t *src, uint16_t *dest, size_t width, size_t height)
{
    size_t chn_size = width * height;
    for(size_t i = 0; i < width * height; i++)
    {
        uint8_t r = src[i];
        uint8_t g = src[chn_size + i];
        uint8_t b = src[chn_size * 2 + i];

        uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
        size_t d_i = i % 2 ? (i - 1) : (i + 1);
        dest[d_i] = rgb;
    }
}

static int sdcard_test(void);
static int fs_test(void);
FRESULT sd_write_test(TCHAR *path);

void sdcard_io_mux_init(void)
{
    fpioa_set_function(SD_SPI_SCLK, FUNC_SPI1_SCLK);
    fpioa_set_function(SD_SPI_MOSI, FUNC_SPI1_D0);
    fpioa_set_function(SD_SPI_MISO, FUNC_SPI1_D1);
    fpioa_set_function(SD_SPI_CS_PIN, FUNC_GPIOHS0 + SD_SPI_CS_PIN);
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

#if(BOARD == BOARD_MAIX_DUINO)
    lcd_set_direction(DIR_YX_RLDU); /* left up 0,0 */
#else
    lcd_set_direction(DIR_YX_RLUD); /* left up 0,0 */
#endif

    lcd_clear(BLACK);

    g_lcd_gram0 = (uint32_t *)iomem_malloc(320 * 240 * 2);
    g_lcd_gram1 = (uint32_t *)iomem_malloc(320 * 240 * 2);
    sdcard_io_mux_init();
    dmac_init();
    // /* SD card init */
    // if(sd_init())
    // {
    //     printf("Fail to init SD card\n");
    //     return -1;
    // }

    // LOGI(TAG, "SD card TEST");
    // if(sdcard_test())
    // {
    //     LOGE(TAG, "SD card err");
    //     return -1;
    // }
    // if(fs_test())
    // {
    //     LOGI(TAG, "FAT32 err");
    //     return -1;
    // }
    // if(sd_write_test(_T("0:test.txt")))
    // {
    //     LOGE(TAG, "SD write err");
    //     return -1;
    // }
    // LCD Display Test
    // rgb888_to_lcd(gImage_image, lcd_gram, 320, 240);
    // lcd_draw_picture(0, 0, 320, 240, lcd_gram);
    // msleep(1000);
    dvp_sensor_init();

    printk(LOG_COLOR_W "-------------END---------------\r\n");
    while(1)
        ;

    return 0;
}

static int sdcard_test(void)
{
    uint8_t status;

    LOGI(TAG, "/******************sdcard test*****************/");
    status = sd_init();
    LOGI(TAG, "sd init %d", status);
    if(status != 0)
    {
        return status;
    }

    LOGI(TAG, "card info status %d", status);
    LOGI(TAG, "CardCapacity:%ld", cardinfo.CardCapacity);
    LOGI(TAG, "CardBlockSize:%d", cardinfo.CardBlockSize);
    return 0;
}

static int fs_test(void)
{
    static FATFS sdcard_fs;
    FRESULT status;
    DIR dj;
    FILINFO fno;

    LOGI(TAG, "/********************fs test*******************/");
    status = f_mount(&sdcard_fs, _T("0:"), 1);
    LOGI(TAG, "mount sdcard:%d", status);
    if(status != FR_OK)
        return status;

    LOGI(TAG, "printf filename");
    status = f_findfirst(&dj, &fno, _T("0:"), _T("*"));
    while(status == FR_OK && fno.fname[0])
    {
        if(fno.fattrib & AM_DIR)
            LOGI(TAG, "dir:%s", fno.fname);
        else
            LOGI(TAG, "file:%s", fno.fname);
        status = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);
    return 0;
}

FRESULT sd_write_test(TCHAR *path)
{
    FIL file;
    FRESULT ret = FR_OK;
    LOGI(TAG, "/*******************sd write test*******************/");
    uint32_t v_ret_len = 0;

    FILINFO v_fileinfo;
    if((ret = f_stat(path, &v_fileinfo)) == FR_OK)
    {
        LOGI(TAG, "%s length is %lld", path, v_fileinfo.fsize);
    } else
    {
        LOGI(TAG, "%s fstat err [%d]", path, ret);
    }

    if((ret = f_open(&file, path, FA_READ)) == FR_OK)
    {
        char v_buf[64] = {0};
        ret = f_read(&file, (void *)v_buf, 64, &v_ret_len);
        if(ret != FR_OK)
        {
            LOGI(TAG, "Read %s err[%d]", path, ret);
        } else
        {
            LOGI(TAG, "Read :> %s %d bytes lenth", v_buf, v_ret_len);
        }
        f_close(&file);
    }

    if((ret = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
    {
        LOGI(TAG, "open file %s err[%d]", path, ret);
        return ret;
    } else
    {
        LOGI(TAG, "Open %s ok", path);
    }
    uint8_t hello[1024];
    uint32_t i;
    for(i = 0; i < 1024; i++)
    {
        hello[i] = 'A';
    }
    ret = f_write(&file, hello, sizeof(hello), &v_ret_len);
    if(ret != FR_OK)
    {
        LOGI(TAG, "Write %s err[%d]", path, ret);
    } else
    {
        LOGI(TAG, "Write %d bytes to %s ok", v_ret_len, path);
    }
    f_close(&file);
    return ret;
}
