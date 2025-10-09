#ifndef RESET_BUTTON_H
#define RESET_BUTTON_H

#include "gpio_pin.h" 

#define NUM_BUZZERS 2

typedef struct ResetButton {
    GPIO_Pin button;
    GPIO_Pin* leds[NUM_BUZZERS];
    atomic_t* pressedFlag;
} ResetButton;

irqreturn_t reset_button_irq_handler(int, void*);
int reset_button_request_irq(ResetButton*);

int reset_button_init(ResetButton*);
void reset_button_cleanup(ResetButton*);

#endif