#ifndef __BOARD_CODNIF_H__
#define __BOARD_CODNIF_H__

#define CONFIG_LOG_COLORS 1

#define BOARD_MAIX_DOCK     1
#define BOARD_MAIX_DUINO    2
#define BOARD_MAIX_CUBE     3
#define BOARD_MAIX_BIT      4
#define BOARD_MAIX_GO       5
#define BOARD_MAIX_NEW_GO   6

#define BOARD   BOARD_MAIX_NEW_GO
#define BOARD_NAME "BOARD_MAIX_NEW_GO"

#if ((BOARD == BOARD_MAIX_DUINO) || (BOARD == BOARD_MAIX_NEW_GO) || (BOARD == BOARD_MAIX_CUBE))

#define DCX_GPIO_HS_NUM        (2)
#define RST_GPIO_HS_NUM        (30)

#define LCD_RST_PIN_NUM 37
#define LCD_DCX_PIN_NUM 38
#define LCD_CS_PIN_NUM  36
#define LCD_CLK_PIN_NUM 39

#define SPI_CHANNEL             0
#define SPI_SLAVE_SELECT        3

#elif (BOARD == BOARD_MAIX_GO)

#else

#endif

/******************************************************************************/

#if (BOARD == BOARD_MAIX_DUINO)

#define LCD_X_MAX   (240)
#define LCD_Y_MAX   (320)

#define LCD_BL_CONTROL  1
#define LCD_BL_GPIONUM 17
#define LCD_BL_GPION_FUN FUNC_GPIOHS17

#elif  (BOARD == BOARD_MAIX_NEW_GO)

#define LCD_X_MAX   (320)
#define LCD_Y_MAX   (480)

#elif  (BOARD == BOARD_MAIX_CUBE)

// #define LCD_X_MAX   (240)
// #define LCD_Y_MAX   (240)
#define LCD_X_MAX   (240)
#define LCD_Y_MAX   (320)

#define LCD_BL_CONTROL  1
#define LCD_BL_GPIONUM 17
#define LCD_BL_GPION_FUN FUNC_GPIOHS17


#else

#endif

#endif
