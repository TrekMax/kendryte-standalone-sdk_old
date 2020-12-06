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
#include <string.h>
#include "sysctl.h"
#include "ram.h"
// Profile fun! microseconds to seconds
// double GetTime() { return (double)esp_timer_get_time() / 1000000; }
#include <stdlib.h>
#include "sleep.h"
#include "time.h"

double GetTime() { return (double)sysctl_get_time_us() / 1000000; }

int RamTest(void)
{
    int rs[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4000};
    printf("Ram Speed Test!\r\n");
    char xx = 0;
    for(int a = 0; a < 13; a++)
    {
        printf("Read Speed 8bit ArraySize %4dkb ", rs[a]);
        int ramsize = rs[a] * 1024;
        char *rm = (char *)malloc(ramsize);

        int iters = 10; // Just enuff to boot the dog
        if(rs[a] < 512)
            iters = 50;
        double st = GetTime();
        for(int b = 0; b < iters; b++)
            for(int c = 0; c < ramsize; c++)
                xx |= rm[c];
        st = GetTime() - st;
        sleep(1); // Dog it!
        double speed = ((double)(iters * ramsize) / (1024 * 1024)) / (st);
        printf(" time: %2.1f %2.1f mb/sec  \r\n", st, speed);
        free(rm);
    }
    printf("\n");
    for(int a = 0; a < 13; a++)
    {
        printf("Read Speed 16bit ArraySize %4dkb ", rs[a]);
        int ramsize = rs[a] * 1024;
        short *rm = (short *)malloc(ramsize);

        int iters = 10; // Just enuff to boot the dog
        if(rs[a] < 512)
            iters = 50;
        double st = GetTime();
        for(int b = 0; b < iters; b++)
            for(int c = 0; c < ramsize / 2; c++)
                xx |= rm[c];
        st = GetTime() - st;
        sleep(1); // Dog it!
        double speed = ((double)(iters * ramsize) / (1024 * 1024)) / (st);
        printf(" time: %2.1f %2.1f mb/sec  \r\n", st, speed);
        free(rm);
    }
    printf("\n");
    for(int a = 0; a < 13; a++)
    {
        printf("Read Speed 32bit ArraySize %4dkb ", rs[a]);
        int ramsize = rs[a] * 1024;
        int *rm = (int *)malloc(ramsize);

        int iters = 10; // Just enuff to boot the dog
        if(rs[a] < 512)
            iters = 50;
        double st = GetTime();
        for(int b = 0; b < iters; b++)
            for(int c = 0; c < ramsize / 4; c++)
                xx |= rm[c];
        st = GetTime() - st;
        sleep(1); // Dog it!
        double speed = ((double)(iters * ramsize) / (1024 * 1024)) / (st);
        printf(" time: %2.1f %2.1f mb/sec  \r\n", st, speed);
        free(rm);
    }
    printf("\n");
    for(int a = 0; a < 13; a++)
    {
        printf("Read Speed 64bit ArraySize %4dkb ", rs[a]);
        int ramsize = rs[a] * 1024;
        int *rm = (int *)malloc(ramsize);

        int iters = 10; // Just enuff to boot the dog
        if(rs[a] < 512)
            iters = 50;
        double st = GetTime();
        for(int b = 0; b < iters; b++)
            for(int c = 0; c < ramsize / 8; c++)
                xx |= rm[c];
        st = GetTime() - st;
        sleep(1); // Dog it!
        double speed = ((double)(iters * ramsize) / (1024 * 1024)) / (st);
        printf(" time: %2.1f %2.1f mb/sec  \r\n", st, speed);
        free(rm);
    }
    printf("Test done!\n");
    return xx;
}
