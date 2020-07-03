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

// # I2C1_SDA      28 CDATA    -> IO_31
// # I2C1_SCL      1 CCLK      -> IO_30

// # I2S_MCLK      2 MCLK      -> IO_19
// # I2S_SCLK      7 SCLK      -> IO_35
// # I2S_DOUT      8 ASDOUT    -> IO_34
// # I2S_LRCK(WS)  9 LECK      -> IO_33
// # I2S_DIN       10 DSDIN    -> IO_18

    #define SD_SPI_SCLK     27
    #define SD_SPI_MOSI     28
    #define SD_SPI_MISO     26

    #define SD_SPI_CS_PIN   29

    #define CONFIG_PIN_NUM_ES8374_I2C_SCL 30
    #define CONFIG_PIN_NUM_ES8374_I2C_SDA 31

    #define CONFIG_PIN_NUM_ES8374_MCLK 19
    #define CONFIG_PIN_NUM_ES8374_SCLK 35
    #define CONFIG_PIN_NUM_ES8374_DOUT 34
    #define CONFIG_PIN_NUM_ES8374_WS   33
    #define CONFIG_PIN_NUM_ES8374_DIN  18

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

    /*
    AUDIO_ES8374_SCL    24
    AUDIO_ES8374_SDA    27

    AUDIO_ES8374_MCLK    
    AUDIO_ES8374_SCLK    

    AUDIO_ES8374_DOUT    
    AUDIO_ES8374_DIN     
    AUDIO_ES8374_WS      
    */
    #define CONFIG_PIN_NUM_ES8374_I2C_SCL 24
    #define CONFIG_PIN_NUM_ES8374_I2C_SDA 27

    // #define CONFIG_PIN_NUM_ES8374_MCLK 13
    // #define CONFIG_PIN_NUM_ES8374_SCLK 21
    // #define CONFIG_PIN_NUM_ES8374_DOUT 34
    // #define CONFIG_PIN_NUM_ES8374_WS   18
    // #define CONFIG_PIN_NUM_ES8374_DIN  35
    #define CONFIG_PIN_NUM_ES8374_MCLK 19
    #define CONFIG_PIN_NUM_ES8374_SCLK 35
    #define CONFIG_PIN_NUM_ES8374_DOUT 34
    #define CONFIG_PIN_NUM_ES8374_WS   33
    #define CONFIG_PIN_NUM_ES8374_DIN  18

#endif



#endif