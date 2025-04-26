#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_cmu.h"

int main(void)
{

    CHIP_Init();

    CMU_ClockEnable(cmuClock_GPIO, true);


    GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 1);
    GPIO_PinModeSet(gpioPortE, 3, gpioModePushPull, 1);

    while (1)
    {
        // 保持 LED1 和 LED0 亮起
    }
}
