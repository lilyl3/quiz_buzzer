#ifndef GPIO_PIN_H
#define GPIO_PIN_H

// Structure representing a GPIO pin with an optional IRQ and name
typedef struct GPIO_Pin {
    unsigned int gpio_num;
    int irq_num;
    const char* name;
} GPIO_Pin;

unsigned int get_gpio_num (GPIO_Pin*);
int get_irq_num (GPIO_Pin*);
void set_irq_num (GPIO_Pin*, int);

// Requests and sets GPIO pin (as input or output)
int setup_gpio (GPIO_Pin*, const char*);

// Cleans up a GPIO pin (unexports and frees it, and releases IRQ if assigned)
void cleanup_gpio(GPIO_Pin*);

#endif