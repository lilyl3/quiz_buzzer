#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/errno.h>

#define GPIO_NUM1 17
#define GPIO_NUM2 23
#define NUM_BUTTONS 2

typedef struct Button_Data {
    unsigned int gpio_num;
    int irq_num;
    const char* name;
} Button_Data;

static Button_Data buttons[NUM_BUTTONS] = {
    { .gpio_num = GPIO_NUM1, .irq_num = -1, .name = "button1" },
    { .gpio_num = GPIO_NUM2, .irq_num = -1, .name = "button2" },
};

/**
 * Interrupt handler that gets called on a falling edge (i.e., button press)
 *
 * @irq: The IRQ number that triggered the handler
 * @dev_id: A pointer to our Button_Data struct (used to identify which button)
 */
static irqreturn_t btn_press_irq_handler(int irq, void* dev_id) {
    Button_Data* btn = (Button_Data*)dev_id;
    printk(KERN_INFO "Button (GPIO %d) was pressed\n", btn->gpio_num);
    return IRQ_HANDLED;
}

/**
 * Setup a single GPIO pin and associate it with an IRQ
 *
 * @btn: Pointer to a Button_Data struct representing the button
 *
 * Returns 0 on success, or a negative error code on failure
 */
static int setup_gpio_and_irq (Button_Data* btn) {
    int ret = 0;
    // Check if GPIO is valid
    if (!gpio_is_valid(btn->gpio_num)) {
        printk(KERN_ERR "Invalid GPIO: %d\n", btn->gpio_num);
        return -ENODEV;     // No such device
    }

    // Request control of the GPIO
    ret = gpio_request(btn->gpio_num, btn->name);
    if (ret){
        printk(KERN_ERR "Failed to request GPIO: %d\n", btn->gpio_num);
        return ret;
    }

    // Set the GPIO direction to input
    ret = gpio_direction_input(btn->gpio_num);
    if (ret){
        printk(KERN_ERR "Failed to set GPIO: %d as input\n", btn->gpio_num);
        gpio_free(btn->gpio_num);
        return ret;
    }

    // Export GPIO to sysfs so userspace can access it
    ret = gpio_export(btn->gpio_num, false);
    if (ret) {
        printk(KERN_ERR "Failed to export GPIO %d\n", btn->gpio_num);
        gpio_free(btn->gpio_num);
        return ret;
    }

    // Get IRQ number associated with GPIO
    btn->irq_num = gpio_to_irq(btn->gpio_num);
    if (btn->irq_num < 0){
        printk(KERN_ERR "Failed to get an IRQ for GPIO %d\n", btn->gpio_num);
        gpio_unexport(btn->gpio_num);
        gpio_free(btn->gpio_num);
        return btn->irq_num;
    }
    printk(KERN_INFO "Get IRQ %d for GPIO %d\n", btn->irq_num, btn->gpio_num);

    // Register the IRQ handler
    ret = request_irq(
        btn->irq_num, 
        btn_press_irq_handler, 
        IRQF_TRIGGER_FALLING, 
        btn->name, 
        btn
    );

    if (ret) {
        printk(KERN_ERR "Failed to request IRQ for GPIO%d\n", btn->gpio_num);
        gpio_unexport(btn->gpio_num);
        gpio_free(btn->gpio_num);
    }

    return ret;
}

/**
 * clean - Unexport and free a GPIO, free IRQ
 */
static void cleanup_gpio_and_irq(Button_Data* btn) {
    if (btn->irq_num >= 0)
        free_irq(btn->irq_num, btn);
    gpio_unexport(btn->gpio_num);
    gpio_free(btn->gpio_num);
}

static int __init buzzer_init() {
    printk(KERN_INFO "Requesting GPIOs\n");
    int ret, i;

    for (i = 0; i < NUM_BUTTONS; i++){
        ret = setup_gpio_and_irq(&buttons[i]);
        if (ret) {
            printk(KERN_ERR "Failed to setup GPIO and IRQ for button (GPIO %d)\n", buttons[i].gpio_num);
            goto fail;
        }
    }

    printk(KERN_INFO "Loaded module!\n");
    return 0;

fail:
    // On failure, clean up already-initialized buttons
    for (--i; i >= 0; --i)
        cleanup_gpio_and_irq(&buttons[i]);
    return ret;
}

/**
 * Module exit function
 *
 * Cleans up GPIOs and IRQs
 */
static void __exit buzzer_exit(void) {
    int i;
    for (i = 0; i < NUM_BUTTONS; i++){
        cleanup_gpio_and_irq(&buttons[i]);
    }
    printk(KERN_INFO "Unloaded module!\n");
}

module_init(buzzer_init);
module_exit(buzzer_exit);   // Not called if __init fails

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("Simple GPIO input module");