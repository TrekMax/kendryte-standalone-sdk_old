#include "config.h"
#include "fpioa.h"
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "utils.h"
#include "kpu.h"
#include "w25qxx.h"
#include "lpbox.h"
#include "nt35310.h"
#include "lcd.h"
#include "syslog.h"
#include "board_config.h"
#include "dvp.h"
#include "iomem.h"

#include "ov5640.h"
#include "ov2640.h"
#include "gc0328.h"

static char *TAG = "main";

const size_t layer0_w = 19, layer0_h = 14;
const size_t layer1_w = 9 , layer1_h = 6;

const size_t rf_stride0 = 16 , rf_stride1 = 32;
const size_t rf_start0  = 15 , rf_start1  = 31;
const size_t rf_size0   = 128, rf_size1   = 256;

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

extern const unsigned char gImage_image[] __attribute__((aligned(128)));

static uint16_t lcd_gram[320 * 240] __attribute__((aligned(32)));

#define LPBOX_KMODEL_SIZE (394280)
uint8_t* lpbox_model_data;

kpu_model_context_t task;

uint8_t lpimage[24 * 94 * 3] __attribute__((aligned(32)));

volatile uint8_t g_ai_done_flag;

static void ai_done(void* arg)
{
    g_ai_done_flag = 1;
}


static uint32_t *g_lcd_gram0;
static uint32_t *g_lcd_gram1;
uint8_t *g_ai_buf;

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

void rgb888_to_lcd(uint8_t* src, uint16_t* dest, size_t width, size_t height)
{
    size_t chn_size = width * height;
    for (size_t i = 0; i < width * height; i++) {
        uint8_t r = src[i];
        uint8_t g = src[chn_size + i];
        uint8_t b = src[chn_size * 2 + i];

        uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
        size_t d_i = i % 2 ? (i - 1) : (i + 1);
        dest[d_i] = rgb;
    }
}


void dvp_sensor_init(void)
{

    g_lcd_gram0 = (uint32_t *)iomem_malloc(320*240*2);
    g_lcd_gram1 = (uint32_t *)iomem_malloc(320*240*2);
    g_ai_buf = (uint8_t *)iomem_malloc(320 * 240 * 3);
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
    #elif (CAMERA == CAMERA_GC0328)
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);

    dvp_set_image_size(320, 240);
    gc0328_init();
    #else
        LOGE(TAG, "Undefined camera model!");
        return;
    #endif

    dvp_set_ai_addr((uint32_t)g_ai_buf, (uint32_t)(g_ai_buf + 320 * 240), (uint32_t)(g_ai_buf + 320 * 240 * 2));
    // dvp_set_ai_addr((uint32_t)0x40600000, (uint32_t)0x40612C00, (uint32_t)0x40625800);
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


}


int main()
{
    // 配置时钟
    uint32_t freq_ppl0 = sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
    uint32_t freq_ppl1 = sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    APP_LOGI("PLL0 = %d PLL1 = %d", freq_ppl0, freq_ppl1);
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

    // LCD init
    APP_LOGI("LCD init");
    lcd_init();
    lcd_set_direction(DIR_YX_RLDU);
    APP_LOGI("LCD end");

    // 加载模型
    APP_LOGI("flash init");
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();
    lpbox_model_data = (uint8_t*)malloc(LPBOX_KMODEL_SIZE);
    w25qxx_read_data(0xA00000, lpbox_model_data, LPBOX_KMODEL_SIZE, W25QXX_QUAD_FAST);
    APP_LOGI("flash end")

    sysctl_enable_irq();

    // 加载模型
    APP_LOGI("model init start");
    if (kpu_load_kmodel(&task, lpbox_model_data) != 0) {
        APP_LOGE("model init error");
        while (1)
            ;
    }
    APP_LOGI("model init OK");

/*********************************************************************/
    APP_LOGI("lpbox init");
    lpbox_t lpbox;
    if (lpbox_new(&lpbox, 1) != 0) {
        APP_LOGE("lpbox new error");
        while (1)
            ;
    }
    (lpbox.kpu_output)[0].w         = layer0_w;
    (lpbox.kpu_output)[0].h         = layer0_h;
    (lpbox.kpu_output)[0].rf_size   = rf_size0;
    (lpbox.kpu_output)[0].rf_start  = rf_start0;
    (lpbox.kpu_output)[0].rf_stride = rf_stride0;

    APP_LOGI("lpbox init end");
/********************************************************************/
    dvp_sensor_init();
/********************************************************************/
    while (1)
    {
        /* ai cal finish*/
        while (g_dvp_finish_flag == 0)
            ;
        g_dvp_finish_flag = 0;
        /* display pic*/
        g_ram_mux ^= 0x01;

        // 运行模型
        g_ai_done_flag = 0;
        kpu_run_kmodel(&task, gImage_image, DMAC_CHANNEL5, ai_done, NULL);
        // kpu_run_kmodel(&task, g_ai_buf, DMAC_CHANNEL5, ai_done, NULL);
        APP_LOGI("model run start");
        while (!g_ai_done_flag)
            ;
        APP_LOGI("model run OK");

        // 提取模型推理结果
        get_lpbox_kpu_output(&task, &lpbox);

        // 提取预测框
        
        APP_LOGI("LPbox run start");
        get_lpbox(&lpbox, 0.8, 0.2);
        APP_LOGI("LPbox run OK");

        APP_LOGI("bbox num：%d", lpbox.bboxes->num);

        // 显示图片
        rgb888_to_lcd(gImage_image, lcd_gram, 320, 240);
        lcd_draw_picture(0, 0, 320, 240, lcd_gram);
        
        // rgb888_to_lcd(g_ai_buf, lcd_gram, 320, 240);
        // lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? g_lcd_gram0 : g_lcd_gram1);
        // lcd_draw_picture(0, 0, 320, 240, g_ai_buf);
        
        for (bbox_t* bbox = lpbox.bboxes->box; bbox != NULL; bbox = bbox->next) {
            APP_LOGI("x1: %f, y1: %f, x2: %f, y2: %f, score: %f", bbox->x1, bbox->y1, bbox->x2, bbox->y2, bbox->score);
            lcd_draw_rectangle(bbox->x1, bbox->y1, bbox->x2, bbox->y2, 2, GREEN);
        }
    }
    iomem_free(g_lcd_gram0);
    iomem_free(g_lcd_gram1);
    

    APP_LOGW("end");

    while (1)
        ;
}