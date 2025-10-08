#ifndef BUZZER_H
#define BUZZER_H

#include "gpio_pin.h"

typedef struct Buzzer {
    GPIO_Pin button;
    GPIO_Pin led;
    atomic_t* pressedFlag;
} Buzzer;

irqreturn_t buzzer_irq_handler(int, void*);
int buzzer_request_irq(Buzzer*);

int buzzer_init(Buzzer*);
void buzzer_cleanup(Buzzer*);

#endif