/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS},
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include "dvp.h"
#include "fpioa.h"
#include "gc2145.h"
// #include "gc2145cfg.h"
#include "gc2145_regs-v1.h"
// #include "gc2145cfg-NG-v1.h"
#include "i2c.h"
#include "plic.h"
#include "sleep.h"

#include "board_config.h"
#include "syslog.h"

static const char *TAG = "gc2145";

void gc2145_i2c_master_init(int num) //1 650, 0 850
{
    if(num)
    {
        fpioa_set_function(40, FUNC_I2C1_SDA); //650
        fpioa_set_function(41, FUNC_I2C1_SDA);
    } else
    {
        fpioa_set_function(40, FUNC_I2C0_SDA); //850
        fpioa_set_function(41, FUNC_I2C0_SCLK);
    }
    i2c_init(num, GC2145_ADDR >> 1, 7, 100000);
}

static void gc2145_wr_reg(uint8_t num, uint8_t reg, uint8_t data)
{
    uint8_t buf[2];

    buf[0] = reg & 0xff;
    buf[1] = data;
    i2c_send_data(num, buf, 2);
}

static uint8_t gc2145_rd_reg(uint8_t num, uint8_t reg)
{
    uint8_t reg_buf[1];
    uint8_t data_buf;

    reg_buf[0] = reg & 0xff;
    i2c_recv_data(num, reg_buf, 1, &data_buf, 1);
    return data_buf;
}

int gc2145_read_id(uint8_t num, uint16_t *id)
{
    uint8_t id_h, id_l;
    id_h = gc2145_rd_reg(num, 0xf0);
    id_l = gc2145_rd_reg(num, 0xf1);
    *id = (id_h<<8) | id_l;
    return 0;
}

// #define sensor_default_regs gc2145_sensor_default_regs
#define sensor_default_regs sensor_gc2145_default_regs

int gc2145_init(void)
{
    uint32_t i;

    uint8_t num = 0, data;
    uint16_t sensor_id; 
 
    gc2145_i2c_master_init(num);
    gc2145_read_id(num, &sensor_id);
    LOGI(TAG, "ID%d: 0x%X", num, sensor_id);

    // printf("-----------------------");
    for(i = 0; sensor_default_regs[i][0]; i++)
    {
        gc2145_wr_reg(num, sensor_default_regs[i][0], sensor_default_regs[i][1]);
        // data = gc2145_rd_reg(num, sensor_default_regs[i][0]);
        
        // if (i %10 == 0){
        //     printf("\r\n%5d:  ", (i/10));
        // }
        // printf("%02X  ", data);
    }
    // printf("-----------------------");
    for(i = 0; sensor_gc2145_qvga_config[i][0]; i++)
    {
        gc2145_wr_reg(num, sensor_gc2145_qvga_config[i][0], sensor_gc2145_qvga_config[i][1]);

    }
    return 0;
}
