#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include "buzzer.h"
#include "gpio_pin.h"

static irqreturn_t buzzer_irq_handler(int irq, void* dev_id) {
    Buzzer* buzzer = (Buzzer*)dev_id;
    int flag = atomic_xchg(buzzer->pressedFlag, 1);
    if (!flag){
        printk(KERN_INFO "Light up LED %s!\n", get_name(&buzzer->led));
        gpio_set_value(get_gpio_num(&buzzer->led), 1);
    }
    return IRQ_HANDLED;
}

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

void buzzer_cleanup(Buzzer* buzzer) {
    cleanup_gpio(&buzzer->button);
    cleanup_gpio(&buzzer->led);
}