#include "board_config.h"
#include "Ap_29demo.h"
#include "epd.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "sleep.h"
#include "spi.h"

#define DATALENGTH 8

static const char *TAG = "epd";

void epd_io_mux_init(void)
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
    // gpiohs_set_drive_mode(SPI_IPS_LCD_BL_GPIO_NUM, GPIO_DM_OUTPUT);

    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_HIGH);
    gpiohs_set_pin(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_PV_HIGH);
    // gpiohs_set_pin(SPI_IPS_LCD_BL_GPIO_NUM, GPIO_PV_HIGH);

    // fpioa_set_function(SPI_IPS_LCD_BL_PIN_NUM, FUNC_GPIOHS2);
    gpiohs_set_drive_mode(SPI_IPS_LCD_BL_PIN_NUM, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_pin_edge(SPI_IPS_LCD_BL_PIN_NUM, GPIO_PE_BOTH);
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

static void epd_write_command(uint8_t command)
{
    uint8_t cmd[1] = {command};
    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_LOW);

    spi_init(SPI_INDEX, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    spi_send_data_standard(SPI_INDEX, SPI_CHIP_SELECT_NSS, cmd, 1, NULL, 0);

    gpiohs_set_pin(SPI_IPS_LCD_DC_GPIO_NUM, GPIO_PV_HIGH);
}

static void epd_write_data(uint8_t data)
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
    // ips_lcd_write_data(dat >> 8);
    // ips_lcd_write_data(dat & 0xFF);
}

#define EPD_W21_WriteCMD epd_write_command
#define EPD_W21_WriteDATA epd_write_data
/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
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
    spi_init_non_standard(SPI_INDEX, 0 /*instrction length*/, 32 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);
    spi_fill_data_dma(DMAC_CHANNEL0, SPI_INDEX, SPI_CHIP_SELECT_NSS, &data, num / 2);
}
/******************************************************/
uint8_t k = 1;
void lut_bw(void)
{
    unsigned int count;
    EPD_W21_WriteCMD(0x20);
    for(count = 0; count < 15; count++)
    {
        EPD_W21_WriteDATA(lut_vcom0[count]);
    }

    EPD_W21_WriteCMD(0x21);
    for(count = 0; count < 15; count++)
    {
        EPD_W21_WriteDATA(lut_w[count]);
    }

    EPD_W21_WriteCMD(0x22);
    for(count = 0; count < 15; count++)
    {
        EPD_W21_WriteDATA(lut_b[count]);
    }

    EPD_W21_WriteCMD(0x23);
    for(count = 0; count < 15; count++)
    {
        EPD_W21_WriteDATA(lut_g1[count]);
    }

    EPD_W21_WriteCMD(0x24);
    for(count = 0; count < 15; count++)
    {
        EPD_W21_WriteDATA(lut_g2[count]);
    }
}

void lut_red(void)
{
    unsigned int count;
    EPD_W21_WriteCMD(0x25);
    for(count = 0; count < 15; count++)
    {
        EPD_W21_WriteDATA(lut_vcom1[count]);
    }

    EPD_W21_WriteCMD(0x26);
    for(count = 0; count < 15; count++)
    {
        EPD_W21_WriteDATA(lut_red0[count]);
    }

    EPD_W21_WriteCMD(0x27);
    for(count = 0; count < 15; count++)
    {
        EPD_W21_WriteDATA(lut_red1[count]);
    }
}

void lcd_chkstatus(void)
{
    while(!gpiohs_get_pin(SPI_IPS_LCD_BL_GPIO_NUM))
        ;
}


void Ultrachip(void)
{
    unsigned int i;
    for(i = 0; i < 10000; i++)
    {
        EPD_W21_WriteDATA(G_Ultrachip[i]);
    }
    msleep(2);
}

void Ultrachip_red(void)
{
    unsigned int i;
    for(i = 0; i < 5000; i++)
    {
        EPD_W21_WriteDATA(G_Ultrachip_red[i]);
    }
    msleep(2);
}

void Ultrachip1(void)
{
    unsigned int i;
    for(i = 0; i < 10000; i++)
    {
        EPD_W21_WriteDATA(G_Ultrachip1[i]);
    }
    msleep(2);
}

void Ultrachip_red1(void)
{
    unsigned int i;
    for(i = 0; i < 5000; i++)
    {
        EPD_W21_WriteDATA(G_Ultrachip_red1[i]);
    }
    msleep(2);
}

void pic_display(void)
{
    if(k == 0)
    {
        EPD_W21_WriteCMD(0x10); //开始传输黑白图像
        Ultrachip();
        EPD_W21_WriteCMD(0x13); //开始传输红图像
        Ultrachip_red();
    }
    if(k == 1)
    {
        EPD_W21_WriteCMD(0x10); //开始传输黑白图像
        Ultrachip1();
        EPD_W21_WriteCMD(0x13); //开始传输红图像
        Ultrachip_red1();
    }
}


void EPD_display_init(void)
{
    epd_io_mux_init();

    gpiohs_set_pin(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_PV_LOW);
    msleep(100);
    gpiohs_set_pin(SPI_IPS_LCD_RST_GPIO_NUM, GPIO_PV_HIGH);
    msleep(100);

    gpiohs_set_pin(SPI_IPS_LCD_BL_GPIO_NUM, GPIO_PV_LOW);
    msleep(100);

    EPD_W21_WriteCMD(0x01);
    EPD_W21_WriteDATA(0x07); //设置高低电压
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(0x0B); //红色电压设置，值越大红色越深
    EPD_W21_WriteDATA(0x00);

    EPD_W21_WriteCMD(0x06); //boost设定
    EPD_W21_WriteDATA(0x07);
    EPD_W21_WriteDATA(0x07);
    EPD_W21_WriteDATA(0x07);

    EPD_W21_WriteCMD(0x04); //上电
    lcd_chkstatus();        //查看芯片状态

    EPD_W21_WriteCMD(0X00);
    EPD_W21_WriteDATA(0xcf); //选择最大分辨率

    EPD_W21_WriteCMD(0X50);
    EPD_W21_WriteDATA(0x37);

    EPD_W21_WriteCMD(0x30); //PLL设定
    EPD_W21_WriteDATA(0x39);

    EPD_W21_WriteCMD(0x61);  //像素设定
    EPD_W21_WriteDATA(0xC8); //200像素
    EPD_W21_WriteDATA(0x00); //200像素
    EPD_W21_WriteDATA(0xC8);

    EPD_W21_WriteCMD(0x82); //vcom设定
    EPD_W21_WriteDATA(0x18);

    lut_bw();
    lut_red();

    pic_display();

    EPD_W21_WriteCMD(0x12);
    lcd_chkstatus();
    msleep(300000); //wait for fresh display

    EPD_W21_WriteCMD(0x82); //to solve Vcom drop
    EPD_W21_WriteDATA(0x00);

    EPD_W21_WriteCMD(0x01);  //power setting
    EPD_W21_WriteDATA(0x02); //gate switch to external
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(0x00);

    msleep(1500);           //delay 1.5S
    EPD_W21_WriteCMD(0X02); //power off
}

// /////////////////////////////Enter deep sleep mode////////////////////////
// void EPD_deep_sleep(void) //Enter deep sleep mode
// {
//     EPD_W21_WriteCMD(0X50);
//     EPD_W21_WriteDATA(0xf7);
//     EPD_W21_WriteCMD(0X02); //power off
//     msleep(100);
//     lcd_chkstatus();
//     EPD_W21_WriteCMD(0X07); //deep sleep
//     EPD_W21_WriteDATA(0xA5);
//     msleep(3000);
// }
// /***************************full display function*************************************/
// void EPD_full_display_clearing(void) //Clear the screen for the first time
// {
//     unsigned int i, j;

//     for(j = 0; j < 2; j++)
//     {
//         EPD_W21_WriteCMD(0x10);
//         for(i = 0; i < 5000; i++)
//         {
//             EPD_W21_WriteDATA(0x00);
//         }
//         msleep(2);
//         EPD_W21_WriteCMD(0x13);
//         for(i = 0; i < 5000; i++)
//         {
//             EPD_W21_WriteDATA(0xff);
//         }
//         msleep(2);

//         EPD_W21_WriteCMD(0x04); //Power on
//         msleep(100);
//         lcd_chkstatus();
//         EPD_W21_WriteCMD(0x12); //DISPLAY REFRESH
//         msleep(100);            //!!!The delay here is necessary, 200uS at least!!!
//         lcd_chkstatus();
//     }
// }

// void EPD_full_display(const unsigned char *old_data, const unsigned char *new_data, unsigned char mode) // mode0:Refresh picture1,mode1:Refresh picture2... ,mode2:Clear
// {
//     unsigned int i;

//     if(mode == 0) //mode0:Refresh picture1
//     {
//         EPD_W21_WriteCMD(0x10);
//         for(i = 0; i < 5000; i++)
//         {
//             EPD_W21_WriteDATA(0xff);
//         }
//         msleep(2);
//         EPD_W21_WriteCMD(0x13);
//         for(i = 0; i < 5000; i++)
//         {
//             EPD_W21_WriteDATA(new_data[i]);
//         }
//         msleep(2);
//     }

//     else if(mode == 1) //mode0:Refresh picture2...
//     {
//         EPD_W21_WriteCMD(0x10);
//         for(i = 0; i < 5000; i++)
//         {
//             EPD_W21_WriteDATA(old_data[i]);
//         }
//         msleep(2);
//         EPD_W21_WriteCMD(0x13);
//         for(i = 0; i < 5000; i++)
//         {
//             EPD_W21_WriteDATA(new_data[i]);
//         }
//         msleep(2);
//     }

//     else
//     {
//         EPD_W21_WriteCMD(0x10);
//         for(i = 0; i < 5000; i++)
//         {
//             EPD_W21_WriteDATA(old_data[i]);
//         }
//         msleep(2);
//         EPD_W21_WriteCMD(0x13);
//         for(i = 0; i < 5000; i++)
//         {
//             EPD_W21_WriteDATA(0xff);
//         }
//         msleep(2);
//     }

//     EPD_W21_WriteCMD(0x04); //Power on
//     msleep(100);
//     lcd_chkstatus();

//     EPD_W21_WriteCMD(0x12); //DISPLAY REFRESH
//     msleep(100);            //!!!The delay here is necessary, 200uS at least!!!
//     lcd_chkstatus();
// }

// /***************************partial display function*************************************/

// void partial_display(uint16_t x_start, uint16_t x_end, uint16_t y_start, uint16_t y_end, const unsigned char *old_data, const unsigned char *new_data, unsigned char mode) //partial display
// {
//     unsigned datas, i;
//     EPD_W21_WriteCMD(0X50);
//     EPD_W21_WriteDATA(0x97);
//     lut1();

//     EPD_W21_WriteCMD(0x91);       //This command makes the display enter partial mode
//     EPD_W21_WriteCMD(0x90);       //resolution setting
//     EPD_W21_WriteDATA(x_start);   //x-start
//     EPD_W21_WriteDATA(x_end - 1); //x-end
//     EPD_W21_WriteDATA(0);         //x Reserved

//     EPD_W21_WriteDATA(y_start); //y-start
//     EPD_W21_WriteDATA(0);       //y Reserved
//     EPD_W21_WriteDATA(y_end);   //y-end
//     EPD_W21_WriteDATA(0x01);
//     datas = (x_end - x_start) * (y_end - y_start) / 8;

//     EPD_W21_WriteCMD(0x10); //writes Old data to SRAM for programming
//     if(mode == 0)
//     {
//         for(i = 0; i < datas; i++)
//         {
//             EPD_W21_WriteDATA(0xff);
//         }
//     } else
//     {
//         for(i = 0; i < datas; i++)
//         {
//             EPD_W21_WriteDATA(old_data[i]);
//         }
//     }
//     EPD_W21_WriteCMD(0x13); //writes New data to SRAM.
//     for(i = 0; i < datas; i++)
//     {
//         EPD_W21_WriteDATA(new_data[i]);
//     }

//     EPD_W21_WriteCMD(0x12); //DISPLAY REFRESH
//     msleep(100);            //!!!The delay here is necessary, 200uS at least!!!
//     lcd_chkstatus();
// }