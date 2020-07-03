#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#define CONFIG_LOG_COLORS 1

#define BOARD_MAIX_DOCK 1
#define BOARD_MAIX_DUINO 2
#define BOARD_MAIX_GO 3
#define BOARD_MAIX_BIT 4

#define BOARD_MAIX_M1N 5
#define BOARD_MAIX_CUBE 6
#define BOARD_MAIX_NEW_GO 7

#define BOARD BOARD_MAIX_M1N
#define BOARD_NAME "BOARD_MAIX_M1N"

#if((BOARD == BOARD_MAIX_CUBE) || (BOARD == BOARD_MAIX_M1N))

#define SPI_INDEX 1
#define SPI_SCLK_RATE 600 * 1000
#define SPI_CHIP_SELECT_NSS 0 //SPI_CHIP_SELECT_0
// SPMOD Interface
// # [4|5] [7  |VCC] [RST|3V3]
// # [3|6] [15 | 21] [D/C|SCK]
// # [2|7] [20 |  8] [CS |SI ]
// # [1|8] [GND|  6] [GND|BL ]

#define SPI_IPS_LCD_CS_PIN_NUM 20
#define SPI_IPS_LCD_SCK_PIN_NUM 21
#define SPI_IPS_LCD_MOSI_PIN_NUM 8
// #define SPI_IPS_LCD_MISO_PIN_NUM    15

#define SPI_IPS_LCD_DC_PIN_NUM 15
#define SPI_IPS_LCD_BL_PIN_NUM 6
#define SPI_IPS_LCD_RST_PIN_NUM 7

#define SPI_IPS_LCD_DC_GPIO_NUM 15
#define SPI_IPS_LCD_BL_GPIO_NUM 6
#define SPI_IPS_LCD_RST_GPIO_NUM 7

#define LCD_W 240
#define LCD_H 135

#elif(BOARD == BOARD_MAIX_NEW_GO)

#else

#endif

#endif
