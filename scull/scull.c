#include"scull.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("example scull device driver");

dev_t dev;

int scull_major=SCULL_MAJOR;//major verison
int scull_minor=SCULL_MINOR;//minor verison;
int scull_nr_devs=SCULL_NR_DEVS;//number of devices

int scull_quantum = SCULL_QUANTUM;
int scull_qset = SCULL_QSET;


module_param(scull_major,int,S_IRUGO);
module_param(scull_minor,int,S_IRUGO);

struct file_operations scull_fops = {
	.owner	 = THIS_MODULE,
	.open	 = scull_open,
	.release = scull_release,
	.read 	 = scull_read,
	.write	 = scull_write
};

static void scull_setup_dev(struct scull_dev *dev,
			int index){

	int err, devno = MKDEV(scull_major,scull_minor+index);

	cdev_init(&dev->cdev,&scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;

	err = cdev_add(&dev->cdev,devno,1);

	if(err){
		printk(KERN_NOTICE "Error %d adding scull %d",
				err,
				index);
	}
}

int scull_open(struct inode *inode, struct file *filp){
	struct scull_dev *dev;
	
	dev = container_of(inode->i_cdev,struct scull_dev,cdev);
	filp->private_data=dev;

	if( (filp->f_flags & O_ACCMODE) == O_WRONLY){
		scull_trim(dev);
	}

	return 0;
}

ssize_t scull_read(struct file * filp, 
		char __user * buff, 
		size_t count, loff_t * offp){
	return 0;
}

ssize_t scull_write(struct file * filp,
		const char __user *buff,
		size_t count, loff_t * offp){
	return 0;
}

int scull_release(struct inode* inode, struct file *filp){
	return 0;
}

int scull_trim(struct scull_dev * dev){

	struct scull_qset *nxt, *dptr;

	int qset = dev->qset;
	int i;

	for(dptr=dev->data;dptr;dptr=nxt){

		if(dptr->data){
			for(i=0;i<qset;i++){
				kfree(dptr->data[i]);
			}
		}

		kfree(dptr->data);
		dptr->data=NULL;

	}

	dev->size=0;
	dev->quantum = scull_quantum;
	dev->qset = scull_qset;
	dev->data = NULL;

	return 0;
}

static int __init scull_init(void){

	printk(KERN_INFO "Init scull\n");

	int result;

	if(scull_major>0){

		dev = MKDEV(scull_major,scull_minor);
		result = register_chrdev_region(dev,scull_nr_devs,"scull");

	}else{
		result = alloc_chrdev_region(&dev,
				scull_minor,
				scull_nr_devs,"scull");

		scull_major=MAJOR(dev);

	}

	if(result<0){

		printk(KERN_WARNING "scull: can't get major %d\n",
				scull_major);

		return result;
	}

	return 0;
}



static void __exit scull_exit(void){

	cleanup();

	unregister_chrdev_region(dev,scull_nr_devs);

	printk(KERN_INFO "Scull exit\n");

}

void cleanup(void){


}

module_init(scull_init);
module_exit(scull_exit);

