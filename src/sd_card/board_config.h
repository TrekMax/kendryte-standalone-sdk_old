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


#define BOARD BOARD_MAIX_NEW_GO
#define BOARD_NAME  "BOARD_MAIX_NEW_GO"

#if ((BOARD == BOARD_MAIX_CUBE) || (BOARD == BOARD_MAIX_M1N))

#define SPI_INDEX           1
#define SPI_SCLK_RATE       600*1000
#define SPI_CHIP_SELECT_NSS 0//SPI_CHIP_SELECT_0
// SPMOD Interface
// # [4|5] [7  |VCC]
// # [3|6] [15 | 21]
// # [2|7] [20 |  8]
// # [1|8] [GND|  6]

#define SPI_FALSH_CS_PIN_NUM    20
#define SPI_FALSH_SCK_PIN_NUM   21
#define SPI_FALSH_MOSI_PIN_NUM  8
#define SPI_FALSH_MISO_PIN_NUM  15

#define SPI_FALSH_D2_PIN_NUM    6
#define SPI_FALSH_D3_PIN_NUM    7


#define SD_SPI_SCLK     27
#define SD_SPI_MOSI     28
#define SD_SPI_MISO     26

#define SD_SPI_CS_PIN   29

#elif (BOARD == BOARD_MAIX_NEW_GO)
    /*
    SD_SPI_SCLK     11
    SD_SPI_MOSI     10
    SD_SPI_MISO     6

    SD_SPI_CS_PIN   26
    */
    #define SD_SPI_SCLK     11
    #define SD_SPI_MOSI     10
    #define SD_SPI_MISO     6
    
    #define SD_SPI_CS_PIN   26

#else

#endif

#endif
