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
#include <sysctl.h>
#include "syslog.h"
#include "board_config.h"

static const char *TAG = "main";

int core1_function(void *ctx)
{
    uint64_t core = current_coreid();
    LOGI(TAG, "Core %ld Hello world", core);
    while(1);
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

    LOGI(TAG, "Core %ld Hello world", core);
    register_core1(core1_function, NULL);

    /* Clear stdin buffer before scanf */
    sys_stdin_flush();

    scanf("%d", &data);
    LOGI(TAG, "\r\nData is %d", data);
    printk(LOG_COLOR_W "-------------END---------------\r\n");
    while(1)
    {
         LOGI(TAG, "Hello K210!");
         msleep(1000);
    }
    return 0;
}
