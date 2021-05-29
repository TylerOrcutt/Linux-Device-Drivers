#include <linux/init.h>
#include<linux/module.h>
#include<linux/errno.h> //error values
#include<linux/fs.h> //char driver
MODULE_LICENSE("Dual BSD/GPL");


static int __init scull_init(void){

	printk(KERN_ALERT "Init scull\n");
	return 0;

}

static void __exit scull_exit(void){
	printk(KERN_ALERT "Scull exit\n");
}

module_init(scull_init);
module_exit(scull_exit);

