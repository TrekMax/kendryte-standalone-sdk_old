#include "board_config.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "ips_lcd.h"
#include "sleep.h"
#include "spi.h"

#define DATALENGTH 8

static const char *TAG = "lcd";

void ips_lcd_io_mux_init(void)
{
    fpioa_set_function(SPI_IPS_LCD_CS_PIN_NUM, FUNC_SPI1_SS0);   // CS
    fpioa_set_function(SPI_IPS_LCD_SCK_PIN_NUM, FUNC_SPI1_SCLK); // SCLK
    fpioa_set_function(SPI_IPS_LCD_MOSI_PIN_NUM, FUNC_SPI1_D0);  // MOSI
    // fpioa_set_function(SPI_IPS_LCD_MISO_PIN_NUM, FUNC_SPI1_D1);  // MISO

    fpioa_set_function(SPI_IPS_LCD_DC_PIN_NUM, FUNC_GPIOHS0 + SPI_IPS_LCD_DC_GPIO_NUM);   // D2
    fpioa_set_function(SPI_IPS_LCD_RST_PIN_NUM, FUNC_GPIOHS0 + SPI_IPS_LCD_RST_GPIO_NUM); // D3
    fpioa_set_function(SPI_IPS_LCD_BL_PIN_NUM, FUNC_GPIOHS0 + SPI_IPS_LCD_BL_GPIO_NUM);   // D2

    gpiohs_set_drive_mode(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_DM_OUTPUT);
    gpiohs_set_drive_mode(SPI_IPS_LCD_BL_GPIO_NUM, GPIO_DM_OUTPUT);

    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_HIGH);
    gpiohs_set_pin(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_PV_HIGH);
    gpiohs_set_pin(SPI_IPS_LCD_BL_GPIO_NUM, GPIO_PV_HIGH);
}

static void spi_write_reg(uint8_t reg_addr, uint8_t data)
{
    uint8_t cmd[2] = {reg_addr, data};

    spi_send_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, cmd, 2, NULL, 0);
}

static void spi_read_reg(uint8_t reg_addr, uint8_t *reg_data)
{
    uint8_t cmd[1] = {reg_addr};

    spi_receive_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, cmd, 1, reg_data, 1);
}

static void ips_lcd_write_command(uint8_t command)
{
    uint8_t cmd[1] = {command};
    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_LOW);

    spi_send_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, cmd, 1, NULL, 0);

    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_HIGH);
}

static void ips_lcd_write_data(uint8_t data)
{
    uint8_t cmd[1] = {data};
    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_HIGH);

    spi_send_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, cmd, 1, NULL, 0);
}

/******************************************************************************
      函数说明：LCD写入数据
      入口数据：dat 写入的数据
      返回值：  无
******************************************************************************/
void LCD_WR_DATA(uint16_t dat)
{
    ips_lcd_write_data(dat >> 8);
    ips_lcd_write_data(dat & 0xFF);
}

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    if(USE_HORIZONTAL == 0)
    {
        ips_lcd_write_command(0x2a); //列地址设置
        LCD_WR_DATA(x1 + 52);
        LCD_WR_DATA(x2 + 52);
        ips_lcd_write_command(0x2b); //行地址设置
        LCD_WR_DATA(y1 + 40);
        LCD_WR_DATA(y2 + 40);
        ips_lcd_write_command(0x2c); //储存器写
    } else if(USE_HORIZONTAL == 1)
    {
        ips_lcd_write_command(0x2a); //列地址设置
        LCD_WR_DATA(x1 + 53);
        LCD_WR_DATA(x2 + 53);
        ips_lcd_write_command(0x2b); //行地址设置
        LCD_WR_DATA(y1 + 40);
        LCD_WR_DATA(y2 + 40);
        ips_lcd_write_command(0x2c); //储存器写
    } else if(USE_HORIZONTAL == 2)
    {
        ips_lcd_write_command(0x2a); //列地址设置
        LCD_WR_DATA(x1 + 40);
        LCD_WR_DATA(x2 + 40);
        ips_lcd_write_command(0x2b); //行地址设置
        LCD_WR_DATA(y1 + 53);
        LCD_WR_DATA(y2 + 53);
        ips_lcd_write_command(0x2c); //储存器写
    } else
    {
        ips_lcd_write_command(0x2a); //列地址设置
        LCD_WR_DATA(x1 + 40);
        LCD_WR_DATA(x2 + 40);
        ips_lcd_write_command(0x2b); //行地址设置
        LCD_WR_DATA(y1 + 52);
        LCD_WR_DATA(y2 + 52);
        ips_lcd_write_command(0x2c); //储存器写
    }
}
/******************************************************************************
      函数说明：在指定位置画点
      入口数据：x,y 画点坐标
                color 点的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_Address_Set(x, y, x, y); //设置光标位置
    LCD_WR_DATA(color);
}

/******************************************************************************
      函数说明：画线
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   线的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1; //计算坐标增量
    delta_y = y2 - y1;
    uRow = x1; //画线起点坐标
    uCol = y1;
    if(delta_x > 0)
        incx = 1; //设置单步方向
    else if(delta_x == 0)
        incx = 0; //垂直线
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }
    if(delta_y > 0)
        incy = 1;
    else if(delta_y == 0)
        incy = 0; //水平线
    else
    {
        incy = -1;
        delta_y = -delta_x;
    }
    if(delta_x > delta_y)
        distance = delta_x; //选取基本增量坐标轴
    else
        distance = delta_y;
    for(t = 0; t < distance + 1; t++)
    {
        LCD_DrawPoint(uRow, uCol, color); //画点
        xerr += delta_x;
        yerr += delta_y;
        if(xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if(yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

/******************************************************************************
      函数说明：画矩形
      入口数据：x1,y1   起始坐标
                x2,y2   终止坐标
                color   矩形的颜色
      返回值：  无
******************************************************************************/
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[])
{
    uint16_t num;
    num = length * width * 2;
    LCD_Address_Set(x, y, x + length - 1, y + width - 1);
    spi_send_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, NULL, 0, pic, num);
}

/******************************************************************************
      函数说明：在指定区域填充颜色
      入口数据：xsta,ysta   起始坐标
                xend,yend   终止坐标
                color       要填充的颜色
      返回值：  无
******************************************************************************/
void LCD_Fill(uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint16_t color)
{
    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_HIGH);
    uint16_t num;
    num = length * width * 2;
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    LCD_Address_Set(x, y, x + length - 1, y + width - 1);

    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, 32, 0);
    spi_init_non_standard(SPI_INDEX, 0 /*instrction length*/, 32 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_fill_data_dma(DMAC_CHANNEL0, SPI_INDEX, SPI_CHIP_SELECT_NSS, &data, num / 2);
    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
}
/******************************************************/

typedef struct {
	uint8_t cmd;
	uint8_t data_len;
	uint8_t data[4];
} spd2010_init_cmd_item_t;

const static spd2010_init_cmd_item_t items[] = {
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20 ,0x10 ,0x10},},
	{.cmd = 0x62, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x61, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x5C, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x58, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x55, .data_len = 1,  .data = {0x55},},
	{.cmd = 0x54, .data_len = 1,  .data = {0x44},},
	{.cmd = 0x51, .data_len = 1,  .data = {0x11},},
	{.cmd = 0x4B, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x4A, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x49, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x47, .data_len = 1,  .data = {0x77},},
	{.cmd = 0x46, .data_len = 1,  .data = {0x66},},
	{.cmd = 0x45, .data_len = 1,  .data = {0x55},},
	{.cmd = 0x44, .data_len = 1,  .data = {0x44},},
	{.cmd = 0x43, .data_len = 1,  .data = {0x33},},
	{.cmd = 0x42, .data_len = 1,  .data = {0x22},},
	{.cmd = 0x41, .data_len = 1,  .data = {0x11},},
	{.cmd = 0x37, .data_len = 1,  .data = {0x12},},
	{.cmd = 0x36, .data_len = 1,  .data = {0x12},},
	{.cmd = 0x35, .data_len = 1,  .data = {0x11},},
	{.cmd = 0x34, .data_len = 1,  .data = {0x11},},
	{.cmd = 0x33, .data_len = 1,  .data = {0x20},},
	{.cmd = 0x32, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x31, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x30, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x27, .data_len = 1,  .data = {0x12},},
	{.cmd = 0x26, .data_len = 1,  .data = {0x12},},
	{.cmd = 0x25, .data_len = 1,  .data = {0x11},},
	{.cmd = 0x24, .data_len = 1,  .data = {0x11},},
	{.cmd = 0x23, .data_len = 1,  .data = {0x20},},
	{.cmd = 0x22, .data_len = 1,  .data = {0x72},},
	{.cmd = 0x21, .data_len = 1,  .data = {0x82},},
	{.cmd = 0x20, .data_len = 1,  .data = {0x81},},
	{.cmd = 0x1B, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x1A, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x16, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x15, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x11, .data_len = 1,  .data = {0x12},},
	{.cmd = 0x10, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x0C, .data_len = 1,  .data = {0x12},},
	{.cmd = 0x0B, .data_len = 1,  .data = {0x43},},
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20, 0x10, 0x11},},
	{.cmd = 0x6A, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x69, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x68, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x67, .data_len = 1,  .data = {0x04},},
	{.cmd = 0x66, .data_len = 1,  .data = {0x38},},
	{.cmd = 0x65, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x64, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x63, .data_len = 1,  .data = {0x04},},
	{.cmd = 0x62, .data_len = 1,  .data = {0x38},},
	{.cmd = 0x61, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x60, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x55, .data_len = 1,  .data = {0x06},},
	{.cmd = 0x50, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x30, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x1E, .data_len = 1,  .data = {0x88},},
	{.cmd = 0x1D, .data_len = 1,  .data = {0x88},},
	{.cmd = 0x1C, .data_len = 1,  .data = {0x88},},
	{.cmd = 0x16, .data_len = 1,  .data = {0x99},},
	{.cmd = 0x15, .data_len = 1,  .data = {0x99},},
	{.cmd = 0x14, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x13, .data_len = 1,  .data = {0xf0},},
	{.cmd = 0x0c, .data_len = 1,  .data = {0xde},},
	{.cmd = 0x0B, .data_len = 1,  .data = {0xde},},
	{.cmd = 0x0A, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x09, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x08, .data_len = 1,  .data = {0x60},},
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20, 0x10, 0x12},},
	{.cmd = 0x2E, .data_len = 1,  .data = {0x1e},},
	{.cmd = 0x2D, .data_len = 1,  .data = {0x28},},
	{.cmd = 0x2C, .data_len = 1,  .data = {0x26},},
	{.cmd = 0x2B, .data_len = 1,  .data = {0x1e},},
	{.cmd = 0x2A, .data_len = 1,  .data = {0x28},},
	{.cmd = 0x21, .data_len = 1,  .data = {0x52},},   //VCOM
	{.cmd = 0x1F, .data_len = 1,  .data = {0xDC},},
	{.cmd = 0x15, .data_len = 1,  .data = {0x0F},},
	{.cmd = 0x12, .data_len = 1,  .data = {0x89},},
	{.cmd = 0x10, .data_len = 1,  .data = {0x0F},},
	{.cmd = 0x0D, .data_len = 1,  .data = {0x66},},
	{.cmd = 0x06, .data_len = 1,  .data = {0x06},},
	{.cmd = 0x00, .data_len = 1,  .data = {0x99},},
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20, 0x10, 0x15},},
	{.cmd = 0x2F, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x2E, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x2D, .data_len = 1,  .data = {0x35},},
	{.cmd = 0x2C, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x2B, .data_len = 1,  .data = {0x32},},
	{.cmd = 0x2A, .data_len = 1,  .data = {0x33},},
	{.cmd = 0x29, .data_len = 1,  .data = {0x19},},
	{.cmd = 0x28, .data_len = 1,  .data = {0x17},},
	{.cmd = 0x27, .data_len = 1,  .data = {0x1D},},
	{.cmd = 0x26, .data_len = 1,  .data = {0x1B},},
	{.cmd = 0x25, .data_len = 1,  .data = {0x09},},
	{.cmd = 0x24, .data_len = 1,  .data = {0x05},},
	{.cmd = 0x23, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x22, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x21, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x20, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x0F, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x0E, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x0D, .data_len = 1,  .data = {0x35},},
	{.cmd = 0x0C, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x0B, .data_len = 1,  .data = {0x32},},
	{.cmd = 0x0A, .data_len = 1,  .data = {0x33},},
	{.cmd = 0x09, .data_len = 1,  .data = {0x16},},
	{.cmd = 0x08, .data_len = 1,  .data = {0x18},},
	{.cmd = 0x07, .data_len = 1,  .data = {0x1A},},
	{.cmd = 0x06, .data_len = 1,  .data = {0x1C},},
	{.cmd = 0x05, .data_len = 1,  .data = {0x04},},
	{.cmd = 0x04, .data_len = 1,  .data = {0x08},},
	{.cmd = 0x03, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x02, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x01, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x00, .data_len = 1,  .data = {0x00},},
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20, 0x10, 0x16},},
	{.cmd = 0x2F, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x2E, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x2D, .data_len = 1,  .data = {0x35},},
	{.cmd = 0x2C, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x2B, .data_len = 1,  .data = {0x32},},
	{.cmd = 0x2A, .data_len = 1,  .data = {0x33},},
	{.cmd = 0x29, .data_len = 1,  .data = {0x18},},
	{.cmd = 0x28, .data_len = 1,  .data = {0x16},},
	{.cmd = 0x27, .data_len = 1,  .data = {0x1C},},
	{.cmd = 0x26, .data_len = 1,  .data = {0x1A},},
	{.cmd = 0x25, .data_len = 1,  .data = {0x08},},
	{.cmd = 0x24, .data_len = 1,  .data = {0x04},},
	{.cmd = 0x23, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x22, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x21, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x20, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x0F, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x0E, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x0D, .data_len = 1,  .data = {0x35},},
	{.cmd = 0x0C, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x0B, .data_len = 1,  .data = {0x32},},
	{.cmd = 0x0A, .data_len = 1,  .data = {0x33},},
	{.cmd = 0x09, .data_len = 1,  .data = {0x17},},
	{.cmd = 0x08, .data_len = 1,  .data = {0x19},},
	{.cmd = 0x07, .data_len = 1,  .data = {0x1B},},
	{.cmd = 0x06, .data_len = 1,  .data = {0x1D},},
	{.cmd = 0x05, .data_len = 1,  .data = {0x05},},
	{.cmd = 0x04, .data_len = 1,  .data = {0x09},},
	{.cmd = 0x03, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x02, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x01, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x00, .data_len = 1,  .data = {0x00},},
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20, 0x10, 0x17},},
	{.cmd = 0x39, .data_len = 1,  .data = {0x3c},},
	{.cmd = 0x37, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x1F, .data_len = 1,  .data = {0x80},},
	{.cmd = 0x1A, .data_len = 1,  .data = {0x80},},
	{.cmd = 0x18, .data_len = 1,  .data = {0xA0},},
	{.cmd = 0x16, .data_len = 1,  .data = {0x12},},
	{.cmd = 0x14, .data_len = 1,  .data = {0xAA},},
	{.cmd = 0x11, .data_len = 1,  .data = {0xAA},},
	{.cmd = 0x10, .data_len = 1,  .data = {0x0E},},
	{.cmd = 0x0B, .data_len = 1,  .data = {0xC3},},
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20, 0x10, 0x18},},
	{.cmd = 0x3A, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x1F, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x01, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x00, .data_len = 1,  .data = {0x1E},},
	{.cmd = 0xff, .data_len = 3,  .data = {0x20, 0x10, 0x2D},},
	{.cmd = 0x01, .data_len = 1,  .data = {0x3E},},
	{.cmd = 0x02, .data_len = 1,  .data = {0x00},},
	{.cmd = 0xff, .data_len = 3,  .data = {0x20, 0x10, 0x31},},
	{.cmd = 0x39, .data_len = 1,  .data = {0xf0},},
	{.cmd = 0x38, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x37, .data_len = 1,  .data = {0xe8},},
	{.cmd = 0x36, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x35, .data_len = 1,  .data = {0xCF},},
	{.cmd = 0x34, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x33, .data_len = 1,  .data = {0xBA},},
	{.cmd = 0x32, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x31, .data_len = 1,  .data = {0xA2},},
	{.cmd = 0x30, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x2f, .data_len = 1,  .data = {0x8E},},
	{.cmd = 0x2e, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x2d, .data_len = 1,  .data = {0x7e},},
	{.cmd = 0x2c, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x2b, .data_len = 1,  .data = {0x62},},
	{.cmd = 0x2a, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x29, .data_len = 1,  .data = {0x38},},
	{.cmd = 0x28, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x27, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x26, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x25, .data_len = 1,  .data = {0xd8},},
	{.cmd = 0x24, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x23, .data_len = 1,  .data = {0xa2},},
	{.cmd = 0x22, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x21, .data_len = 1,  .data = {0x78},},
	{.cmd = 0x20, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x1f, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x1e, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x1d, .data_len = 1,  .data = {0xfa},},
	{.cmd = 0x1c, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x1b, .data_len = 1,  .data = {0xbb},},
	{.cmd = 0x1a, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x19, .data_len = 1,  .data = {0x70},},
	{.cmd = 0x18, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x17, .data_len = 1,  .data = {0x3e},},
	{.cmd = 0x16, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x15, .data_len = 1,  .data = {0xf4},},
	{.cmd = 0x14, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x13, .data_len = 1,  .data = {0xd0},},
	{.cmd = 0x12, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x11, .data_len = 1,  .data = {0xa0},},
	{.cmd = 0x10, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x0f, .data_len = 1,  .data = {0x90},},
	{.cmd = 0x0e, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x0d, .data_len = 1,  .data = {0x7c},},
	{.cmd = 0x0c, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x0b, .data_len = 1,  .data = {0x6a},},
	{.cmd = 0x0a, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x09, .data_len = 1,  .data = {0x58},},
	{.cmd = 0x08, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x07, .data_len = 1,  .data = {0x4c},},
	{.cmd = 0x06, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x05, .data_len = 1,  .data = {0x2f},},
	{.cmd = 0x04, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x03, .data_len = 1,  .data = {0x14},},
	{.cmd = 0x02, .data_len = 1,  .data = {0x00},},
	{.cmd = 0xff, .data_len = 3,  .data = {0x20, 0x10, 0x32},},
	{.cmd = 0x39, .data_len = 1,  .data = {0xf0},},
	{.cmd = 0x38, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x37, .data_len = 1,  .data = {0xe8},},
	{.cmd = 0x36, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x35, .data_len = 1,  .data = {0xCF},},
	{.cmd = 0x34, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x33, .data_len = 1,  .data = {0xBA},},
	{.cmd = 0x32, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x31, .data_len = 1,  .data = {0xA2},},
	{.cmd = 0x30, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x2f, .data_len = 1,  .data = {0x8E},},
	{.cmd = 0x2e, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x2d, .data_len = 1,  .data = {0x7E},},
	{.cmd = 0x2c, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x2b, .data_len = 1,  .data = {0x62},},
	{.cmd = 0x2a, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x29, .data_len = 1,  .data = {0x38},},
	{.cmd = 0x28, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x27, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x26, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x25, .data_len = 1,  .data = {0xd8},},
	{.cmd = 0x24, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x23, .data_len = 1,  .data = {0xa2},},
	{.cmd = 0x22, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x21, .data_len = 1,  .data = {0x78},},
	{.cmd = 0x20, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x1f, .data_len = 1,  .data = {0x34},},
	{.cmd = 0x1e, .data_len = 1,  .data = {0x02},},
	{.cmd = 0x1d, .data_len = 1,  .data = {0xfa},},
	{.cmd = 0x1c, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x1b, .data_len = 1,  .data = {0xbb},},
	{.cmd = 0x1a, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x19, .data_len = 1,  .data = {0x70},},
	{.cmd = 0x18, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x17, .data_len = 1,  .data = {0x3e},},
	{.cmd = 0x16, .data_len = 1,  .data = {0x01},},
	{.cmd = 0x15, .data_len = 1,  .data = {0xf4},},
	{.cmd = 0x14, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x13, .data_len = 1,  .data = {0xd0},},
	{.cmd = 0x12, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x11, .data_len = 1,  .data = {0xa0},},
	{.cmd = 0x10, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x0f, .data_len = 1,  .data = {0x90},},
	{.cmd = 0x0e, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x0d, .data_len = 1,  .data = {0x7c},},
	{.cmd = 0x0c, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x0b, .data_len = 1,  .data = {0x6a},},
	{.cmd = 0x0a, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x09, .data_len = 1,  .data = {0x58},},
	{.cmd = 0x08, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x07, .data_len = 1,  .data = {0x4c},},
	{.cmd = 0x06, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x05, .data_len = 1,  .data = {0x2F},},
	{.cmd = 0x04, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x03, .data_len = 1,  .data = {0x14},},
	{.cmd = 0x02, .data_len = 1,  .data = {0x00},},
	{.cmd = 0xff, .data_len = 3,  .data = {0x20, 0x10, 0x40},},
	{.cmd = 0x86, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x83, .data_len = 1,  .data = {0xC4},},
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20, 0x10, 0x42},},
	{.cmd = 0x06, .data_len = 1,  .data = {0x03},},
	{.cmd = 0x05, .data_len = 1,  .data = {0x3D},},
	{.cmd = 0xFF, .data_len = 3,  .data = {0x20, 0x10, 0x45},},
	{.cmd = 0x03, .data_len = 1,  .data = {0xB4},},
	{.cmd = 0x02, .data_len = 1,  .data = {0x00},},
	{.cmd = 0x01, .data_len = 1,  .data = {0xC6},},
#if CONFIG_SPD2010_TOUCH
	{.cmd = 0xFF, .data_len =3,  .data = { 0x20, 0x10, 0x50},},
	{.cmd = 0x08, .data_len =1,  .data = { 0x55,},},
	{.cmd = 0x05, .data_len =1,  .data = { 0x08,},},
	{.cmd = 0x01, .data_len =1,  .data = { 0xA6,},},
	{.cmd = 0x00, .data_len =1,  .data = { 0xA6,},},
#endif /* end of CONFIG_SPD2010_TOUCH */
	{.cmd = 0xFF, .data_len = 3, .data = {0x20, 0x10, 0xA0},},
	{.cmd = 0x08, .data_len = 1, .data = {0xdc},},
	{.cmd = 0xFF, .data_len = 3, .data = {0x20, 0x10, 0x00},},
	{.cmd = 0x2A, .data_len = 4, .data = {0x00, 0x00, 0x00, 0xB3},},
	{.cmd = 0x2B, .data_len = 4, .data = {0x00, 0x00, 0x01, 0xC5},},
	{.cmd = 0x3A, .data_len = 1, .data = {0x05},},
};

static int spd2010_transmit(uint8_t cmd, const void *tx_data, size_t tx_len)
{
    ips_lcd_write_command(cmd);
    for (size_t i = 0; i < tx_len; i++) {
        ips_lcd_write_data(((uint8_t *)tx_data)[i]);
    }
    return 0;
}

#define SPD2010_SOFTRESET 0x01
#define SPD2010_ID	  0x04
#define SPD2010_RDISPMODE 0x09
#define SPD2010_RDDPM	  0x0A
#define SPD2010_RDDMADCTL 0x0B
#define SPD2010_RDPIX	  0x0C
#define SPD2010_RDDIM	  0x0D
#define SPD2010_RDDSM	  0x0E
#define SPD2010_RDDSDR	  0x0F
#define SPD2010_SLPIN	  0x10
#define SPD2010_SLPOUT	  0x11
#define SPD2010_NORON	  0x13
#define SPD2010_INVOFF	  0x20
#define SPD2010_INVON	  0x21
#define SPD2010_DISPOFF	  0x28
#define SPD2010_DISPON	  0x29
#define SPD2010_SETCOL	  0x2A
#define SPD2010_SETPAGE	  0x2B
#define SPD2010_WRMEMST	  0x2C
#define SPD2010_TEOFF	  0x34
#define SPD2010_TEON	  0x35
#define SPD2010_MADCTR	  0x36
#define SPD2010_IDMOFF	  0x38
#define SPD2010_IDMON	  0x39
#define SPD2010_SETPIXEL  0x3A
#define SPD2010_WRMEMCONT 0x3C
#define SPD2010_TESS	  0x44
#define SPD2010_RSS	  0x45
#define SPD2010_SETBR	  0x51
#define SPD2010_SETMODE	  0x53
#define SPD2010_RDMODE	  0x54
#define SPD2010_SETPWR	  0x55
#define SPD2010_RDPWR	  0x56
#define SPD2010_SETMINBR  0x5E
#define SPD2010_RDMINBR	  0x5F
#define SPD2010_RDDDBST	  0xA1
#define SPD2010_RDDDBCON  0xA8

static int spd2010_exit_sleep()
{
	return spd2010_transmit(SPD2010_SLPOUT, NULL, 0);
}

void ips_lcd_init(void)
{
    ips_lcd_io_mux_init();
    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);

    gpiohs_set_pin(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_PV_LOW);
    msleep(100);
    gpiohs_set_pin(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_PV_HIGH);
    msleep(100);
	uint16_t i;
    
	for (i = 0; i < sizeof(items) / sizeof(items[0]); i++) {
		int ret = spd2010_transmit(items[i].cmd, &items[i].data, items[i].data_len);

		if (ret) {
			return ret;
		}
	}
    spd2010_exit_sleep();
    gpiohs_set_pin(SPI_IPS_LCD_BL_GPIO_NUM, GPIO_PV_LOW);
    LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);
}

void ips_lcd_color_bar_test(void)
{
    uint8_t color_bar_w = LCD_W / 8;
    LCD_Fill(color_bar_w * 0, 0, color_bar_w, LCD_H, WHITE);
    LCD_Fill(color_bar_w * 1, 0, color_bar_w, LCD_H, YELLOW);
    LCD_Fill(color_bar_w * 2, 0, color_bar_w, LCD_H, GBLUE);
    LCD_Fill(color_bar_w * 3, 0, color_bar_w, LCD_H, GREEN);
    LCD_Fill(color_bar_w * 4, 0, color_bar_w, LCD_H, CYAN);
    LCD_Fill(color_bar_w * 5, 0, color_bar_w, LCD_H, RED);
    LCD_Fill(color_bar_w * 6, 0, color_bar_w, LCD_H, BLUE);
    LCD_Fill(color_bar_w * 7, 0, color_bar_w, LCD_H, BLACK);
}
