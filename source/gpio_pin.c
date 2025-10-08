#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/kernel.h>

#include "gpio_pin.h"

// Returns the GPIO number associated with the pin
unsigned int get_gpio_num(GPIO_Pin* pin) {
    return pin->gpio_num;
}

const char* get_name(GPIO_Pin* pin) {
    return pin->name;
}

// Returns the IRQ number associated with the pin
int get_irq_num(GPIO_Pin* pin) {
    return pin->irq_num;
}

// Assigns an IRQ number to the pin
void set_irq_num(GPIO_Pin* pin, int irq_num) {
    pin->irq_num = irq_num;
}

/*
 * setup_gpio - Configures a GPIO pin as input or output
 * @pin: The GPIO pin structure
 * @direction: "input" or "output"
 *
 * Returns: 0 on success, negative errno on failure
 */
int setup_gpio(GPIO_Pin* pin, const char* direction) {
    int ret = 0;

    if (!gpio_is_valid(pin->gpio_num)) {
        printk(KERN_ERR "Invalid GPIO: %d\n", pin->gpio_num);
        return -ENODEV;
    }

    // Request ownership of the GPIO
    ret = gpio_request(pin->gpio_num, pin->name);
    if (ret) {
        printk(KERN_ERR "Failed to request GPIO: %d\n", pin->gpio_num);
        return ret;
    }

    // Configure direction
    if (strcmp(direction, "input") == 0) {
        ret = gpio_direction_input(pin->gpio_num);
    } else if (strcmp(direction, "output") == 0) {
        ret = gpio_direction_output(pin->gpio_num, 0);
    } else {
        printk(KERN_ERR "Invalid direction (%s) for GPIO: %d\n", direction, pin->gpio_num);
        gpio_free(pin->gpio_num);
        return -EINVAL;
    }

    if (ret) {
        printk(KERN_ERR "Failed to set GPIO: %d as %s\n", pin->gpio_num, direction);
        gpio_free(pin->gpio_num);
        return ret;
    }

    // Export the GPIO to sysfs (e.g., /sys/class/gpio/gpioXX)
    // This allows user-space access if needed
    ret = gpio_export(pin->gpio_num, false);
    if (ret) {
        printk(KERN_ERR "Failed to export GPIO %d\n", pin->gpio_num);
        gpio_free(pin->gpio_num);
    }

    return ret;
}

/*
 * cleanup_gpio - Frees resources associated with a GPIO pin
 * @pin: The GPIO pin structure
 *
 * This function:
 *   - Frees IRQ if one was set
 *   - Unexports GPIO from sysfs
 *   - Releases the GPIO
 */
void cleanup_gpio(GPIO_Pin* pin) {
    if (pin->irq_num >= 0)
        free_irq(pin->irq_num, pin);
    gpio_unexport(pin->gpio_num);
    gpio_free(pin->gpio_num);
}