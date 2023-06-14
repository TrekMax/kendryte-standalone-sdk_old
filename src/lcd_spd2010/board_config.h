#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#define CONFIG_LOG_COLORS 1

#define BOARD_MAIX_DOCK     1
#define BOARD_MAIX_DUINO    2
#define BOARD_MAIX_GO       3
#define BOARD_MAIX_BIT      4

#define BOARD_MAIX_M1N      5
#define BOARD_MAIX_CUBE     6
#define BOARD_MAIX_NEW_GO   7


#define BOARD BOARD_MAIX_M1N
#define BOARD_NAME  "BOARD_MAIX_M1N"

#if ((BOARD == BOARD_MAIX_CUBE) || (BOARD == BOARD_MAIX_M1N))

#define SPI_INDEX           1
#define SPI_SCLK_RATE       600*1000
#define SPI_CHIP_SELECT_NSS 0//SPI_CHIP_SELECT_0
// SPMOD Interface
// # [4|5] [7  |VCC] [RST|3V3]
// # [3|6] [15 | 21] [D/C|SCK]
// # [2|7] [20 |  8] [CS |SI ]
// # [1|8] [GND|  6] [GND|BL ]

#define SPI_IPS_LCD_RST_GPIO_NUM    8
#define SPI_IPS_LCD_DC_GPIO_NUM     9
#define SPI_IPS_LCD_CS_PIN_NUM      10
#define SPI_IPS_LCD_SCK_PIN_NUM     11
#define SPI_IPS_LCD_MOSI_PIN_NUM    12
// #define SPI_IPS_LCD_MISO_PIN_NUM    15

#define SPI_IPS_LCD_RST_PIN_NUM     8
#define SPI_IPS_LCD_DC_PIN_NUM      9

#define SPI_IPS_LCD_BL_PIN_NUM      6
#define SPI_IPS_LCD_BL_GPIO_NUM     6


#define USE_HORIZONTAL 2    //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 135
#define LCD_H 240

#else
#define LCD_W 240
#define LCD_H 135
#endif


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



#elif (BOARD == BOARD_MAIX_NEW_GO)

#else

#endif

#endif
