#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luis Salgado");
MODULE_DESCRIPTION("Un \"character device Kernel Module\" para acceso a GPIO a través del gpioLib");
MODULE_VERSION("0.1O");

#define DEVICE_NAME "charDev_libGPIOirq"
#define BUFF_SIZE 1024

static dev_t charDev_readWrite_dev;
static struct cdev charDev_readWrite_cdev;
static int open_count = 0;

static char charDev_readWrite_buff[BUFF_SIZE];
static int buff_count=0;

static int charDev_readWrite_open(struct inode *inode, struct file *f) {
	if (open_count>0) {
		return -EBUSY;
	}
	open_count++;
	return 0;
}

static int charDev_readWrite_release(struct inode *inode, struct file *f) {
	open_count--;
	return 0;
}

static ssize_t charDev_readWrite_read(struct file *f, char __user *buff, size_t count, loff_t *pos) {
	if (count<0) {
		return 0;
	}
	charDev_readWrite_buff[0] = gpio_get_value(19);
	if (copy_to_user(buff, charDev_readWrite_buff, count)) {
		return -EFAULT;
	}
	printk(KERN_ERR "hwkm --> reading\n");
	return count;
}

static ssize_t charDev_readWrite_write(struct file *f, const char __user *buff, size_t count, loff_t *pos) {
	if (count<0) {
		return 0;
	}
	if (copy_from_user(charDev_readWrite_buff, buff, count)) {
		return -EFAULT;
	}
	switch(charDev_readWrite_buff[0]) {
		case 'r':
				gpio_set_value(16,0);
				gpio_set_value(17,1);
				gpio_set_value(18,1);
				break;
		case 'g':
				gpio_set_value(16,1);
				gpio_set_value(17,0);
				gpio_set_value(18,1);
				break;
		case 'b':
				gpio_set_value(16,1);
				gpio_set_value(17,1);
				gpio_set_value(18,0);
				break;
	}

	printk(KERN_ERR "hwkm --> writting\n");
	buff_count=count;
	return count;
}

irqreturn_t irqHandler(int irq, void *dev_id) {
	printk(KERN_ERR "hwkm --> interrupt!\n");
	return IRQ_HANDLED;
}

//*****************************************************************************
static struct file_operations charDev_readWrite_fops = {
	.owner = THIS_MODULE,
	.open = charDev_readWrite_open,
	.release = charDev_readWrite_release,
	.read = charDev_readWrite_read,
	.write = charDev_readWrite_write,
};

//*****************************************************************************
static int __init hwkm_init(void) {
	int irq_number;
	int ret=alloc_chrdev_region(&charDev_readWrite_dev,0,1,DEVICE_NAME);
	if (ret<0) {
		printk(KERN_ERR "hwkm --> Error al solicitar el devNumber para este dispositivo\n");
		return ret;
	}
	printk(KERN_ERR "hwkm --> MajorDevNumber=%d\n",MAJOR(charDev_readWrite_dev));

	cdev_init(&charDev_readWrite_cdev,&charDev_readWrite_fops);
	ret=cdev_add(&charDev_readWrite_cdev,charDev_readWrite_dev,1);
	if (ret<0) {
		unregister_chrdev_region(charDev_readWrite_dev,1);
		printk(KERN_ERR "Error al añadir este dispositivo al \"filesystem\"\n");
		return ret;
	}

	if ((gpio_request(16,"jetson-out16")<0) ||
	     (gpio_request(17,"jetson-out17")<0) ||
	      (gpio_request(18,"jetson-out18")<0) ||
	       (gpio_request(19,"jetson-in19")<0) ||
	        (gpio_request(20,"jetson-in20")<0) ) {
		printk(KERN_INFO "hwkm --> gpio_request ERROR\n");
		return -EBUSY;
	}
	if ((gpio_direction_output(16,1)<0) ||
	     (gpio_direction_output(17,1)<0) ||
	      (gpio_direction_output(18,0)<0) ||
	       (gpio_direction_input(19)<0) ||
	        (gpio_direction_input(20)<0)) {
		printk(KERN_INFO "hwkm --> gpio_direction ERROR\n");
		gpio_free(16);
		gpio_free(17);
		gpio_free(18);
		gpio_free(19);
		gpio_free(20);
		return -EBUSY;
	}

	irq_number = gpio_to_irq(20);
	if (request_threaded_irq(irq_number,NULL,&irqHandler,IRQF_SHARED|IRQF_TRIGGER_RISING|IRQF_ONESHOT,DEVICE_NAME,NULL)<0) {
		printk(KERN_ERR "hwkm --> Unable to allocate IRQ\n");
	} else {
		printk(KERN_INFO "hwkm --> GPIO#20 - IRQ#%d\n",irq_number);
	}

	printk(KERN_INFO "hwkm --> inited, loaded and running :)\n");
	return 0;
}

static void __exit hwkm_exit(void) {
	cdev_del(&charDev_readWrite_cdev);
	unregister_chrdev_region(charDev_readWrite_dev,1);
	gpio_set_value(16,1);
	gpio_set_value(17,1);
	gpio_set_value(18,1);
	gpio_free(16);
	gpio_free(17);
	gpio_free(18);
	gpio_free(19);
	gpio_free(20);
	printk(KERN_INFO "hwkm --> unloaded and exited\n");
}

module_init(hwkm_init);
module_exit(hwkm_exit);
