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

#include "demo.h"
#include "epd.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "uarths.h"

static const char *TAG = "main";
void epd_test(void)
{

    EPD_display_init();                      //EPD init
    EPD_full_display(gImage_1, gImage_1, 0); //pic1
    EPD_deep_sleep();                        //Enter deep sleep mode,This step is a must
    msleep(1000);

    EPD_display_init();                      //EPD init
    EPD_full_display(gImage_1, gImage_2, 1); //pic2
    EPD_deep_sleep();                        //Enter deep sleep mode,This step is a must
    msleep(1000);

    EPD_display_init();                      //EPD init
    EPD_full_display(gImage_2, gImage_3, 1); //pic3
    EPD_deep_sleep();                        //Enter deep sleep mode,This step is a must
    msleep(1000);

    /******End screen clearing*****/
    EPD_display_init();                      //EPD init
    EPD_full_display(gImage_3, gImage_3, 2); //End screen clearing

    //(Perform a full-screen refresh operation after 5 partial refreshes, otherwise the residual image may not be removed.)
    //////////////////////////partial display 0~9////////////////////////////////////////////

    partial_display(0, 32, 0, 64, gImage_num1, gImage_num1, 0); //partial display 0 1
    partial_display(0, 32, 0, 64, gImage_num1, gImage_num2, 1); //partial display 1 2                         //////////////��x,y��   ----Y----
    partial_display(0, 32, 0, 64, gImage_num2, gImage_num3, 1); //partial display 1 2                         //////////////��x,y��   ----Y----
    partial_display(0, 32, 0, 64, gImage_num3, gImage_num4, 1); //partial display 1 2                         //////////////��x,y��   ----Y----
    partial_display(0, 32, 0, 64, gImage_num4, gImage_num5, 1); //partial display 1 2                         //////////////��x,y��   ----Y----

    /******Clear screen after Partial refresh*****/
    EPD_display_init();          //EPD init
    EPD_full_display_clearing(); //Clear screen after power on
    EPD_deep_sleep();            //Enter deep sleep mode,This step is a must
}
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

    epd_test();

    printk(LOG_COLOR_W "-------------END---------------\r\n");
    while(1)
        ;
    return 0;
}
