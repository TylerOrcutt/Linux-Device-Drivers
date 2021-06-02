scull-objs := scull_main.o scull_proc.o 
obj-m := scull.o

KERNDIR = /lib/modules/$(shell uname -r)/build/
PWD:=$(shell pwd)

default:
	$(MAKE) -C $(KERNDIR) M=$(PWD) modules

clean:
	rm *.o
	rm *.ko
	rm *.mod

