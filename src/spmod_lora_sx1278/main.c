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

sx1278_hw_t SX1278_hw;
sx1278_t SX1278;

int ret;

char buffer[64];

int message;
int message_length;


void lora_test(uint8_t master);
#define PIN_KEY 16
#define GPIO_KEY 0

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

    fpioa_set_function(PIN_KEY, FUNC_GPIOHS0);
    gpiohs_set_drive_mode(GPIO_KEY, GPIO_DM_INPUT_PULL_DOWN);
    // gpiohs_set_pin_edge(GPIO_KEY, GPIO_PE_FALLING);

    msleep(1500);
    uint8_t value = gpiohs_get_pin(GPIO_KEY);
    printf("------>   PIN_KEY[%d:%d]\r\n", GPIO_KEY, value);
    if (value)
    {
        lora_test(1);
    }
    else
    {
        lora_test(0);
    }


    printk(LOG_COLOR_W "-------------END---------------\r\n");
    return 0;
}
void lora_test(uint8_t master)
{
    LOGI(TAG, "Lora demo");
    sx1278_begin(&SX1278, SX1278_433MHZ, SX1278_POWER_17DBM, SX1278_LORA_SF_8,
                 SX1278_LORA_BW_20_8KHZ, 10);

    if (master == 1)
    {
        printf("====MASTER====\r\n");
        ret = sx1278_LoRaEntryTx(&SX1278, 16, 2000);
    }
    else
    {
        printf("====SALAVE====\r\n");
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

            message_length = sprintf(buffer, "Hello %d", message);
            ret = sx1278_LoRaEntryTx(&SX1278, message_length, 2000);
            LOGD(TAG, "Sending package[Entry: %d]...", ret);

            LOGI(TAG, "Sending %s", buffer);
            ret = sx1278_LoRaTxPacket(&SX1278, (uint8_t *)buffer, message_length,
                                      2000);
            message += 1;

            LOGD(TAG, "Package sent[Transmission: %d]...", ret);
        }
        else
        {
            LOGD(TAG, "Slave ...");
            msleep(1000);

            ret = sx1278_LoRaRxPacket(&SX1278);
            LOGD(TAG, "Receiving package[Received: %d]...", ret);
            if (ret > 0)
            {
                sx1278_read(&SX1278, (uint8_t *)buffer, ret);
                LOGI(TAG, "Content (%d): " LOG_RESET_COLOR LOG_COLOR_BROWN "\r\n%s" LOG_RESET_COLOR "\r\n", ret, buffer);
            }
            LOGD(TAG, "Package received ...");
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    LOGE(TAG, "Lora Error");
}
