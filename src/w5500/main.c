#include <stdio.h>

#include "sysctl.h"
#include "bsp.h"
#include "fpioa.h"
#include "timer.h"
#include "printf.h"

#include "lcd.h"

#include "myspi.h"

#include "socket.h" // Just include one header for WIZCHIP
#include "dhcp.h"
#include "syslog.h"
#include "system_config.h"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static uint8_t dhcp_get_ip = 0;

static RIP_MSG dhcp_msg;
static uint8_t w5500_mac[6] = {0x00, 0x08, 0xdc, 0x11, 0x11, 0x11};

static const char *TAG = "main";

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void io_mux_init(void)
{
    /* DVP */
    fpioa_set_function(47, FUNC_CMOS_PCLK);
    fpioa_set_function(46, FUNC_CMOS_XCLK);
    fpioa_set_function(45, FUNC_CMOS_HREF);
    fpioa_set_function(44, FUNC_CMOS_PWDN);
    fpioa_set_function(43, FUNC_CMOS_VSYNC);
    fpioa_set_function(42, FUNC_CMOS_RST);
    fpioa_set_function(41, FUNC_SCCB_SCLK);
    fpioa_set_function(40, FUNC_SCCB_SDA);

    /* LCD */
    fpioa_set_function(CONFIG_LCD_PIN_RST, FUNC_GPIOHS0 + CONFIG_LCD_GPIOHS_RST);
    fpioa_set_function(CONFIG_LCD_PIN_DCX, FUNC_GPIOHS0 + CONFIG_LCD_GPIOHS_DCX);
    fpioa_set_function(CONFIG_LCD_PIN_WRX, FUNC_SPI0_SS3);
    fpioa_set_function(CONFIG_LCD_PIN_SCK, FUNC_SPI0_SCLK);

    /* change to 1.8V */
    sysctl_set_spi0_dvp_data(1);

    /* ETH */
    fpioa_set_function(CONFIG_ETH_PIN_CSSX, FUNC_GPIOHS0 + CONFIG_ETH_GPIOHS_CSX); //CSS
    fpioa_set_function(CONFIG_ETH_PIN_INTX, FUNC_GPIOHS0 + CONFIG_ETH_GPIOHS_INT); //INT
    fpioa_set_function(CONFIG_ETH_PIN_RSTX, FUNC_GPIOHS0 + CONFIG_ETH_GPIOHS_RST); //RST

    fpioa_set_function(CONFIG_ETH_PIN_MISO, FUNC_SPI1_D1);   //MISO
    fpioa_set_function(CONFIG_ETH_PIN_MOSI, FUNC_SPI1_D0);   //MOSI
    fpioa_set_function(CONFIG_ETH_PIN_SCLK, FUNC_SPI1_SCLK); //CLK
}

static void io_set_power(void)
{
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int eth_w5500_timer_callback(void *ctx)
{
    DHCP_time_handler();
    return 0;
}

/* timer 1s */
void eth_w5500_timer_init(void)
{
    /* Init timer devices */
    timer_init(TIMER_DEVICE_0);
    /* Set timer interval to 500ms */
    timer_set_interval(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1000 * 1000 * 1000 * 1);
    /* Set timer callback function with single shot method */
    timer_irq_register(TIMER_DEVICE_0, TIMER_CHANNEL_0, 0, 1, eth_w5500_timer_callback, NULL);
    /* Enable timer */
    timer_set_enable(TIMER_DEVICE_0, TIMER_CHANNEL_0, 1);
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* The default handler of ip assign first */
static void dhcp_ip_assign(void)
{
    printf("[%s]->%d\r\n", __func__, __LINE__);

    uint8_t ip[4], gw[4], sn[4], dns[4];

    getIPfromDHCP(ip);
    getGWfromDHCP(gw);
    getSNfromDHCP(sn);
    getDNSfromDHCP(dns);

    setSIPR(ip);
    setSUBR(sn);
    setGAR(gw);

    printf("SIP: %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
    printf("GAR: %d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
    printf("SUB: %d.%d.%d.%d\r\n", sn[0], sn[1], sn[2], sn[3]);
    printf("DNS: %d.%d.%d.%d\r\n", dns[0], dns[1], dns[2], dns[3]);

    dhcp_get_ip = 1;
    return;
}

int main(void)
{
    io_set_power();
    io_mux_init();

    lcd_init(LCD_ST7789);
    lcd_set_direction(DIR_YX_RLUD);
    lcd_clear(RED);

    ///////////////////////////////////////////////////////////////////////////
    //初始化spi
    eth_w5500_spi_init();
    eth_w5500_timer_init();

    //注册回调
    reg_wizchip_cs_cbfunc(eth_w5500_spi_cs_sel, eth_w5500_spi_cs_desel); //注册SPI片选信号函数
    reg_wizchip_spi_cbfunc(eth_w5500_spi_read, eth_w5500_spi_write);     //注册读写函数
    LOGI(TAG, "%d %s",__LINE__, __FUNCTION__);
    ///////////////////////////////////////////////////////////////////////////
    //设置socket的内存分配大小
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
    /* WIZCHIP SOCKET Buffer initialize */
    if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
    {
        printf("WIZCHIP Initialized fail.\r\n");
        while (1)
            ;
    }
    ///////////////////////////////////////////////////////////////////////////
    uint8_t tmp;
    /* PHY link status check */
    do
    {
        if (ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1)
        {
            printf("Unknown PHY Link stauts.\r\n");
        }
    } while (tmp == PHY_LINK_OFF);
    ///////////////////////////////////////////////////////////////////////////

    reg_dhcp_cbfunc(dhcp_ip_assign, NULL, NULL);

    DHCP_init(0, &dhcp_msg, w5500_mac);

    while (1)
    {
        uint8_t dhcp_stat = DHCP_run();

        if ((dhcp_stat == DHCP_FAILED) || (dhcp_stat == DHCP_STOPPED))
        {
            //失败了。重新来，这里应该加上重试次数判断
            DHCP_init(0, &dhcp_msg, w5500_mac);
        }
    };
}
