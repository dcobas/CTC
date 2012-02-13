CPU=L865
include /acc/dsc/src/co/Make.common

# KVER=2.6.24.7-rt21
# KVER=2.6.24.7-rt27
# KVER=2.6.29.4-rt15
# KVER=2.6.31.6-rt19

KVER=2.6.24.7-rt27

KERNELSRC=/acc/sys/$(CPU)/usr/src/kernels/$(KVER)
DRIVER_NAME = "ctc"
CFLAGS += -I /acc/local/L865/include/
CFLAGS += -g -Wall -fPIC

all: modules

modules: 
	cp Module.symvers.vmebus Module.symvers
	make -C $(KERNELSRC) M=`pwd` KVER=$(KVER) CPU=$(CPU) 'DRIVER_NAME=$(DRIVER_NAME)' modules
clean:
	rm -f *.o *.a *.so
	make -C $(KERNELSRC) M=`pwd` clean

