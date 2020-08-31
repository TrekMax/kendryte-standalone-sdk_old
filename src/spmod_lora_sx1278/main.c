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
#include "gpiohs.h"
#include "uarths.h"

#include "sx1278.h"

static const char *TAG = "main";

// sx1278_t sx1278_module;
sx1278_hw_t SX1278_hw;
sx1278_t SX1278;

int master;
int ret;

char buffer[64];

int message;
int message_length;


void lora_test(void);

int main(void)
{
    uint32_t freq = 0;
    freq = sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uint64_t core = current_coreid();
    printk(LOG_COLOR_W "-------------------------------\r\n");
    printk(LOG_COLOR_W "Sipeed@QinYUN575\r\n");
    printk(LOG_COLOR_W "Compile@ %s %s\r\n", __DATE__, __TIME__);
    printk(LOG_COLOR_W "Board: " LOG_COLOR_E BOARD_NAME "\r\n");
    printk(LOG_COLOR_W "pll freq: %dhz\r\n", freq);
    printk(LOG_COLOR_W "-------------------------------\r\n");

    lora_test();

    printk(LOG_COLOR_W "-------------END---------------\r\n");
    return 0;
}
void lora_test(void)
{
    master = 0;
    LOGI(TAG, "Lora demo");
    sx1278_begin(&SX1278, SX1278_433MHZ, SX1278_POWER_17DBM, SX1278_LORA_SF_8,
                 SX1278_LORA_BW_20_8KHZ, 10);

    if (master == 1)
    {
        ret = sx1278_LoRaEntryTx(&SX1278, 16, 2000);
    }
    else
    {
        ret = sx1278_LoRaEntryRx(&SX1278, 16, 2000);
    }

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        if (master == 1)
        {
            LOGD(TAG, "Master ...");
            msleep(2500);
            LOGD(TAG, "Sending package...");

            message_length = sprintf(buffer, "Hello %d", message);
            ret = sx1278_LoRaEntryTx(&SX1278, message_length, 2000);
            LOGD(TAG, "Entry: %d", ret);

            LOGI(TAG, "Sending %s", buffer);
            ret = sx1278_LoRaTxPacket(&SX1278, (uint8_t *)buffer, message_length,
                                      2000);
            message += 1;

            LOGD(TAG, "Transmission: %d", ret);
            LOGD(TAG, "Package sent...");
        }
        else
        {
            LOGD(TAG, "Slave ...");
            msleep(1000);
            LOGI(TAG, "Receiving package...");

            ret = sx1278_LoRaRxPacket(&SX1278);
            LOGI(TAG, "Received: %d", ret);
            if (ret > 0)
            {
                sx1278_read(&SX1278, (uint8_t *)buffer, ret);
                LOGD(TAG, "Content (%d): %s", ret, buffer);
            }
            LOGD(TAG, "Package received ...");
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    LOGE(TAG, "Lora Error");
}
