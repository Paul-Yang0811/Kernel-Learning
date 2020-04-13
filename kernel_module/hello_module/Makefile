KERNELDIR:=/home/user/Myfiles/project/Android/Kernel-Learning/4412_Kernel_3.0

all:
	make -C $(KERNELDIR) M=`pwd` modules

clean:
	make -C $(KERNELDIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m  += hello_driver.o
