#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
MODULE_LICENSE("Dual BSD/GPL");

static char *whom = "world";
static int howmany = 1;
module_param(howmany,int,S_IRUGO);
module_param(whom,charp,S_IRUGO);

static int __init hellop_init(void){

	int i=0;
	while(i<howmany){
		printk(KERN_ALERT "Hello, %s!\n",whom);
		i++;
	}

	return 0;
}

static void __exit hellop_exit(void){
	printk(KERN_ALERT "Good bye, %s!\n",whom);

}

module_init(hellop_init);
module_exit(hellop_exit);

