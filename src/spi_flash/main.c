/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <bsp.h>
#include <stdio.h>
#include <sysctl.h>
#include "board_config.h"
#include "syslog.h"

#include "fpioa.h"
#include "uarths.h"
#include "w25qxx.h"

#define TEST_NUMBER (40 * 1024 + 5)
#define DATA_ADDRESS 0xB00000

uint8_t data_buf[TEST_NUMBER];

static const char *TAG = "main";

int core1_function(void *ctx)
{
    uint64_t core = current_coreid();
    LOGI(TAG, "Core %ld Hello world", core);
    while(1)
        ;
}

void spi_io_mux_init(void)
{
    fpioa_set_function(SPI_FALSH_CS_PIN_NUM,    FUNC_SPI1_SS0);   // CS
    fpioa_set_function(SPI_FALSH_SCK_PIN_NUM,   FUNC_SPI1_SCLK); // SCLK
    fpioa_set_function(SPI_FALSH_MOSI_PIN_NUM,  FUNC_SPI1_D0);  // MOSI
    fpioa_set_function(SPI_FALSH_MISO_PIN_NUM,  FUNC_SPI1_D1);  // MISO

    fpioa_set_function(SPI_FALSH_D2_PIN_NUM,    FUNC_SPI1_D2); // D2
    fpioa_set_function(SPI_FALSH_D3_PIN_NUM,    FUNC_SPI1_D3); // D3
}

int spi_flash_w25qxx_test(void)
{
    uint8_t manuf_id, device_id;
    uint32_t index;

    LOGI(TAG, "spi%d master test", SPI_INDEX);
    spi_io_mux_init();

    w25qxx_init(SPI_INDEX, SPI_CHIP_SELECT_NSS);

    w25qxx_read_id(&manuf_id, &device_id);
    LOGI(TAG, "manuf_id:0x%02X, device_id:0x%02X", manuf_id, device_id);
    if((manuf_id != 0xEF && manuf_id != 0xC8 && manuf_id != 0x0B) || (device_id != 0x17 && device_id != 0x16))
    {
        LOGI(TAG, "manuf_id:0x%02X, device_id:0x%02X", manuf_id, device_id);
        return 0;
    }

    /*write data*/
    for(index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = (uint8_t)(index);
    LOGI(TAG, "erase sector");
    w25qxx_sector_erase(DATA_ADDRESS);
    while(w25qxx_is_busy() == W25QXX_BUSY)
        ;
    LOGI(TAG, "write data");
    w25qxx_write_data_direct(DATA_ADDRESS, data_buf, TEST_NUMBER);

    /* standard read test*/
    for(index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    LOGI(TAG, "standard read test start");
    w25qxx_read_data(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_STANDARD);
    for(index = 0; index < TEST_NUMBER; index++)
    {
        if(data_buf[index] != (uint8_t)(index))
        {
            LOGI(TAG, "standard read test error");
            return 0;
        }
    }

    /*standard fast read test*/
    for(index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    LOGI(TAG, "standard fast read test start");
    /*w25qxx_read_data(0, data_buf, TEST_NUMBER, W25QXX_STANDARD_FAST);*/
    w25qxx_read_data(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_STANDARD_FAST);
    for(index = 0; index < TEST_NUMBER; index++)
    {
        if(data_buf[index] != (uint8_t)index)
        {
            LOGI(TAG, "standard fast read test error");
            return 0;
        }
    }

    /*dual read test*/
    for(index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    LOGI(TAG, "dual read test start");
    w25qxx_read_data(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_DUAL);
    for(index = 0; index < TEST_NUMBER; index++)
    {
        if(data_buf[index] != (uint8_t)index)
        {
            LOGI(TAG, "dual read test error");
            return 0;
        }
    }

    w25qxx_enable_quad_mode();

    /*quad read test*/
    for(index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    LOGI(TAG, "quad read test start");
    w25qxx_read_data(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_QUAD);
    for(index = 0; index < TEST_NUMBER; index++)
    {
        if(data_buf[index] != (uint8_t)(index))
        {
            LOGI(TAG, "quad read test error");
            return 0;
        }
    }

    /*dual fast read test*/
    for(index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    LOGI(TAG, "dual fast read test start");
    w25qxx_read_data(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_DUAL_FAST);
    for(index = 0; index < TEST_NUMBER; index++)
    {
        if(data_buf[index] != (uint8_t)index)
        {
            LOGI(TAG, "dual fast read test error");
            return 0;
        }
    }

    /*quad fast read test*/
    for(index = 0; index < TEST_NUMBER; index++)
        data_buf[index] = 0;
    LOGI(TAG, "quad fast read test start");
    w25qxx_read_data(DATA_ADDRESS, data_buf, TEST_NUMBER, W25QXX_QUAD_FAST);
    for(index = 0; index < TEST_NUMBER; index++)
    {
        if(data_buf[index] != (uint8_t)(index))
        {
            LOGI(TAG, "quad fast read test error");
            return 0;
        }
    }

    LOGI(TAG, "spi%d master test ok", SPI_INDEX);
}

int main(void)
{
    uint32_t freq = 0;
    freq = sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uint64_t core = current_coreid();
    int data;
    printk(LOG_COLOR_W "-------------------------------\r\n");
    printk(LOG_COLOR_W "Sipeed@QinYUN575\r\n");
    printk(LOG_COLOR_W "Compile@ %s %s\r\n", __DATE__, __TIME__);
    printk(LOG_COLOR_W "Board: " LOG_COLOR_E BOARD_NAME "\r\n");
    printk(LOG_COLOR_W "pll freq: %dhz\r\n", freq);
    printk(LOG_COLOR_W "-------------------------------\r\n");

    // LOGI(TAG, "Core %ld Hello world", core);
    // register_core1(core1_function, NULL);

    spi_flash_w25qxx_test();

    printk(LOG_COLOR_W "-------------END---------------\r\n");
    return 0;
}
