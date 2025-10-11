# 🛠️ GPIO Interrupt Kernel Module

This project implements a **Linux kernel module** (device driver) in C for the Raspberry Pi to simulate a **buzzer system**.

The system is modular and includes:

1. **GPIO_Pin** 
    - Handles requesting GPIO pins and registering interrupts.  
2. **Buzzer** 
    - Composed of a button and an LED for each team. 
    - Interrupt triggers, the corresponding LED lights up, and further presses are disabled until reset (via `atomic_t` flag)
3. **Reset Button**
    - Resets the buzzer system for the next round.  

**Goal:** Two teams race to answer a question. The first team to press their button lights up their LED. A reset is required to start the next round.

---

## ⚡ Features

- Detects **which button is pressed first** using interrupts.  
- Modular design with reusable components for GPIO pins, buzzers, and reset functionality.  
- Written using the **legacy GPIO API** (`gpio_request`, `gpio_to_irq`, etc.) for learning purposes.  
- Avoids CPU-intensive polling.  
- Active-low button logic with pull-up resistors.

---

## 🧰 Hardware Required

- **3 Buttons:** Use pull-up resistors; pressing a button connects the pin to GND (active-low).  
- **2 LEDs:** Indicate which team pressed first.  
- **Raspberry Pi 4**

---

## 🖥️ Kernel Source

### Version

- Tested on Raspberry Pi kernel **`6.1.21-v8+`** (Debian).  
- **Note:** Kernel headers are **not available via APT**; the kernel source must be manually prepared.

---

## ⚙️ Installation Instructions

### 1️⃣ Prepare the Kernel Build Environment

```bash
wget https://github.com/raspberrypi/linux/archive/0afb5e98488aed7017b9bf321b575d0177feb7ed.zip
unzip 0afb5e98488aed7017b9bf321b575d0177feb7ed.zip
cd linux-0afb5e98488aed7017b9bf321b575d0177feb7ed

make bcm2611_defconfig
make modules_prepare
make kernelversion
```

## Transferring and Building the Module
Transfer your module source (e.g. GPIO_Kernel/) to the Pi:
```bash
scp -r /path/to/local/folder/ user@raspberrypi:/path/to/remote/folder/
```

On the Raspberry Pi:
```bash
make
sudo insmod driver.ko
dmesg | tail -10   # Check for logs
sudo rmmod driver
make clean
```

## User Testing
You can test GPIO behavior from userspace to verify wiring:

```bash
# Export GPIO 26 to userspace
echo 26 | sudo tee /sys/class/gpio/export

# Set GPIO 26 direction to input
echo in | sudo tee /sys/class/gpio/gpio26/direction

# Continuously monitor GPIO 26 value every 0.2 seconds
watch -n 0.2 cat /sys/class/gpio/gpio26/value

# When finished, unexport GPIO 26
echo 26 | sudo tee /sys/class/gpio/unexport
```
