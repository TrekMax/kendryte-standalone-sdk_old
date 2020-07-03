#include <stdio.h>
#include "bsp.h"
#include "dmac.h"
#include "ff.h"
#include "fpioa.h"
#include "i2s.h"
#include "plic.h"
#include "sdcard.h"
#include "sysctl.h"
#include "uarths.h"

#include "board_config.h"
#include "syslog.h"

static const char *TAG = "main";

static int sdcard_test(void);
static int fs_test(void);
FRESULT sd_write_test(TCHAR *path);

void sdcard_io_mux_init(void)
{
    fpioa_set_function(SD_SPI_SCLK, FUNC_SPI0_SCLK);
    fpioa_set_function(SD_SPI_MOSI, FUNC_SPI0_D0);
    fpioa_set_function(SD_SPI_MISO, FUNC_SPI0_D1);
    fpioa_set_function(SD_SPI_CS_PIN,FUNC_GPIOHS0 + SD_SPI_CS_PIN);
}

int main(void)
{
    uint32_t freq = 0;
    freq = sysctl_pll_set_freq(SYSCTL_PLL0, 800000000);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    uint64_t core = current_coreid();
    printk(LOG_COLOR_W "-------------------------------\r\n");
    printk(LOG_COLOR_W "Sipeed@QinYUN575\r\n");
    printk(LOG_COLOR_W "Compile@ %s %s\r\n", __DATE__, __TIME__);
    printk(LOG_COLOR_W "Board: " LOG_COLOR_E BOARD_NAME "\r\n");
    printk(LOG_COLOR_W "pll freq: %dhz\r\n", freq);
    printk(LOG_COLOR_W "-------------------------------\r\n");

    int data;
    sdcard_io_mux_init();
    dmac_init();
    plic_init();
    sysctl_enable_irq();

    if(sdcard_test())
    {
        LOGI(TAG, "SD card err");
        return -1;
    }
    if(fs_test())
    {
        LOGI(TAG, "FAT32 err");
        return -1;
    }
    if(sd_write_test(_T("0:test.txt")))
    {
        LOGI(TAG, "SD write err");
        return -1;
    }
    return 0;
}

static int sdcard_test(void)
{
    uint8_t status;

    LOGI(TAG, "/******************sdcard test*****************/");
    status = sd_init();
    LOGI(TAG, "sd init %d", status);
    if(status != 0)
    {
        return status;
    }

    LOGI(TAG, "card info status %d", status);
    LOGI(TAG, "CardCapacity:%ld", cardinfo.CardCapacity);
    LOGI(TAG, "CardBlockSize:%d", cardinfo.CardBlockSize);
    return 0;
}

static int fs_test(void)
{
    static FATFS sdcard_fs;
    FRESULT status;
    DIR dj;
    FILINFO fno;

    LOGI(TAG, "/********************fs test*******************/");
    status = f_mount(&sdcard_fs, _T("0:"), 1);
    LOGI(TAG, "mount sdcard:%d", status);
    if(status != FR_OK)
        return status;

    LOGI(TAG, "printf filename");
    status = f_findfirst(&dj, &fno, _T("0:"), _T("*"));
    while(status == FR_OK && fno.fname[0])
    {
        if(fno.fattrib & AM_DIR)
            LOGI(TAG, "dir:%s", fno.fname);
        else
            LOGI(TAG, "file:%s", fno.fname);
        status = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);
    return 0;
}

FRESULT sd_write_test(TCHAR *path)
{
    FIL file;
    FRESULT ret = FR_OK;
    LOGI(TAG, "/*******************sd write test*******************/");
    uint32_t v_ret_len = 0;

    FILINFO v_fileinfo;
    if((ret = f_stat(path, &v_fileinfo)) == FR_OK)
    {
        LOGI(TAG, "%s length is %lld", path, v_fileinfo.fsize);
    } else
    {
        LOGI(TAG, "%s fstat err [%d]", path, ret);
    }

    if((ret = f_open(&file, path, FA_READ)) == FR_OK)
    {
        char v_buf[64] = {0};
        ret = f_read(&file, (void *)v_buf, 64, &v_ret_len);
        if(ret != FR_OK)
        {
            LOGI(TAG, "Read %s err[%d]", path, ret);
        } else
        {
            LOGI(TAG, "Read :> %s %d bytes lenth", v_buf, v_ret_len);
        }
        f_close(&file);
    }

    if((ret = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
    {
        LOGI(TAG, "open file %s err[%d]", path, ret);
        return ret;
    } else
    {
        LOGI(TAG, "Open %s ok", path);
    }
    uint8_t hello[1024];
    uint32_t i;
    for(i = 0; i < 1024; i++)
    {
        hello[i] = 'A';
    }
    ret = f_write(&file, hello, sizeof(hello), &v_ret_len);
    if(ret != FR_OK)
    {
        LOGI(TAG, "Write %s err[%d]", path, ret);
    } else
    {
        LOGI(TAG, "Write %d bytes to %s ok", v_ret_len, path);
    }
    f_close(&file);
    return ret;
}
