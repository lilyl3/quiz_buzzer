#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include "buzzer.h"
#include "gpio_pin.h"

/**
 * buzzer_irq_handler - Interrupt handler for buzzer button press
 * @irq: IRQ number
 * @dev_id: Pointer to the Buzzer structure (used as dev_id)
 *
 * This handler is triggered on falling edge of the button GPIO.
 * It atomically checks and sets the `pressedFlag`. If the flag was 0 (not already pressed),
 * it turns on the associated LED.
 *
 * Return: IRQ_HANDLED
 */
static irqreturn_t buzzer_irq_handler(int irq, void* dev_id) {
    Buzzer* buzzer = (Buzzer*)dev_id;
    int flag = atomic_xchg(buzzer->pressedFlag, 1);
    if (!flag){
        printk(KERN_INFO "Light up LED %s!\n", get_name(&buzzer->led));
        gpio_set_value(get_gpio_num(&buzzer->led), 1);
    }
    return IRQ_HANDLED;
}

/**
 * buzzer_request_irq - Requests an IRQ for the buzzer's button GPIO
 * @buzzer: Pointer to the Buzzer structure
 *
 * Converts the GPIO to an IRQ and registers the buzzer IRQ handler for it.
 *
 * Return: 0 on success, negative error code on failure
 */
int buzzer_request_irq(Buzzer* buzzer) {
    int ret;

    // Get IRQ number associated with GPIO
    ret = gpio_to_irq(get_gpio_num(&buzzer->button));
    set_irq_num(&buzzer->button, ret);
    if (ret < 0){
        printk(KERN_ERR "Failed to get an IRQ for buzzer %s (GPIO %d)\n", get_name(&buzzer->button), get_gpio_num(&buzzer->button));
        goto fail;
    }
    printk(KERN_INFO "Got IRQ %d for buzzer (GPIO %d)\n", get_irq_num(&buzzer->button), get_gpio_num(&buzzer->button));

    // Register the IRQ handler
    ret = request_irq(
        get_irq_num(&buzzer->button), 
        buzzer_irq_handler, 
        IRQF_TRIGGER_FALLING, 
        "buzzer", 
        buzzer
    );
    if (ret){
        printk(KERN_ERR "Failed to register IRQ handler for buzzer %s (GPIO %d)\n", get_name(&buzzer->button), get_gpio_num(&buzzer->button));
        goto fail;
    }
    
    return 0;

fail:
    cleanup_gpio(&buzzer->button);
    return ret;
}

/**
 * buzzer_init - Initializes the buzzer (button and LED GPIOs, IRQ)
 * @buzzer: Pointer to the Buzzer structure
 *
 * This function sets up the button as input and the LED as output.
 * It also requests the IRQ for the button.
 *
 * Return: 0 on success, negative error code on failure
 */
int buzzer_init(Buzzer* buzzer) {
    int ret;

    ret = setup_gpio(&buzzer->button, "input");
    if (ret) goto fail;
    ret = buzzer_request_irq(buzzer);
    if (ret) goto fail;

    ret = setup_gpio(&buzzer->led, "output");
    if (ret) {
        printk(KERN_ERR "Failed to setup buzzer LED %s (GPIO %d)\n", get_name(&buzzer->led), get_gpio_num(&buzzer->led));
        cleanup_gpio(&buzzer->button);
        return ret;
    }
    return 0;

fail:
    printk(KERN_ERR "Failed to setup buzzer %s (GPIO %d)\n", get_name(&buzzer->button), get_gpio_num(&buzzer->button));
    return ret;
}

/**
 * buzzer_cleanup - Cleans up the buzzer's resources
 * @buzzer: Pointer to the Buzzer structure
 *
 * Frees the IRQ and GPIOs associated with the button and LED.
 */
void buzzer_cleanup(Buzzer* buzzer) {
    cleanup_gpio(&buzzer->button);
    cleanup_gpio(&buzzer->led);
}