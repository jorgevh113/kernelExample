#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/gpio.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luis Salgado");
MODULE_DESCRIPTION("Un \"character device Kernel Module\" para acceso a GPIO");
MODULE_VERSION("0.1O");

#define DEVICE_NAME "charDev_GPIO"
#define BUFF_SIZE 1024

#define GPIO_1	0x6000d000
#define GPIO_2	0x6000d100
#define GPIO_3	0x6000d200
#define GPIO_4	0x6000d300
#define GPIO_5	0x6000d400
#define GPIO_6	0x6000d500
#define GPIO_7	0x6000d600
#define GPIO_8	0x6000d700

static dev_t charDev_readWrite_dev;
static struct cdev charDev_readWrite_cdev;
static int open_count = 0;

static char charDev_readWrite_buff[BUFF_SIZE];
static int buff_count=0;

struct GPIO_mem {
	uint32_t CNF[4];
	uint32_t OE[4];
	uint32_t OUT[4];
	uint32_t IN[4];
	uint32_t INT_STA[4];
	uint32_t INT_ENB[4];
	uint32_t INT_LVL[4];
	uint32_t INT_CLR[4];
};
uint32_t cnf,oe,out,in,int_sta,int_enb,int_lvl,int_clr;

void __iomem *memBase;

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
	charDev_readWrite_buff[0]='L';
	charDev_readWrite_buff[1]='e';
	charDev_readWrite_buff[2]='c';
	charDev_readWrite_buff[3]='t';
	charDev_readWrite_buff[4]='u';
	charDev_readWrite_buff[5]='r';
	charDev_readWrite_buff[6]='a';
	charDev_readWrite_buff[7]=0;
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
	printk(KERN_ERR "hwkm --> writting\n");
	buff_count=count;
	return count;
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
	if (!request_mem_region((unsigned long)GPIO_3,1,DEVICE_NAME)) {
		printk(KERN_INFO "hwkm --> request_mem_region ERROR\n");
	} else {
		printk(KERN_INFO "hwkm --> request_mem_region SUCCEEDED!\n");
		//memBase=ioremap(GPIO_3,256);
		//cnf=ioread32(memBase+2);
		//cnf=cnf|0x32768;
		//iowrite32(cnf,memBase+2);
	}

	if ((gpio_request(16,"jetson-out16")<0) ||
	     (gpio_request(17,"jetson-out17")<0) ||
	      (gpio_request(18,"jetson-out18")<0)) {
		printk(KERN_INFO "hwkm --> gpio_request ERROR\n");
	}
	if ((gpio_direction_output(16,1)<0) ||
	     (gpio_direction_output(17,1)<0) ||
	       (gpio_direction_output(18,0)<0)) {
		printk(KERN_INFO "hwkm --> gpio_direction ERROR\n");
		gpio_free(79);
	}

	printk(KERN_INFO "hwkm --> inited, loaded and running :)\n");
	return 0;
}

static void __exit hwkm_exit(void) {
	cdev_del(&charDev_readWrite_cdev);
	unregister_chrdev_region(charDev_readWrite_dev,1);
	//iounmap(memBase);
	gpio_set_value(16,1);
	gpio_set_value(17,1);
	gpio_set_value(18,1);
	gpio_free(16);
	gpio_free(17);
	gpio_free(18);
	printk(KERN_INFO "hwkm --> unloaded and exited\n");
}

module_init(hwkm_init);
module_exit(hwkm_exit);
