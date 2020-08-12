#include <stdio.h>

#include "board_config.h"

#include "sysctl.h"
#include "fpioa.h"
#include "plic.h"
#include "dmac.h"
#include "i2s.h"
#include "uarths.h"
#include "ff.h"
#include "bsp.h"

#include "syslog.h"

#include "sdcard.h"
#include "wav_decode.h"

#include "es8374.h"


static int sdcard_test(void);
static int fs_test(void);
static int wav_test(TCHAR *path);
FRESULT sd_write_test(TCHAR *path);

static const char *TAG = "main";

void io_mux_init(void)
{
    LOGI(TAG, "%s", __FUNCTION__);
    // fpioa_set_function(27, FUNC_SPI0_SCLK);
    // fpioa_set_function(28, FUNC_SPI0_D0);
    // fpioa_set_function(26, FUNC_SPI0_D1);
    // fpioa_set_function(29, FUNC_GPIOHS0 + SD_CS_PIN);
    
    fpioa_set_function(SD_SPI_SCLK,     FUNC_SPI0_SCLK);
    fpioa_set_function(SD_SPI_MOSI,     FUNC_SPI0_D0);
    fpioa_set_function(SD_SPI_MISO,     FUNC_SPI0_D1);
    
    fpioa_set_function(SD_SPI_CS_PIN,   FUNC_GPIOHS0 + SD_SPI_CS_PIN);

}

void i2s_io_mux_init()
{
    LOGI(TAG, "%s", __FUNCTION__);
    fpioa_set_function(CONFIG_PIN_NUM_ES8374_MCLK,  FUNC_I2S0_MCLK + 11 * ES8374_I2S_DEVICE);
    fpioa_set_function(CONFIG_PIN_NUM_ES8374_SCLK,  FUNC_I2S0_SCLK + 11 * ES8374_I2S_DEVICE);
    fpioa_set_function(CONFIG_PIN_NUM_ES8374_WS,    FUNC_I2S0_WS + 11 * ES8374_I2S_DEVICE);
    fpioa_set_function(CONFIG_PIN_NUM_ES8374_DOUT,  FUNC_I2S0_IN_D0 + 11 * ES8374_I2S_DEVICE);
    fpioa_set_function(CONFIG_PIN_NUM_ES8374_DIN,   FUNC_I2S0_OUT_D2 + 11 * ES8374_I2S_DEVICE);

    fpioa_set_io_driving(CONFIG_PIN_NUM_ES8374_MCLK,  FPIOA_DRIVING_0);
    fpioa_set_io_driving(CONFIG_PIN_NUM_ES8374_SCLK,  FPIOA_DRIVING_0);
    fpioa_set_io_driving(CONFIG_PIN_NUM_ES8374_WS,    FPIOA_DRIVING_0);
    fpioa_set_io_driving(CONFIG_PIN_NUM_ES8374_DOUT,  FPIOA_DRIVING_0);
    fpioa_set_io_driving(CONFIG_PIN_NUM_ES8374_DIN,   FPIOA_DRIVING_0);
    fpioa_set_sl(CONFIG_PIN_NUM_ES8374_MCLK,  1);
    fpioa_set_sl(CONFIG_PIN_NUM_ES8374_SCLK,  1);
    fpioa_set_sl(CONFIG_PIN_NUM_ES8374_WS,    1);
    fpioa_set_sl(CONFIG_PIN_NUM_ES8374_DOUT,  1);
    fpioa_set_sl(CONFIG_PIN_NUM_ES8374_DIN,   1);
}

void audio_es8374_config_init(void)
{
    
     LOGI(TAG, "%s", __FUNCTION__);
    ///////////////////////////////////////////////////////////////////////////
    es8374_i2s_iface_t iface;
    iface.bits = ES8374_BIT_LENGTH_16BITS;
    iface.fmt = ES8374_I2S_NORMAL;
    iface.mode = ES8374_MODE_SLAVE;
    iface.samples = ES8374_16K_SAMPLES;

    es8374_config_t cfg;
    cfg.adc_input = ES8374_ADC_INPUT_LINE1;
    cfg.dac_output = ES8374_DAC_OUTPUT_LINE1;
    cfg.es8374_mode = ES8374_MODE_BOTH;
    cfg.i2s_iface = iface;
    int ret = es8374_init(&cfg);

    if (ret != 0)
    {
        LOGE(TAG, "es8374 init fail");
    }
    else
    {
        LOGI(TAG, "es8374 init ok!!!");
    }

    es8374_ctrl_state(cfg.es8374_mode, ES8374_CTRL_START);
    
    LOGI(TAG, "es8374 mode: %d, state: %d", cfg.es8374_mode, ES8374_CTRL_START);
    
    es8374_set_voice_volume(80);

    es8374_write_reg(0x1e, 0xA4);

    // LOGI(TAG, "es8374 read reg all");
    // es8374_read_all();

}



int main(void)
{
    uint32_t freq = sysctl_pll_set_freq(SYSCTL_PLL0, 320000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 160000000UL);
    // sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 262144000UL);
    uint64_t core = current_coreid();
    printk(LOG_COLOR_W "-------------------------------\r\n");
    printk(LOG_COLOR_W "Sipeed@QinYUN575\r\n");
    printk(LOG_COLOR_W "Compile@ %s %s\r\n", __DATE__, __TIME__);
    printk(LOG_COLOR_W "Board: " LOG_COLOR_E BOARD_NAME "\r\n");
    printk(LOG_COLOR_W "pll freq: %dhz\r\n", freq);
    printk(LOG_COLOR_W "-------------------------------\r\n");


    LOGI(TAG, "%s", __FUNCTION__);
    io_mux_init();

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
    while (1) {
        if(wav_test(_T("0:ido.wav")))
        {
            LOGI(TAG, "Play music err");
            return -1;
        }
    }

    return 0;
}

static int sdcard_test(void)
{
    uint8_t status;

    LOGI(TAG, "/******************sdcard test*****************/");
    status = sd_init();
    LOGI(TAG, "sd init %d", status);
    if (status != 0)
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
    if (status != FR_OK)
        return status;

    LOGI(TAG, "printf filename");
    status = f_findfirst(&dj, &fno, _T("0:"), _T("*"));
    while (status == FR_OK && fno.fname[0]) {
        if (fno.fattrib & AM_DIR)
            LOGI(TAG, "dir:%s", fno.fname);
        else
            LOGI(TAG, "file:%s", fno.fname);
        status = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);
    return 0;
}

struct wav_file_t wav_file;

static int on_irq_dma3(void *ctx)
{
    if (wav_file.buff_end) {
        wav_file.buff_end = 2;
        return 0;
    }
    if (wav_file.buff_index == 0) {
        if (wav_file.buff1_used == 0) {
            LOGI(TAG, "error");
            return 0;
        }
        wav_file.buff0_used = 0;
        wav_file.buff_index = 1;
        wav_file.buff_current = wav_file.buff1;
        wav_file.buff_current_len = wav_file.buff1_read_len;
        if (wav_file.buff1_len > wav_file.buff1_read_len)
            wav_file.buff_end = 1;
    } else if (wav_file.buff_index == 1) {
        if (wav_file.buff0_used == 0) {
            LOGI(TAG, "error");
            return 0;
        }
        wav_file.buff1_used = 0;
        wav_file.buff_index = 0;
        wav_file.buff_current = wav_file.buff0;
        wav_file.buff_current_len = wav_file.buff0_read_len;
        if (wav_file.buff0_len > wav_file.buff0_read_len)
            wav_file.buff_end = 1;
    }

    i2s_play(ES8374_I2S_DEVICE, ES8374_TX_DMAC_CHANNEL, 
        (void *)wav_file.buff_current, wav_file.buff_current_len, wav_file.buff_current_len, 16, 2);

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
    }
    else
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
        }
        else
        {
            LOGI(TAG, "Read :> %s %d bytes lenth", v_buf, v_ret_len);
        }
        f_close(&file);
    }

    if ((ret = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
        LOGI(TAG, "open file %s err[%d]", path, ret);
        return ret;
    }
    else
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
    }
    else
    {
        LOGI(TAG, "Write %d bytes to %s ok", v_ret_len, path);
    }
    f_close(&file);
    return ret;
}

static int wav_test(TCHAR *path)
{
    enum errorcode_e status;
    FIL file;

    LOGI(TAG, "/*******************wav test*******************/");
    if (FR_OK != f_open(&file, path, FA_READ)) {
        LOGI(TAG, "open file fail");
        return -1;
    }

    wav_file.fp = &file;
    status = wav_init(&wav_file);
    LOGI(TAG, "result:%d", status);
    LOGI(TAG, "point:0x%08x", (uint32_t)f_tell(&file));
    LOGI(TAG, "numchannels:%d", wav_file.numchannels);
    LOGI(TAG, "samplerate:%d", wav_file.samplerate);
    LOGI(TAG, "byterate:%d", wav_file.byterate);
    LOGI(TAG, "blockalign:%d", wav_file.blockalign);
    LOGI(TAG, "bitspersample:%d", wav_file.bitspersample);
    LOGI(TAG, "datasize:%d", wav_file.datasize);


    LOGI(TAG, "start decode");
    status = wav_decode_init(&wav_file);
    if (OK != status) {
        f_close(&file);
        LOGI(TAG, "decode init fail");
        return -1;
    }    
replay:
    audio_es8374_config_init();
    i2s_init(I2S_DEVICE_0, I2S_TRANSMITTER, 0x0c);
    // i2s_set_enable(I2S_DEVICE_0, 0);
    
    uint32_t i2s_freq = sysctl_clock_get_freq(SYSCTL_CLOCK_I2S0);
    LOGI(TAG, "1.i2s clock is: %d", i2s_freq);

    sysctl_clock_set_threshold(SYSCTL_THRESHOLD_I2S0_M, 31); // 16384000 / (16000 * 256) = 4 ;
    // i2s_freq = sysctl_clock_get_freq(SYSCTL_CLOCK_I2S0);
    // LOGI(TAG, "2.i2s clock is: %d", i2s_freq);

    i2s_tx_channel_config(ES8374_I2S_DEVICE, ES8374_TX_CHANNEL, RESOLUTION_16_BIT, SCLK_CYCLES_32, TRIGGER_LEVEL_4, RIGHT_JUSTIFYING_MODE);
    i2s_rx_channel_config(ES8374_I2S_DEVICE, ES8374_RX_CHANNEL, RESOLUTION_16_BIT, SCLK_CYCLES_32, TRIGGER_LEVEL_4, STANDARD_MODE);

    i2s_set_sample_rate(ES8374_I2S_DEVICE, wav_file.samplerate/2);
    
    dmac_set_irq(ES8374_TX_DMAC_CHANNEL, on_irq_dma3, NULL, 1);
    i2s_io_mux_init();

    LOGI(TAG, "decode ok");
    i2s_play(ES8374_I2S_DEVICE, ES8374_TX_DMAC_CHANNEL, 
        (void *)wav_file.buff_current, wav_file.buff_current_len, wav_file.buff_current_len, 16, 2);

    while (1) {
        status = wav_decode(&wav_file);
        if (FILE_END == status) {
            while (wav_file.buff_end != 2)
                ;
            LOGI(TAG, "decode finish");
            break;
        } else if (FILE_FAIL == status) {
            LOGI(TAG, "decode init fail");
            break;
        }
    }
    f_close(&file);
    wav_decode_finish(&wav_file);
    goto replay;
    return 0;
}
