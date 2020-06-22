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

    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    spi_send_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, cmd, 2, NULL, 0);
}

static void spi_read_reg(uint8_t reg_addr, uint8_t *reg_data)
{
    uint8_t cmd[1] = {reg_addr};

    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    spi_receive_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, cmd, 1, reg_data, 1);
}

static void ips_lcd_write_command(uint8_t command)
{
    uint8_t cmd[1] = {command};
    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_LOW);

    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    spi_send_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, cmd, 1, NULL, 0);

    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_HIGH);
}

static void ips_lcd_write_data(uint8_t data)
{
    uint8_t cmd[1] = {data};
    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_HIGH);

    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
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
    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
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
    spi_init_non_standard(SPI_INDEX, 0/*instrction length*/, 32/*address length*/, 0/*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT/*spi address trans mode*/);
    spi_fill_data_dma(DMAC_CHANNEL0, SPI_INDEX, SPI_CHIP_SELECT_NSS, &data, num/2);

}
/******************************************************/

void ips_lcd_init(void)
{
    ips_lcd_io_mux_init();

    gpiohs_set_pin(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_PV_LOW);
    msleep(100);
    gpiohs_set_pin(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_PV_HIGH);
    msleep(100);

    gpiohs_set_pin(SPI_IPS_LCD_BL_GPIO_NUM, GPIO_PV_LOW);
    msleep(100);

    ips_lcd_write_command(0x11);
    msleep(120);
    ips_lcd_write_command(0x36);
    if(USE_HORIZONTAL == 0)
        ips_lcd_write_data(0x00);
    else if(USE_HORIZONTAL == 1)
        ips_lcd_write_data(0xC0);
    else if(USE_HORIZONTAL == 2)
        ips_lcd_write_data(0x70);
    else
        ips_lcd_write_data(0xA0);

    ips_lcd_write_command(0x3A);
    ips_lcd_write_data(0x05);

    ips_lcd_write_command(0xB2);
    ips_lcd_write_data(0x0C);
    ips_lcd_write_data(0x0C);
    ips_lcd_write_data(0x00);
    ips_lcd_write_data(0x33);
    ips_lcd_write_data(0x33);

    ips_lcd_write_command(0xB7);
    ips_lcd_write_data(0x35);

    ips_lcd_write_command(0xBB);
    ips_lcd_write_data(0x19);

    ips_lcd_write_command(0xC0);
    ips_lcd_write_data(0x2C);

    ips_lcd_write_command(0xC2);
    ips_lcd_write_data(0x01);

    ips_lcd_write_command(0xC3);
    ips_lcd_write_data(0x12);

    ips_lcd_write_command(0xC4);
    ips_lcd_write_data(0x20);

    ips_lcd_write_command(0xC6);
    ips_lcd_write_data(0x0F);

    ips_lcd_write_command(0xD0);
    ips_lcd_write_data(0xA4);
    ips_lcd_write_data(0xA1);

    ips_lcd_write_command(0xE0);
    ips_lcd_write_data(0xD0);
    ips_lcd_write_data(0x04);
    ips_lcd_write_data(0x0D);
    ips_lcd_write_data(0x11);
    ips_lcd_write_data(0x13);
    ips_lcd_write_data(0x2B);
    ips_lcd_write_data(0x3F);
    ips_lcd_write_data(0x54);
    ips_lcd_write_data(0x4C);
    ips_lcd_write_data(0x18);
    ips_lcd_write_data(0x0D);
    ips_lcd_write_data(0x0B);
    ips_lcd_write_data(0x1F);
    ips_lcd_write_data(0x23);

    ips_lcd_write_command(0xE1);
    ips_lcd_write_data(0xD0);
    ips_lcd_write_data(0x04);
    ips_lcd_write_data(0x0C);
    ips_lcd_write_data(0x11);
    ips_lcd_write_data(0x13);
    ips_lcd_write_data(0x2C);
    ips_lcd_write_data(0x3F);
    ips_lcd_write_data(0x44);
    ips_lcd_write_data(0x51);
    ips_lcd_write_data(0x2F);
    ips_lcd_write_data(0x1F);
    ips_lcd_write_data(0x1F);
    ips_lcd_write_data(0x20);
    ips_lcd_write_data(0x23);

    ips_lcd_write_command(0x21);

    ips_lcd_write_command(0x29);

    LCD_Fill(0,0,LCD_W,LCD_H,BLACK);


}
