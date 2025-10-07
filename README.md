# Raspberry Pi GPIO Interrupt Kernel Module

This project implements a Linux kernel module (device driver) in C for the Raspberry Pi that:

1. Uses two physical buttons connected to GPIO pins.
2. Detects which button is pressed **first**.
3. Uses **interrupts** instead of polling.
4. Is written for educational purposes using the **legacy GPIO API**.

## Kernel Version Compatibility

Tested on Raspberry Pi kernel `6.1.21-v8+` running on Debian.  
Kernel headers for this version are **not available via APT**, so the kernel source must be manually prepared.

## Setup Instructions

To prepare the kernel build environment on the Raspberry Pi:

```bash
git clone --depth=1 --branch rpi-6.1.y https://github.com/raspberrypi/linux.git
cd linux
make bcm2711_defconfig
make modules_prepare
# make modules
```

## Transferring and Building the Module
Transfer your module source (e.g. GPIO_Kernel/) to the Pi:
```bash
scp -r /path/to/local/folder/ user@raspberrypi:/path/to/remote/folder/
```

On the Raspberry Pi:
```bash
make
sudo insmod buzzer.ko
dmesg | tail -10   # Check for logs
sudo rmmod buzzer
make clean
```

## Notes
- This project uses the legacy GPIO API (`gpio_request`, `gpio_to_irq`, etc.) for clarity and learning purposes.
- Hardware setup uses pull-up resistors for button detection (active-low logic).
- See dmesg output for identifying which button was pressed first.


## User Testing
To verify the hardware wiring and GPIO behavior outside of the kernel module, you can manually test the GPIO pins from userspace:

```bash
# Export GPIO 27 to userspace
echo 27 | sudo tee /sys/class/gpio/export

# Set GPIO 27 direction to input
echo in | sudo tee /sys/class/gpio/gpio27/direction

# Continuously monitor GPIO 27 value every 0.2 seconds
watch -n 0.2 cat /sys/class/gpio/gpio27/value

# When finished, unexport GPIO 27
echo 27 | sudo tee /sys/class/gpio/unexport
```