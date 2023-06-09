#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jorge Villegas Fernando Cuéllar TecMTY");
MODULE_DESCRIPTION("Un \"character device Kernel Module\" usando gpioLib");
MODULE_VERSION("0.1O");

#define DEVICE_NAME "charDev_KernelModule"
#define BUFF_SIZE 1024

static dev_t charDev_dev;
static struct cdev charDev_cdev;
static int open_count = 0;

static char charDev_buff[BUFF_SIZE];
static int buff_count=0;
static unsigned int sensorArray[16];
static unsigned int sensorIndex=0;

static int charDev_open(struct inode *inode, struct file *f) {
	int i = 0;	
	if (open_count>0) {
		return -EBUSY;
	}

	//Inicializar valores del sensorArray ...
	for (i= 0; i< sizeof(sensorArray);i++){
		sensorArray[i] = 0;
	}
	open_count++;
	return 0;
}

static int charDev_release(struct inode *inode, struct file *f) {
	open_count--;
	return 0;
}

static ssize_t charDev_read(struct file *f, char __user *buff, size_t count, loff_t *pos) {
	if (count<0) {
		return 0;
	}

	//Regresar valor del sensor seleccionado (sensorIndex) ...
	charDev_buff[0] = sensorArray[sensorIndex];
	copy_to_user(buff, charDev_buff, 1);
	return 1;
}

static ssize_t charDev_write(struct file *f, const char __user *buff, size_t count, loff_t *pos) {
	if (count<0) {
		return 0;
	}

	//Guardar el índice que se utilizará para la lectura de datos del sensorArray
	copy_from_user(charDev_buff, buff, 1);
	sensorIndex = charDev_buff[0];
	return count;
}

irqreturn_t irqHandler(int irq, void *dev_id) {
	//Adquisición de datos en base al evento de interrupción ...
	sensorArray[0] = gpio_get_value(IRQ_01);
	sensorArray[1] = gpio_get_value(IRQ_02);
	sensorArray[2] = gpio_get_value(IRQ_03);
	sensorArray[3] = gpio_get_value(IRQ_04);
	sensorArray[4] = gpio_get_value(IRQ_05);
	sensorArray[5] = gpio_get_value(IRQ_06);
	sensorArray[6] = gpio_get_value(IRQ_07);
	sensorArray[7] = gpio_get_value(IRQ_08);
	sensorArray[8] = gpio_get_value(IRQ_09);
	sensorArray[9] = gpio_get_value(IRQ_10);
	sensorArray[10] = gpio_get_value(IRQ_11);
	sensorArray[11] = gpio_get_value(IRQ_12);
	sensorArray[12] = gpio_get_value(IRQ_13);
	sensorArray[13] = gpio_get_value(IRQ_14);
	sensorArray[14] = gpio_get_value(IRQ_15);
	sensorArray[15] = gpio_get_value(IRQ_16);
	return IRQ_HANDLED;
}

//*****************************************************************************
static struct file_operations charDev_fops = {
	.owner = THIS_MODULE,
	.open = charDev_open,
	.release = charDev_release,
	.read = charDev_read,
	.write = charDev_write,
};

//*****************************************************************************
static int __init hwkm_init(void) {
	int irq_number;
	int ret=alloc_chrdev_region(&charDev_dev,0,1,DEVICE_NAME);
	if (ret<0) {
		return ret;
	}

	cdev_init(&charDev_cdev,&charDev_fops);
	ret=cdev_add(&charDev_cdev,charDev_dev,1);
	if (ret<0) {
		unregister_chrdev_region(charDev_dev,1);
		return ret;
	}

	if ((gpio_request(IRQ_01,"jetson-in01")<0) ||
		(gpio_request(IRQ_02,"jetson-in02")<0) ||
		(gpio_request(IRQ_03,"jetson-in03")<0) ||
		(gpio_request(IRQ_04,"jetson-in04")<0) ||
		(gpio_request(IRQ_05,"jetson-in05")<0) ||
		(gpio_request(IRQ_06,"jetson-in06")<0) ||
		(gpio_request(IRQ_07,"jetson-in07")<0) ||
		(gpio_request(IRQ_08,"jetson-in08")<0) ||
		(gpio_request(IRQ_09,"jetson-in09")<0) ||
		(gpio_request(IRQ_10,"jetson-in10")<0) ||
		(gpio_request(IRQ_11,"jetson-in11")<0) ||
		(gpio_request(IRQ_12,"jetson-in12")<0) ||
		(gpio_request(IRQ_13,"jetson-in13")<0) ||
		(gpio_request(IRQ_14,"jetson-in14")<0) ||
		(gpio_request(IRQ_15,"jetson-in15")<0) ||
		(gpio_request(IRQ_16,"jetson-in16")<0) ) {
			return -EBUSY;
		}
	if ((gpio_direction_input(IRQ_01)<0) ||
		(gpio_direction_input(IRQ_02)<0) ||
		(gpio_direction_input(IRQ_03)<0) ||
		(gpio_direction_input(IRQ_04)<0) ||
		(gpio_direction_input(IRQ_05)<0) ||
		(gpio_direction_input(IRQ_06)<0) ||
		(gpio_direction_input(IRQ_07)<0) ||
		(gpio_direction_input(IRQ_08)<0) ||
		(gpio_direction_input(IRQ_09)<0) ||
		(gpio_direction_input(IRQ_10)<0) ||
		(gpio_direction_input(IRQ_11)<0) ||
		(gpio_direction_input(IRQ_12)<0) ||
		(gpio_direction_input(IRQ_13)<0) ||
		(gpio_direction_input(IRQ_14)<0) ||
		(gpio_direction_input(IRQ_15)<0) ||
		(gpio_direction_input(IRQ_16)<0)) {
			gpio_free(IRQ_01);
			gpio_free(IRQ_02);
			gpio_free(IRQ_03);
			gpio_free(IRQ_04);
			gpio_free(IRQ_05);
			gpio_free(IRQ_06);
			gpio_free(IRQ_07);
			gpio_free(IRQ_08);
			gpio_free(IRQ_09);
			gpio_free(IRQ_10);
			gpio_free(IRQ_11);
			gpio_free(IRQ_12);
			gpio_free(IRQ_13);
			gpio_free(IRQ_14);
			gpio_free(IRQ_15);
			gpio_free(IRQ_16);
			return -EBUSY;
	}

	irq_number = gpio_to_irq(IRQ_01);
	irq_number = gpio_to_irq(IRQ_02);
	irq_number = gpio_to_irq(IRQ_03);
	irq_number = gpio_to_irq(IRQ_04);
	irq_number = gpio_to_irq(IRQ_05);
	irq_number = gpio_to_irq(IRQ_06);
	irq_number = gpio_to_irq(IRQ_07);
	irq_number = gpio_to_irq(IRQ_08);
	irq_number = gpio_to_irq(IRQ_09);
	irq_number = gpio_to_irq(IRQ_10);
	irq_number = gpio_to_irq(IRQ_11);
	irq_number = gpio_to_irq(IRQ_12);
	irq_number = gpio_to_irq(IRQ_13);
	irq_number = gpio_to_irq(IRQ_14);
	irq_number = gpio_to_irq(IRQ_15);
	irq_number = gpio_to_irq(IRQ_16);
	if ((request_threaded_irq(irq_number,NULL,&irqHandler,IRQF_SHARED|IRQF_TRIGGER_RISING|IRQF_ONESHOT,DEVICE_NAME,NULL)<0)
			) {	
	} else {
		printk(KERN_INFO "KernelModule --> irq request error!\n");
	}

	return 0;
}

static void __exit hwkm_exit(void) {
	cdev_del(&charDev_cdev);
	unregister_chrdev_region(charDev_dev,1);
	gpio_free(IRQ_01);
	gpio_free(IRQ_02);
	gpio_free(IRQ_03);
	gpio_free(IRQ_04);
	gpio_free(IRQ_05);
	gpio_free(IRQ_06);
	gpio_free(IRQ_07);
	gpio_free(IRQ_08);
	gpio_free(IRQ_09);
	gpio_free(IRQ_10);
	gpio_free(IRQ_11);
	gpio_free(IRQ_12);
	gpio_free(IRQ_13);
	gpio_free(IRQ_14);
	gpio_free(IRQ_15);
	gpio_free(IRQ_16);

}

module_init(hwkm_init);
module_exit(hwkm_exit);
