#include "em_chip.h"
#include "em_dac.h"
#include "em_timer.h"
#include "em_cmu.h"
#include "em_emu.h"


void DAC_Setup(void) {
    CMU_ClockEnable(cmuClock_DAC0, true);
    DAC_Init_TypeDef dacInit = DAC_INIT_DEFAULT;
    dacInit.reference = dacRefVDD;
    DAC_Init(DAC0, &dacInit);
    DAC_Enable(DAC0, 0, true);
}

void TIMER_Setup(void) {
    CMU_ClockEnable(cmuClock_TIMER0, true);
    TIMER_Init_TypeDef timerInit = TIMER_INIT_DEFAULT;
    timerInit.enable = true;
    timerInit.prescale = timerPrescale1024;
    TIMER_Init(TIMER0, &timerInit);

    uint32_t timerFreq = CMU_ClockFreqGet(cmuClock_TIMER0);
    TIMER_TopSet(TIMER0, timerFreq / (1024*20));
    TIMER_IntEnable(TIMER0, TIMER_IF_OF);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(void) {
    static bool toggle = false;
    TIMER_IntClear(TIMER0, TIMER_IF_OF);

    if (toggle) {

        DAC_ChannelOutputSet(DAC0, 0, 4095);
    } else {

        DAC_ChannelOutputSet(DAC0, 0, 0);
    }
    toggle = !toggle;
}

int main(void) {
    CHIP_Init();
    DAC_Setup();
    TIMER_Setup();

    while (1) {
        EMU_EnterEM1();
    }
}
