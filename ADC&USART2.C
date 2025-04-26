#include "em_device.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "em_adc.h"
#include "em_chip.h"
#include <stdio.h>

#define COM_PORT 3  // GPIO 端口 D (USART 位置 #1: PD0 和 PD1)
#define ADC_PORT 3  // GPIO 端口 D (ADC 通道 6 位置 #0: PD6)
#define TX_PIN 0
#define ADC_PIN 6  // ADC 通道 6
#define VREF 3.3    // 參考電壓 (假設使用 3.3V)
#define ADC_MAX 4095 // 12-bit ADC 最大值

void initADC(void) {
    CMU_ClockEnable(cmuClock_ADC0, true);
    
    ADC_Init_TypeDef adcInit = ADC_INIT_DEFAULT;
    adcInit.timebase = ADC_TimebaseCalc(0);
    adcInit.prescale = ADC_PrescaleCalc(7000000, 0);
    ADC_Init(ADC0, &adcInit);
    
    ADC_InitSingle_TypeDef adcSingleInit = ADC_INITSINGLE_DEFAULT;
    adcSingleInit.reference = adcRefVDD;
    adcSingleInit.acqTime = adcAcqTime4;
    adcSingleInit.input = adcSingleInputCh6; // 修正 posSel 為 input
    ADC_InitSingle(ADC0, &adcSingleInit);
}

void initUSART(void) {
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_USART1, true);
    
    GPIO_PinModeSet(gpioPortD, TX_PIN, gpioModePushPull, 1);
    
    USART_InitAsync_TypeDef usartInit = USART_INITASYNC_DEFAULT;
    USART_InitAsync(USART1, &usartInit);
    
    // 修正 USART 路由設定，適用於 EFM32LG990F256
    USART1->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_LOCATION_LOC1;
}

void USART_SendChar(char ch) {
    while (!(USART1->STATUS & USART_STATUS_TXBL));
    USART_Tx(USART1, ch);
}

void USART_SendString(const char *str) {
    while (*str) {
        USART_SendChar(*str++);
    }
}

uint32_t readADC(void) {
    ADC_Start(ADC0, adcStartSingle);
    while (ADC0->STATUS & ADC_STATUS_SINGLEACT);
    return ADC_DataSingleGet(ADC0);
}

float convertToVoltage(uint32_t adcValue) {
    return (adcValue * VREF) / ADC_MAX;
}

int main(void) {
    CHIP_Init();
    initADC();
    initUSART();
    
    while (1) {
        uint32_t adcValue = readADC();
        float voltage = convertToVoltage(adcValue);
        char buffer[30];
        sprintf(buffer, "ADC: %lu, Voltage: %.2fV\r\n", adcValue, voltage);
        USART_SendString(buffer);
        
        for (volatile uint32_t i = 0; i < 100000; i++); // 簡單的延遲
    }
}
