#ifndef __BOARD_CODNIF_H__
#define __BOARD_CODNIF_H__

#define CONFIG_LOG_COLORS 1

#define BOARD_MAIX_BIT      1

#define BOARD_MAIX_DOCK     2
#define BOARD_MAIX_GO       3
#define BOARD_MAIX_DUINO    4

#define BOARD_MAIX_M1N      5
#define BOARD_MAIX_CUBE     6
#define BOARD_MAIX_NEW_GO   7

#define BOARD       BOARD_MAIX_DUINO
#define BOARD_NAME  "BOARD_MAIX_DUINO"



/******************************************************************************/

#define CAMERA_OV2640   1
#define CAMERA_OV5640   2
#define CAMERA_OV7740   3

#define CAMERA_GC0328   4
#define CAMERA_GC2145   5

#define CAMERA  CAMERA_OV2640

/******************************************************************************/

#if ((BOARD == BOARD_MAIX_DUINO) || (BOARD == BOARD_MAIX_NEW_GO))

#define DCX_GPIO_HS_NUM        (2)
#define RST_GPIO_HS_NUM        (30)

#define LCD_RST_PIN_NUM 37
#define LCD_DCX_PIN_NUM 38
#define LCD_CS_PIN_NUM  36
#define LCD_CLK_PIN_NUM 39

#define SPI_CHANNEL             0
#define SPI_SLAVE_SELECT        3

#elif (BOARD == BOARD_MAIX_CUBE)

#else

#endif

/******************************************************************************/

#if (BOARD == BOARD_MAIX_DUINO)

#define LCD_X_MAX   (240)
#define LCD_Y_MAX   (320)

#elif  (BOARD == BOARD_MAIX_NEW_GO)

#define LCD_X_MAX   (320)
#define LCD_Y_MAX   (480)

#else

#endif
/******************************************************************************/
#endif
