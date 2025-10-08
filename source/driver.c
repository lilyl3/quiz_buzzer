#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/errno.h>

#include "gpio_pin.h"
#include "buzzer.h"
#include "reset_button.h"

// Array of buzzer structures: each has a button and a corresponding LED
static Buzzer buzzers[NUM_BUZZERS] = {
    {
        { .gpio_num = 27, .irq_num = -1, .name = "button 1"},
        { .gpio_num = 24, .irq_num = -1, .name = "led 1 (blue)"},
    },
    {
        { .gpio_num = 23, .irq_num = -1, .name = "button 2"},
        { .gpio_num = 25, .irq_num = -1, .name = "led 2 (red)"},
    },
};

// Reset button structure with references to all buzzer LEDs
static ResetButton reset_button = {
    .button = { .gpio_num = 26, .irq_num = -1, .name = "reset button" },
    .leds = { &buzzers[0].led, &buzzers[1].led }
};

// Shared atomic flag to prevent multiple buzzers from activating simultaneously
static atomic_t pressedFlag = ATOMIC_INIT(0);

/**
 * gpio_module_init - Called when the module is loaded
 *
 * Initializes each buzzer and the reset button.
 * Sets up GPIOs and IRQs for input and output.
 */
static int __init gpio_module_init(void) {
    int ret, i;

    printk(KERN_INFO "Setting up buzzers\n");
    for (i = 0; i < NUM_BUZZERS; i++){
        buzzers[i].pressedFlag = &pressedFlag;
        ret = buzzer_init(&buzzers[i]);
        if (ret) {
            printk(KERN_ERR "Failed to setup buzzer %d\n", i);
            goto fail;
        }
    }

    printk(KERN_INFO "Setting up reset button\n");
    reset_button.pressedFlag = &pressedFlag;
    ret = reset_button_init(&reset_button);
    if (ret) {
        printk(KERN_ERR "Failed to setup reset button\n");
        goto fail;
    }

    printk(KERN_INFO "Loaded module!\n");
    return 0;

fail:
    // On failure, clean up already-initialized gpios
    for (--i; i >= 0; --i)
        buzzer_cleanup(&buzzers[i]);
    return ret;
}

/**
 * gpio_module_exit - Called when the module is unloaded
 *
 * Cleans up all GPIOs and frees IRQs for buzzers and reset button.
 */
static void __exit gpio_module_exit(void) {
    int i;
    for (i = 0; i < NUM_BUZZERS; i++){
        buzzer_cleanup(&buzzers[i]);
    }
    reset_button_cleanup(&reset_button);

    printk(KERN_INFO "Unloaded module!\n");
}

module_init(gpio_module_init);
module_exit(gpio_module_exit);   // Not called if __init fails

MODULE_LICENSE("10/2025");
MODULE_AUTHOR("Lily");
MODULE_DESCRIPTION("Linux GPIO Kernel Module");