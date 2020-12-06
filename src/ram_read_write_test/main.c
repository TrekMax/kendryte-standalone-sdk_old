/*
 * @Author: your name
 * @Date: 2020-12-06 14:07:06
 * @LastEditTime: 2020-12-06 16:27:59
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /kendryte-standalone-sdk_old/src/flash_w25qxx_test/main.c
 */
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
#include "fpioa.h"
#include "sysctl.h"
#include "ram.h"
#include "uarths.h"

int main(void)
{
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    uarths_init();
    printf("============= RAM SPEED TEST ===============\r\n");
    RamTest();
    printf("============= RAM SPEED TEST END ===============\r\n");

    while (1)
        ;
    return 0;
}
