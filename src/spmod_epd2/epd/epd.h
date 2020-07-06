#ifndef __EPD_H__
#define __EPD_H__
#include "syslog.h"

void EPD_display_init(void);
void EPD_full_display(const unsigned char *old_data, const unsigned char *new_data, unsigned char mode); // mode0:Refresh picture1,mode1:Refresh picture2... ,mode2:Clear
void EPD_full_display_clearing(void);
void EPD_deep_sleep(void);
void partial_display(uint16_t x_start, uint16_t x_end, 
    uint16_t y_start, uint16_t y_end, 
    const unsigned char *old_data, const unsigned char *new_data, unsigned char mode); //partial display

#endif
