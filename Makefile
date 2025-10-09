obj-m += driver.o
driver-objs := source/driver.o source/buzzer.o source/gpio_pin.o source/reset_button.o

EXTRA_CFLAGS := -I$(PWD)/include

KDIR := /home/cat/linux-0afb5e98488aed7017b9bf321b575d0177feb7ed
PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean