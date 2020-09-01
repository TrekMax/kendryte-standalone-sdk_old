#include "board_config.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "spi.h"
#include "sx1278.h"
#include "syslog.h"
// #include <stdlib.h>
#include <string.h>
#include "sleep.h"

#define DATALENGTH 8

static const char *TAG = "lora";

void sx1278_hw_init(sx1278_hw_t *hw)
{
    LOGD(TAG, "%d|%s", __LINE__, __FUNCTION__);
    fpioa_set_function(SPI_LORA_SX127X_CS_PIN_NUM, FUNC_SPI1_SS0);   // CS
    fpioa_set_function(SPI_LORA_SX127X_SCK_PIN_NUM, FUNC_SPI1_SCLK); // SCLK
    fpioa_set_function(SPI_LORA_SX127X_MOSI_PIN_NUM, FUNC_SPI1_D0);  // MOSI
    fpioa_set_function(SPI_LORA_SX127X_MISO_PIN_NUM, FUNC_SPI1_D1);  // MISO

    fpioa_set_function(SPI_LORA_SX127X_IRQ_PIN_NUM, FUNC_GPIOHS0 + SPI_LORA_SX127X_IQR_GPIO_NUM); // IQR->D0
    fpioa_set_function(SPI_LORA_SX127X_RST_PIN_NUM, FUNC_GPIOHS0 + SPI_LORA_SX127X_RST_GPIO_NUM); // RST

    gpiohs_set_drive_mode(SPI_LORA_SX127X_IRQ_PIN_NUM, GPIO_DM_INPUT);
    gpiohs_set_drive_mode(SPI_LORA_SX127X_RST_PIN_NUM, GPIO_DM_OUTPUT);

    gpiohs_set_pin(SPI_LORA_SX127X_CS_PIN_NUM, GPIO_PV_HIGH);
    gpiohs_set_pin(SPI_LORA_SX127X_RST_PIN_NUM, GPIO_PV_HIGH);

    spi_init(SPI_DEVICE_1, SPI_WORK_MODE_0, SPI_FF_STANDARD, DATALENGTH, 0);
}

void sx1278_hw_delay_ms(uint32_t delay_mses)
{
    msleep(delay_mses);
}

void sx1278_hw_reset(sx1278_hw_t *hw)
{
    gpiohs_set_pin(SPI_LORA_SX127X_CS_PIN_NUM, GPIO_PV_HIGH);

    gpiohs_set_pin(SPI_LORA_SX127X_RST_PIN_NUM, GPIO_PV_LOW);
    sx1278_hw_delay_ms(10);
    gpiohs_set_pin(SPI_LORA_SX127X_RST_PIN_NUM, GPIO_PV_HIGH);
    sx1278_hw_delay_ms(100);
}

uint8_t sx1278_hw_get_dio0(sx1278_hw_t *hw)
{
    return gpiohs_get_pin(SPI_LORA_SX127X_IRQ_PIN_NUM);
}

uint8_t sx1278_SPIRead(sx1278_t *module, uint8_t addr)
{
    uint8_t tmp;
    spi_receive_data_standard(SPI_DEVICE_1, SPI_CHIP_SELECT_NSS, &addr, 1, &tmp, 1);
    return tmp;
}

void sx1278_SPIWrite(sx1278_t *module, uint8_t addr, uint8_t cmd)
{
    addr = addr | 0x80;
    spi_send_data_standard(SPI_DEVICE_1, SPI_CHIP_SELECT_NSS, &addr, 1, &cmd, 1);
}

void sx1278_SPIBurstRead(sx1278_t *module, uint8_t addr, uint8_t *rxBuf,
                         uint8_t length)
{
    uint8_t i;
    if(length <= 1)
    {
        return;
    } else
    {
        spi_receive_data_standard(SPI_DEVICE_1, SPI_CHIP_SELECT_NSS, &addr, 1, rxBuf, length);
    }
}

void sx1278_SPIBurstWrite(sx1278_t *module, uint8_t addr, uint8_t *txBuf,
                          uint8_t length)
{
    unsigned char i;
    if(length <= 1)
    {
        return;
    } else
    {
        addr = addr | 0x80;
        spi_send_data_standard(SPI_DEVICE_1, SPI_CHIP_SELECT_NSS, &addr, 1, txBuf, length);
    }
}

void sx1278_defaultConfig(sx1278_t *module)
{
    sx1278_config(module, module->frequency, module->power, module->LoRa_Rate,
                  module->LoRa_BW);
}

void sx1278_config(sx1278_t *module, uint8_t frequency, uint8_t power,
                   uint8_t LoRa_Rate, uint8_t LoRa_BW)
{
    sx1278_sleep(module); //Change modem mode Must in Sleep mode
    sx1278_hw_delay_ms(15);

    sx1278_entryLoRa(module);
    //sx1278_SPIWrite(module, 0x5904); //?? Change digital regulator form 1.6V to 1.47V: see errata note

    sx1278_SPIBurstWrite(module, LR_RegFrMsb,
                         (uint8_t *)SX1278_Frequency[frequency], 3); //setting  frequency parameter

    //setting base parameter
    sx1278_SPIWrite(module, LR_RegPaConfig, SX1278_Power[power]); //Setting output power parameter

    sx1278_SPIWrite(module, LR_RegOcp, 0x0B); //RegOcp,Close Ocp
    sx1278_SPIWrite(module, LR_RegLna, 0x23); //RegLNA,High & LNA Enable
    if(SX1278_SpreadFactor[LoRa_Rate] == 6)
    { //SFactor=6
        uint8_t tmp;
        sx1278_SPIWrite(module,
                        LR_RegModemConfig1,
                        ((SX1278_LoRaBandwidth[LoRa_BW] << 4) + (SX1278_CR << 1) + 0x01)); //Implicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

        sx1278_SPIWrite(module,
                        LR_RegModemConfig2,
                        ((SX1278_SpreadFactor[LoRa_Rate] << 4) + (SX1278_CRC << 2) + 0x03));

        tmp = sx1278_SPIRead(module, 0x31);
        tmp &= 0xF8;
        tmp |= 0x05;
        sx1278_SPIWrite(module, 0x31, tmp);
        sx1278_SPIWrite(module, 0x37, 0x0C);
    } else
    {
        sx1278_SPIWrite(module,
                        LR_RegModemConfig1,
                        ((SX1278_LoRaBandwidth[LoRa_BW] << 4) + (SX1278_CR << 1) + 0x00)); //Explicit Enable CRC Enable(0x02) & Error Coding rate 4/5(0x01), 4/6(0x02), 4/7(0x03), 4/8(0x04)

        sx1278_SPIWrite(module,
                        LR_RegModemConfig2,
                        ((SX1278_SpreadFactor[LoRa_Rate] << 4) + (SX1278_CRC << 2) + 0x03)); //SFactor &  LNA gain set by the internal AGC loop
    }

    sx1278_SPIWrite(module, LR_RegSymbTimeoutLsb, 0xFF); //RegSymbTimeoutLsb Timeout = 0x3FF(Max)
    sx1278_SPIWrite(module, LR_RegPreambleMsb, 0x00);    //RegPreambleMsb
    sx1278_SPIWrite(module, LR_RegPreambleLsb, 12);      //RegPreambleLsb 8+4=12byte Preamble
    sx1278_SPIWrite(module, REG_LR_DIOMAPPING2, 0x01);   //RegDioMapping2 DIO5=00, DIO4=01
    module->readBytes = 0;
    sx1278_standby(module); //Entry standby mode
}

void sx1278_standby(sx1278_t *module)
{
    sx1278_SPIWrite(module, LR_RegOpMode, 0x09);
    module->status = STANDBY;
}

void sx1278_sleep(sx1278_t *module)
{
    sx1278_SPIWrite(module, LR_RegOpMode, 0x08);
    module->status = SLEEP;
}

void sx1278_entryLoRa(sx1278_t *module)
{
    sx1278_SPIWrite(module, LR_RegOpMode, 0x88);
}

void sx1278_clearLoRaIrq(sx1278_t *module)
{
    sx1278_SPIWrite(module, LR_RegIrqFlags, 0xFF);
}

int sx1278_LoRaEntryRx(sx1278_t *module, uint8_t length, uint32_t timeout)
{
    uint8_t addr;

    module->packetLength = length;

    sx1278_defaultConfig(module);                      //Setting base parameter

    sx1278_SPIWrite(module, REG_LR_PADAC, 0x84);       //Normal and RX
    // addr = sx1278_SPIRead(module, REG_LR_PADAC); //Read RxBaseAddr
    // LOGD(TAG, "ADDR: [REG_LR_PADAC-0x%02X:0x%02X]\r\n", REG_LR_PADAC, addr);

    sx1278_SPIWrite(module, LR_RegHopPeriod, 0xFF);    //No FHSS
    // addr = sx1278_SPIRead(module, LR_RegHopPeriod); //Read RxBaseAddr
    // LOGD(TAG, "ADDR: [LR_RegHopPeriod-0x%02X:0x%02X]\r\n", LR_RegHopPeriod, addr);

    sx1278_SPIWrite(module, REG_LR_DIOMAPPING1, 0x01); //DIO=00,DIO1=00,DIO2=00, DIO3=01
    addr = sx1278_SPIRead(module, REG_LR_DIOMAPPING1); //Read RxBaseAddr
    LOGD(TAG, "ADDR: [REG_LR_DIOMAPPING1-0x%02X:0x%02X]\r\n", REG_LR_DIOMAPPING1, addr);

    sx1278_SPIWrite(module, LR_RegIrqFlagsMask, 0x3F); //Open RxDone interrupt & Timeout
    addr = sx1278_SPIRead(module, LR_RegIrqFlagsMask); //Read RxBaseAddr
    LOGD(TAG, "ADDR: [LR_RegIrqFlagsMask-0x%02X:0x%02X]\r\n", LR_RegIrqFlagsMask, addr);

    sx1278_clearLoRaIrq(module);
    sx1278_SPIWrite(module, LR_RegPayloadLength, length); //Payload Length 21byte(this register must difine when the data long of one byte in SF is 6)
    addr = sx1278_SPIRead(module, LR_RegPayloadLength); //Read RxBaseAddr
    LOGD(TAG, "ADDR: [LR_RegPayloadLength-0x%02X:0x%02X]\r\n", LR_RegPayloadLength, addr);

    sx1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RxBaseAddr->FiFoAddrPtr
    sx1278_SPIWrite(module, LR_RegOpMode, 0x8d);      //Mode//Low Frequency Moder
    //sx1278_SPIWrite(module, LR_RegOpMode,0x05);	//Continuous Rx Mode //High Frequency Mode
    module->readBytes = 0;

    while(1)
    {
        if((sx1278_SPIRead(module, LR_RegModemStat) & 0x04) == 0x04)
        { //Rx-on going RegModemStat
            module->status = RX;
            return 1;
        }
        if(--timeout == 0)
        {
            sx1278_hw_reset(module->hw);
            sx1278_defaultConfig(module);
            return 0;
        }
        sx1278_hw_delay_ms(1);
    }
}

uint8_t sx1278_LoRaRxPacket(sx1278_t *module)
{
    unsigned char addr;
    unsigned char packet_size;

    if(sx1278_hw_get_dio0(module->hw))
    {
        LOGD(TAG, "sx1278_LoRaRxPacket\r\n");
        memset(module->rxBuffer, 0x00, SX1278_MAX_PACKET);

        addr = sx1278_SPIRead(module, LR_RegFifoRxCurrentaddr); //last packet addr
        sx1278_SPIWrite(module, LR_RegFifoAddrPtr, addr);       //RxBaseAddr -> FiFoAddrPtr

        if(module->LoRa_Rate == SX1278_LORA_SF_6)
        { //When SpreadFactor is six,will used Implicit Header mode(Excluding internal packet length)
            packet_size = module->packetLength;
        } else
        {
            packet_size = sx1278_SPIRead(module, LR_RegRxNbBytes); //Number for received bytes
        }

        sx1278_SPIBurstRead(module, 0x00, module->rxBuffer, packet_size);
        module->readBytes = packet_size;
        sx1278_clearLoRaIrq(module);
    }
    return module->readBytes;
}

int sx1278_LoRaEntryTx(sx1278_t *module, uint8_t length, uint32_t timeout)
{
    uint8_t addr;
    uint8_t temp;

    module->packetLength = length;

    sx1278_defaultConfig(module);                      //setting base parameter
    sx1278_SPIWrite(module, REG_LR_PADAC, 0x87);       //Tx for 20dBm
    sx1278_SPIWrite(module, LR_RegHopPeriod, 0x00);    //RegHopPeriod NO FHSS
    sx1278_SPIWrite(module, REG_LR_DIOMAPPING1, 0x41); //DIO0=01, DIO1=00,DIO2=00, DIO3=01
    sx1278_clearLoRaIrq(module);
    sx1278_SPIWrite(module, LR_RegIrqFlagsMask, 0xF7);    //Open TxDone interrupt
    sx1278_SPIWrite(module, LR_RegPayloadLength, length); //RegPayloadLength 21byte
    addr = sx1278_SPIRead(module, LR_RegFifoTxBaseAddr);  //RegFiFoTxBaseAddr
    LOGD(TAG, "[%d]ADDR:0x%X", __LINE__, addr);
    sx1278_SPIWrite(module, LR_RegFifoAddrPtr, addr); //RegFifoAddrPtr

    while(1)
    {
        temp = sx1278_SPIRead(module, LR_RegPayloadLength);
        if(temp == length)
        {
            module->status = TX;
            return 1;
        }

        if(--timeout == 0)
        {
            sx1278_hw_reset(module->hw);
            sx1278_defaultConfig(module);
            return 0;
        }
    }
}

int sx1278_LoRaTxPacket(sx1278_t *module, uint8_t *txBuffer, uint8_t length,
                        uint32_t timeout)
{
    sx1278_SPIBurstWrite(module, 0x00, txBuffer, length);
    sx1278_SPIWrite(module, LR_RegOpMode, 0x8b); //Tx Mode
    while(1)
    {
        if(sx1278_hw_get_dio0(module->hw))
        { //if(Get_NIRQ()) //Packet send over
            sx1278_SPIRead(module, LR_RegIrqFlags);
            sx1278_clearLoRaIrq(module); //Clear irq
            sx1278_standby(module);      //Entry Standby mode
            return 1;
        }

        if(--timeout == 0)
        {
            sx1278_hw_reset(module->hw);
            sx1278_defaultConfig(module);
            return 0;
        }
        sx1278_hw_delay_ms(1);
    }
}

void sx1278_begin(sx1278_t *module, uint8_t frequency, uint8_t power,
                  uint8_t LoRa_Rate, uint8_t LoRa_BW, uint8_t packetLength)
{
    sx1278_hw_init(module->hw);
    uint32_t ret = spi_set_clk_rate(SPI_DEVICE_1, 10 * 1000 * 1000);
    LOGI(TAG, "spi clk is %d", ret);
    module->frequency = frequency;
    module->power = power;
    module->LoRa_Rate = LoRa_Rate;
    module->LoRa_BW = LoRa_BW;
    module->packetLength = packetLength;
    sx1278_defaultConfig(module);

    sx1278_hw_reset(module->hw);
    uint8_t temp, addr;
    addr = 0x06;
    while(1)
    {
        spi_receive_data_standard(SPI_DEVICE_1, SPI_CHIP_SELECT_NSS, &addr, 1, &temp, 1);
        if(0x6c == temp)
        {
            break;
        }
        LOGE(TAG, "[ERROR %s()-%d]SPI error, temp:0x%X\r\n", __func__, __LINE__, temp);
        msleep(1000);
    }
    LOGI(TAG, "SPI CONNECTED\r\n");
    LOGI(TAG, "=============================\r\n");
}

int sx1278_transmit(sx1278_t *module, uint8_t *txBuf, uint8_t length,
                    uint32_t timeout)
{
    if(sx1278_LoRaEntryTx(module, length, timeout))
    {
        return sx1278_LoRaTxPacket(module, txBuf, length, timeout);
    }
    return 0;
}

int sx1278_receive(sx1278_t *module, uint8_t length, uint32_t timeout)
{
    return sx1278_LoRaEntryRx(module, length, timeout);
}

uint8_t sx1278_available(sx1278_t *module)
{
    return sx1278_LoRaRxPacket(module);
}

uint8_t sx1278_read(sx1278_t *module, uint8_t *rxBuf, uint8_t length)
{
    if(length != module->readBytes)
        length = module->readBytes;
    memcpy(rxBuf, module->rxBuffer, length);
    rxBuf[length] = '\0';
    module->readBytes = 0;
    return length;
}

uint8_t sx1278_RSSI_LoRa(sx1278_t *module)
{
    uint32_t temp = 10;
    temp = sx1278_SPIRead(module, LR_RegRssiValue); //Read RegRssiValue, Rssi value
    temp = temp + 127 - 137;                        //127:Max RSSI, 137:RSSI offset
    return (uint8_t)temp;
}

uint8_t sx1278_RSSI(sx1278_t *module)
{
    uint8_t temp = 0xff;
    temp = sx1278_SPIRead(module, 0x11);
    temp = 127 - (temp >> 1); //127:Max RSSI
    return temp;
}
