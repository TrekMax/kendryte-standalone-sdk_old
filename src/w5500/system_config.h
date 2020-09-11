#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

/* clang-format off */

/* some definition */
#define CONFIG_LCD_WIDTH                                        (320)
#define CONFIG_LCD_HEIGHT                                       (240)

#define CONFIG_LCD_CLK_FREQ_MHZ                                 (15)

/* Pin */
#define CONFIG_LCD_PIN_RST                                      (37)
#define CONFIG_LCD_PIN_DCX                                      (38)
#define CONFIG_LCD_PIN_WRX                                      (36)
#define CONFIG_LCD_PIN_SCK                                      (39)
// SPMOD Interface
// # [4|5] [7  |VCC] [RST|3V3]
// # [3|6] [15 | 21] [SO |SCK]
// # [2|7] [20 |  8] [CS |SI ]
// # [1|8] [GND|  6] [GND|IRQ ]



#define CONFIG_ETH_PIN_MOSI                                     (8)//(19)
#define CONFIG_ETH_PIN_MISO                                     (15)//(18)
#define CONFIG_ETH_PIN_SCLK                                     (21)//(23)
#define CONFIG_ETH_PIN_CSSX                                     (20)//(21)
#define CONFIG_ETH_PIN_INTX                                     (6)//(22)
#define CONFIG_ETH_PIN_RSTX                                     (7)//(20)

/* Pin Function */
#define CONFIG_LCD_GPIOHS_DCX                                   (0)
#define CONFIG_LCD_GPIOHS_RST                                   (1)

#define CONFIG_ETH_GPIOHS_CSX                                   (2)
#define CONFIG_ETH_GPIOHS_INT                                   (3)
#define CONFIG_ETH_GPIOHS_RST                                   (4)

/* clang-format on */

#endif
