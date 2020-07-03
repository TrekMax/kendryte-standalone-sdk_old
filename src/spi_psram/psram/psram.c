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
#include <stdio.h>
#include "dmac.h"
#include "fpioa.h"
#include "psram.h"
#include "spi.h"
#include "sysctl.h"
#include "syslog.h"

#include "board_config.h"
#include "utils_dump.h"

static const char *TAG = "PSRAM";

uint32_t spi_bus_no = 0;
uint32_t spi_chip_select = 0;

static psram_status_t psram_receive_data(uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *rx_buff, uint32_t rx_len)
{
    spi_init(spi_bus_no, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    spi_receive_data_standard(spi_bus_no, spi_chip_select, cmd_buff, cmd_len, rx_buff, rx_len);
    return PSRAM_OK;
}

static psram_status_t psram_quad_send_data(uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len)
{
    spi_init(spi_bus_no, SPI_WORK_MODE_0, SPI_FF_QUAD, 32, 0);
    spi_init_non_standard(spi_bus_no, 8 /*instrction length*/, 32 /*address length*/, 0 /*wait cycles*/,
                          SPI_AITM_STANDARD /*spi address trans mode*/);
    spi_send_data_multiple(spi_bus_no, spi_chip_select, cmd_buff, cmd_len, tx_buff, tx_len);
    return PSRAM_OK;
}

static psram_status_t psram_send_data(uint8_t *cmd_buff, uint8_t cmd_len, uint8_t *tx_buff, uint32_t tx_len)
{
    spi_init(spi_bus_no, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    spi_send_data_standard(spi_bus_no, spi_chip_select, cmd_buff, cmd_len, tx_buff, tx_len);
    return PSRAM_OK;
}

psram_status_t psram_init(uint8_t spi_index, uint8_t spi_ss)
{
    spi_bus_no = spi_index;
    spi_chip_select = spi_ss;

    spi_init(spi_bus_no, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
    uint32_t ret = spi_set_clk_rate(spi_bus_no,  10*1000*1000);//10Mhz
    LOGI(TAG, "spi clk is %d", ret);
    return PSRAM_OK;
}
//exit QPI mode(set back to SPI mode)
void psram_disable_quad_mode(void)
{
    LOGI(TAG, "[%d|%s]", __LINE__, __FUNCTION__);
    uint32_t cmd[2];
    cmd[0] = PSRAM_EXIT_QMODE;
    cmd[1] = 0;

    spi_init(spi_bus_no, SPI_WORK_MODE_0, SPI_FF_QUAD, 8, 0);
    spi_init_non_standard(spi_bus_no, 8,
        24 /*address length*/,
        0 /*wait cycles*/,
        SPI_AITM_AS_FRAME_FORMAT /*spi address trans mode*/);

    // spi_send_data_multiple(spi_bus_no, spi_chip_select, NULL, 0, cmd, sizeof(cmd));
    spi_send_data_multiple(spi_bus_no, spi_chip_select, cmd, sizeof(cmd), NULL, 0);
    LOGI(TAG, "[%d|%s]OK", __LINE__, __FUNCTION__);
}

void psram_enable_quad_mode(void)
{
    LOGI(TAG, "[%d|%s]", __LINE__, __FUNCTION__);
    uint32_t cmd[1];
    cmd[0] = PSRAM_ENTER_QMODE;//小端模式
    spi_init(spi_bus_no, SPI_WORK_MODE_0, SPI_FF_STANDARD, 8, 0);
    spi_send_data_standard(spi_bus_no, spi_chip_select, cmd, 1, NULL, 0);

}

psram_status_t psram_read_id(uint64_t *device_id)
{
    uint32_t psram_id[2] = {0};
    uint8_t cmd[4];
    psram_disable_quad_mode();
    
    cmd[0] = PSRAM_DEVICE_ID;

    psram_receive_data(cmd, sizeof(cmd), psram_id, sizeof(psram_id));
    *device_id = (uint64_t)(((uint64_t)psram_id[1] << 32) | psram_id[0]);

    dump(psram_id, sizeof(psram_id));
    LOGI(TAG, "device_id: 0x%X%X", psram_id[1], psram_id[0]);

    return PSRAM_OK;
}
