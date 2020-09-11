#include <stdio.h>

#include "utils.h"
#include "spi.h"
#include "fpioa.h"
#include "sleep.h"
#include "printf.h"

#include "myspi.h"
#include "gpiohs.h"

#include "system_config.h"

static void sipeed_spi_transfer_data_standard(spi_device_num_t spi_num, spi_chip_select_t chip_select,
                                              const uint8_t *tx_buff, uint8_t *rx_buff, size_t len);

volatile uint8_t w5500_irq_flag = 0;

static int irq_gpiohs(void *ctx)
{
    w5500_irq_flag = 1;
    printk("#");
    return 0;
}

/* SPI端口初始化 */
void eth_w5500_spi_init(void)
{
    printf("hard spi\r\n");

    /* init cs */
    gpiohs_set_drive_mode(CONFIG_ETH_GPIOHS_CSX, GPIO_DM_OUTPUT);
    gpiohs_set_pin(CONFIG_ETH_GPIOHS_CSX, 1);

    /* init rst */
    gpiohs_set_drive_mode(CONFIG_ETH_GPIOHS_RST, GPIO_DM_OUTPUT);
    gpiohs_set_pin(CONFIG_ETH_GPIOHS_RST, 0);
    msleep(50);
    gpiohs_set_pin(CONFIG_ETH_GPIOHS_RST, 1);
    msleep(50);

    /* init int */
    uint8_t key_dir = 0;
    gpiohs_set_drive_mode(CONFIG_ETH_GPIOHS_INT, GPIO_DM_INPUT_PULL_DOWN);
    if (key_dir)
    {
        gpiohs_set_drive_mode(CONFIG_ETH_GPIOHS_INT, GPIO_DM_INPUT_PULL_DOWN);
        gpiohs_set_pin_edge(CONFIG_ETH_GPIOHS_INT, GPIO_PE_RISING);
    }
    else
    {
        gpiohs_set_drive_mode(CONFIG_ETH_GPIOHS_INT, GPIO_DM_INPUT_PULL_UP);
        gpiohs_set_pin_edge(CONFIG_ETH_GPIOHS_INT, GPIO_PE_FALLING);
    }
    gpiohs_irq_register(CONFIG_ETH_GPIOHS_INT, 2, irq_gpiohs, NULL);

    spi_init(SPI_DEVICE_1, SPI_WORK_MODE_0, SPI_FF_STANDARD, 8, 0);
    printf("set spi clk:%d\r\n", spi_set_clk_rate(SPI_DEVICE_1, 1000000 * 8)); /*set clk rate*/
}

void eth_w5500_reset(uint8_t val)
{
    gpiohs_set_pin(CONFIG_ETH_GPIOHS_RST, val);
}

///////////////////////////////////////////////////////////////////////////////
//lib call
uint8_t eth_w5500_spi_read(void)
{
    uint8_t c, data = 0xFF;
    sipeed_spi_transfer_data_standard(SPI_DEVICE_1, SPI_CHIP_SELECT_0, &data, &c, 1);
    return c;
}

void eth_w5500_spi_write(uint8_t data)
{
    uint8_t c;
    sipeed_spi_transfer_data_standard(SPI_DEVICE_1, SPI_CHIP_SELECT_0, &data, &c, 1);
    return;
}

void eth_w5500_spi_cs_sel(void)
{
    gpiohs_set_pin(CONFIG_ETH_GPIOHS_CSX, 0);
    return;
}

void eth_w5500_spi_cs_desel(void)
{
    gpiohs_set_pin(CONFIG_ETH_GPIOHS_CSX, 1);
    return;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//这个函数进行了精简，所有不能随意传参数。
static void sipeed_spi_transfer_data_standard(spi_device_num_t spi_num, spi_chip_select_t chip_select,
                                              const uint8_t *tx_buff, uint8_t *rx_buff, size_t len)
{
    // configASSERT(spi_num < SPI_DEVICE_MAX && spi_num != 2);
    // configASSERT(len > 0);

    size_t fifo_len;
    size_t rx_len = len;
    size_t tx_len = rx_len;

    volatile spi_t *spi_handle = spi[spi_num];

    // not always need
    uint8_t tmod_offset = 8;
    set_bit(&spi_handle->ctrlr0, 3 << tmod_offset, SPI_TMOD_TRANS_RECV << tmod_offset);

    //always need
    spi_handle->ctrlr1 = (uint32_t)(tx_len / SPI_TRANS_CHAR - 1);
    spi_handle->ssienr = 0x01;
    spi_handle->ser = 1U << chip_select;

    uint32_t i = 0;
    while (tx_len)
    {
        fifo_len = 32 - spi_handle->txflr;
        fifo_len = fifo_len < tx_len ? fifo_len : tx_len;

        //no switch
        for (size_t index = 0; index < fifo_len; index++)
            spi_handle->dr[0] = tx_buff[i++];

        tx_len -= fifo_len;
    }
    while ((spi_handle->sr & 0x05) != 0x04)
        ;

    i = 0;
    while (rx_len)
    {
        fifo_len = spi_handle->rxflr;
        fifo_len = fifo_len < rx_len ? fifo_len : rx_len;

        //no switch
        for (size_t index = 0; index < fifo_len; index++)
            rx_buff[i++] = (uint8_t)spi_handle->dr[0];

        rx_len -= fifo_len;
    }
    spi_handle->ser = 0x00;
    spi_handle->ssienr = 0x00;

    return;
}
