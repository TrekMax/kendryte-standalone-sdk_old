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
// # [4|5] [7  |VCC]
// # [3|6] [15 | 21]
// # [2|7] [20 |  8]
// # [1|8] [GND|  6]

#define SPI_PSRAM_CS_PIN_NUM    20
#define SPI_PSRAM_SCK_PIN_NUM   21
#define SPI_PSRAM_MOSI_PIN_NUM  8
#define SPI_PSRAM_MISO_PIN_NUM  15

#define SPI_PSRAM_D2_PIN_NUM    6
#define SPI_PSRAM_D3_PIN_NUM    7


#elif (BOARD == BOARD_MAIX_NEW_GO)

#else

#endif

#endif
