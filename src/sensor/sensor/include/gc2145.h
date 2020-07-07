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
#ifndef __GC2145_H
#define __GC2145_H

#include <stdint.h>

/**
 * GC2145 Device Address:
 * serial bus write address = 0x78, serial bus read address = 0x79
 * 
 */
#define GC2145_ADDR 0x78

int gc2145_init(void);
uint8_t gc2145_detect(void);

#endif /* __GC2145_H */
