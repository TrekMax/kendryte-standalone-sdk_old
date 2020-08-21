SPI_FLASH
======

SPI ESP_PSRAM



- output log
```shell
-------------------------------
Sipeed@QinYUN575
Compile@ Aug 21 2020 15:48:04
Board: BOARD_MAIX_M1N
pll freq: 806000000hz
-------------------------------
I (14017825) main: spi1 master test
I (15709915) PSRAM: spi clk is 31000000
I (17515895) PSRAM: [68|psram_disable_quad_mode]
I (19651447) PSRAM: [81|psram_disable_quad_mode]OK
-----------------------
    0:  0D      5D      46      A0      B9      AA      BA      38
-----------------------
I (24791990) PSRAM: device_id: 0x38BAAAB9A0465D0D
I (26960449) main: PSRAM IS VALID!
I (28604512) main: PSRAM_EID_SIZE_64MBITS
I (30493416) PSRAM: [86|psram_enable_quad_mode]
I (32607258) PSRAM: [68|psram_disable_quad_mode]
I (34727530) PSRAM: [81|psram_disable_quad_mode]OK
-----------------------
    0:  0D      5D      46      A0      B9      AA      BA      38
-----------------------
I (39868398) PSRAM: device_id: 0x38BAAAB9A0465D0D
-------------END---------------
W (43443400) SYSCALL: sys_exit called by core 0 with 0x1
```