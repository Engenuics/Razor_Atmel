#include "configuration.h"

void SystemInit(void)
{
    extern unsigned int __isr_vector;

    unsigned int * src = (unsigned int *)&__isr_vector;

    AT91C_BASE_NVIC->NVIC_VTOFFR = ((unsigned int)(src)) | (0x0 << 7);
}
