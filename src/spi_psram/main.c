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
#include "psram.h"

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
    fpioa_set_function(SPI_PSRAM_CS_PIN_NUM,    FUNC_SPI1_SS0);   // CS
    fpioa_set_function(SPI_PSRAM_SCK_PIN_NUM,   FUNC_SPI1_SCLK); // SCLK
    fpioa_set_function(SPI_PSRAM_MOSI_PIN_NUM,  FUNC_SPI1_D0);  // MOSI
    fpioa_set_function(SPI_PSRAM_MISO_PIN_NUM,  FUNC_SPI1_D1);  // MISO

    fpioa_set_function(SPI_PSRAM_D2_PIN_NUM,    FUNC_SPI1_D2); // D2
    fpioa_set_function(SPI_PSRAM_D3_PIN_NUM,    FUNC_SPI1_D3); // D3
}

int spi_flash_psram_test(void)
{

    LOGI(TAG, "spi%d master test", SPI_INDEX);
    spi_io_mux_init();

    psram_init(SPI_INDEX, SPI_CHIP_SELECT_NSS);
   
    
    uint64_t device_id;
    psram_read_id(&device_id);
    if (!PSRAM_IS_VALID(device_id)) {
        LOGE(TAG, "PSRAM_IS_INVALID");
        return -1;
    }
    else
    {
        LOGI(TAG, "PSRAM IS VALID!");
        switch (PSRAM_SIZE_ID(device_id))
        {
        case PSRAM_EID_SIZE_16MBITS://2MB
            LOGI(TAG, "PSRAM_EID_SIZE_16MBITS");
            break;

        case PSRAM_EID_SIZE_32MBITS://4MB
            LOGI(TAG, "PSRAM_EID_SIZE_32MBITS");
            break;

        case PSRAM_EID_SIZE_64MBITS://8MB
            LOGI(TAG, "PSRAM_EID_SIZE_64MBITS");
            break;
        
        default:
            break;
        }
    }
    psram_enable_quad_mode();
    psram_read_id(&device_id);
    if (!PSRAM_IS_VALID(device_id)) {
        LOGE(TAG, "PSRAM_IS_INVALID");
        return -1;
    }
    
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
    
    spi_flash_psram_test();

    printk(LOG_COLOR_W "-------------END---------------\r\n");
    return 0;
}
