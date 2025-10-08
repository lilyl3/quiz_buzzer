#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/errno.h>

#include "reset_button.h"
#include "gpio_pin.h"

/**
 * reset_button_irq_handler - IRQ handler for the reset button
 * @irq: IRQ number
 * @dev_id: Pointer to the ResetButton structure
 *
 * When the reset button is pressed, this handler turns off all associated LEDs
 * and resets the shared pressedFlag to 0 to allow buzzer presses again.
 *
 * Returns: IRQ_HANDLED
 */
irqreturn_t reset_button_irq_handler(int irq, void* dev_id) {
    ResetButton* reset_btn = (ResetButton*)dev_id;
    printk(KERN_INFO "Reset all LED lights!\n");
    for (int i = 0; i < NUM_BUZZERS; i++) {
        if (reset_btn->leds[i])
            gpio_set_value(get_gpio_num(reset_btn->leds[i]), 0);  // turn off LED
    }
    atomic_set(reset_btn->pressedFlag, 0);
    return IRQ_HANDLED;
}

/**
 * reset_button_request_irq - Configures and registers IRQ for the reset button
 * @reset_btn: Pointer to the ResetButton structure
 *
 * Returns: 0 on success, negative errno on failure
 */
int reset_button_request_irq(ResetButton* reset_btn) {
    int ret;

    // Get IRQ number associated with GPIO
    ret = gpio_to_irq(get_gpio_num(&reset_btn->button));
    set_irq_num(&reset_btn->button, ret);
    if (ret < 0){
        printk(KERN_ERR "Failed to get an IRQ for reset button (GPIO %d)\n", get_gpio_num(&reset_btn->button));
        goto fail;
    }
    printk(KERN_INFO "Got IRQ %d for reset button (GPIO %d)\n", get_irq_num(&reset_btn->button), get_gpio_num(&reset_btn->button));

    // Register the IRQ handler
    ret = request_irq(
        get_irq_num(&reset_btn->button), 
        reset_button_irq_handler, 
        IRQF_TRIGGER_FALLING, 
        "reset button", 
        reset_btn
    );
    if (ret){
        printk(KERN_ERR "Failed to register IRQ handler for reset button (GPIO %d)\n", get_gpio_num(&reset_btn->button));
        goto fail;
    }
    
    return 0;

fail:
    cleanup_gpio(&reset_btn->button);
    return ret;
}

/**
 * reset_button_init
 * @reset_btn: Pointer to the ResetButton structure
 *
 * Requests and configures the GPIO as an input, and registers the IRQ.
 *
 * Returns: 0 on success, negative errno on failure
 */
int reset_button_init(ResetButton* reset_btn) {
    int ret;

    ret = setup_gpio(&reset_btn->button, "input");
    if (ret) goto fail;

    ret = reset_button_request_irq(reset_btn);
    if (ret) goto fail;

    return 0;

fail:
    printk(KERN_ERR "Failed to setup reset button (GPIO %d)\n", get_gpio_num(&reset_btn->button));
    return ret;
}

/**
 * reset_button_cleanup - Releases resources associated with the reset button
 * @reset_btn: Pointer to the ResetButton structure
 *
 * Frees the GPIO and IRQ associated with the reset button.
 */
void reset_button_cleanup(ResetButton* reset_btn) {
    cleanup_gpio(&reset_btn->button);
}