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
#ifndef _PSRAM_H
#define _PSRAM_H

#include <stdint.h>

/* clang-format off */
#define DATALENGTH                          8

#define SPI_SLAVE_SELECT                    (0x01)

//Commands for PSRAM chip
#define PSRAM_READ                 0x03
#define PSRAM_FAST_READ            0x0B
#define PSRAM_FAST_READ_DUMMY      0x3
#define PSRAM_FAST_READ_QUAD       0xEB
#define PSRAM_FAST_READ_QUAD_DUMMY 0x5
#define PSRAM_WRITE                0x02
#define PSRAM_QUAD_WRITE           0x38
#define PSRAM_ENTER_QMODE          0x35
#define PSRAM_EXIT_QMODE           0xF5
#define PSRAM_RESET_EN             0x66
#define PSRAM_RESET                0x99
#define PSRAM_SET_BURST_LEN        0xC0
#define PSRAM_DEVICE_ID            0x9F

typedef enum {
    PSRAM_CLK_MODE_NORM = 0,    /*!< Normal SPI mode */
    PSRAM_CLK_MODE_DCLK = 1,    /*!< Two extra clock cycles after CS is set high level */
} psram_clk_mode_t;

#define PSRAM_ID_KGD_M          0xff
#define PSRAM_ID_KGD_S             8
#define PSRAM_ID_KGD            0x5d
#define PSRAM_ID_EID_M          0xff
#define PSRAM_ID_EID_S            16

// Use the [7:5](bit7~bit5) of EID to distinguish the psram size:
//
//   BIT7  |  BIT6  |  BIT5  |  SIZE(MBIT)
//   -------------------------------------
//    0    |   0    |   0    |     16
//    0    |   0    |   1    |     32
//    0    |   1    |   0    |     64
#define PSRAM_EID_SIZE_M         0x07
#define PSRAM_EID_SIZE_S            5

typedef enum {
    PSRAM_EID_SIZE_16MBITS = 0,
    PSRAM_EID_SIZE_32MBITS = 1,
    PSRAM_EID_SIZE_64MBITS = 2,
} psram_eid_size_t;

#define PSRAM_KGD(id)         (((id) >> PSRAM_ID_KGD_S) & PSRAM_ID_KGD_M)
#define PSRAM_EID(id)         (((id) >> PSRAM_ID_EID_S) & PSRAM_ID_EID_M)
#define PSRAM_SIZE_ID(id)     ((PSRAM_EID(id) >> PSRAM_EID_SIZE_S) & PSRAM_EID_SIZE_M)
#define PSRAM_IS_VALID(id)    (PSRAM_KGD(id) == PSRAM_ID_KGD)

// For the old version 32Mbit psram, using the spicial driver */
#define PSRAM_IS_32MBIT_VER0(id)  (PSRAM_EID(id) == 0x20)
#define PSRAM_IS_64MBIT_TRIAL(id) (PSRAM_EID(id) == 0x26)

/* clang-format on */

/**
 * @brief      psram operating status enumerate
 */
typedef enum _psram_status
{
    PSRAM_OK = 0,
    PSRAM_BUSY,
    PSRAM_ERROR,
} psram_status_t;

/**
 * @brief      psram read operating enumerate
 */
typedef enum _psram_read
{
    PSRAM_STANDARD = 0,
    PSRAM_STANDARD_FAST,
    PSRAM_DUAL,
    PSRAM_DUAL_FAST,
    PSRAM_QUAD,
    PSRAM_QUAD_FAST,
} psram_read_t;

typedef struct {
    uint16_t cmd;                /*!< Command value */
    uint16_t cmdBitLen;          /*!< Command byte length*/
    uint32_t *addr;              /*!< Point to address value*/
    uint16_t addrBitLen;         /*!< Address byte length*/
    uint32_t *txData;            /*!< Point to send data buffer*/
    uint16_t txDataBitLen;       /*!< Send data byte length.*/
    uint32_t *rxData;            /*!< Point to recevie data buffer*/
    uint16_t rxDataBitLen;       /*!< Recevie Data byte length.*/
    uint32_t dummyBitLen;
} psram_cmd_t;


psram_status_t psram_init(uint8_t spi_index, uint8_t spi_ss);
psram_status_t psram_read_id(uint64_t *device_id);

void psram_disable_quad_mode(void);
void psram_enable_quad_mode(void);

#endif

