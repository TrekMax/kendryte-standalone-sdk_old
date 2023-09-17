#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#define CONFIG_LOG_COLORS 1

#define BOARD_MAIX_M1N      5
#define BOARD BOARD_MAIX_M1N
#define BOARD_NAME  "BOARD_MAIX_M1N"

#define SPI_INDEX                       1
#define SPI_CLK_RATE                    600*1000
#define SPI_CHIP_SELECT_NSS             0//SPI_CHIP_SELECT_0

// 356(H)RGB x400(V)
#define LCD_WIDTH                       (356)
#define LCD_HIGH                        (400)

// PIN 定义，使用 QSPI 接口
#define SPI_IPS_LCD_PIN_NUM_RST                 0
#define SPI_IPS_LCD_PIN_NUM_CS                  1
#define SPI_IPS_LCD_PIN_NUM_SCK                 2
#define SPI_IPS_LCD_PIN_NUM_SIO1                3
#define SPI_IPS_LCD_PIN_NUM_SIO0_SDA_MOSI       4
#define SPI_IPS_LCD_PIN_NUM_SIO3                5
#define SPI_IPS_LCD_PIN_NUM_SIO2                6
#define SPI_IPS_LCD_PIN_NUM_DC                  7

#define SPI_IPS_LCD_GPIO_NUM_RST                 0
#define SPI_IPS_LCD_GPIO_NUM_CS                  1
#define SPI_IPS_LCD_GPIO_NUM_SCK                 2
#define SPI_IPS_LCD_GPIO_NUM_SIO1                3
#define SPI_IPS_LCD_GPIO_NUM_SIO0_SDA_MOSI       4
#define SPI_IPS_LCD_GPIO_NUM_SIO3                5
#define SPI_IPS_LCD_GPIO_NUM_SIO2                6
#define SPI_IPS_LCD_GPIO_NUM_DC                  7





//画笔颜色
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40      //棕色
#define BRRED 0XFC07      //棕红色
#define GRAY 0X8430       //灰色
#define DARKBLUE 0X01CF   //深蓝色
#define LIGHTBLUE 0X7D7C  //浅蓝色
#define GRAYBLUE 0X5458   //灰蓝色
#define LIGHTGREEN 0X841F //浅绿色
#define LGRAY 0XC618      //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE 0XA651  //浅灰蓝色(中间层颜色)
#define LBBLUE 0X2B12     //浅棕蓝色(选择条目的反色)

#endif