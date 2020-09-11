#ifndef __MYSPI_H
#define __MYSPI_H

#include <stdint.h>

extern volatile uint8_t w5500_irq_flag;

void eth_w5500_spi_init(void);
void eth_w5500_reset(uint8_t val);

//lib call

uint8_t eth_w5500_spi_read(void);
void eth_w5500_spi_write(uint8_t data);

void eth_w5500_spi_cs_sel(void);
void eth_w5500_spi_cs_desel(void);

#endif
