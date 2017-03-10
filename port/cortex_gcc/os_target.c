#include "cmtos.h"
#include "os_target.h"

#include "common.h" // TODO delet

//TODO OS_USE_CUSTOM_TIMER

void os_initSysTimer(void)    //FIXME самому настраивать systick
{
    if (SysTick_Config(SystemCoreClock / SYSTICK_FREQ)) // 1 мс = 0.001 с = 1/1000 с
    {
        while (1);          //TODO assert(0);
    }
    __enable_irq();
}

void SysTick_Handler(void)
{
    os_sysTimer_isr();
}
