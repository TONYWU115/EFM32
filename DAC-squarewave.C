#include "em_device.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_timer.h"
#include "em_dac.h"

#define FREQ 1000
#define DAC_VALUE 621

void initDAC(void) {

    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DAC0, true);
    CMU_ClockEnable(cmuClock_TIMER0, true);

    DAC_Init_TypeDef dacInit = DAC_INIT_DEFAULT;
    dacInit.reference = dacRefVDD;
    DAC_Init(DAC0, &dacInit);

    DAC_Enable(DAC0, 0, true);

    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    timerInit.prescale = timerPrescale1;
    TIMER_Init(TIMER0, &timerInit);

    uint32_t topValue = CMU_ClockFreqGet(cmuClock_TIMER0) / FREQ;
    TIMER_TopSet(TIMER0, topValue);

    TIMER_Enable(TIMER0, true);
}

void setDACValue(uint32_t value) {
    DAC0->CH0DATA = value;
}

int main(void) {

    CHIP_Init();
    initDAC();

    uint32_t toggle = 0;

    while (1) {
        if (TIMER0->IF & TIMER_IF_OF) {
            TIMER0->IFC = TIMER_IFC_OF;

            if (toggle == 0) {
                setDACValue(DAC_VALUE);
                toggle = 1;
            } else {
                setDACValue(0);
                toggle = 0;
            }
        }
    }
}
