#include"scull.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("example scull device driver");

dev_t dev;
struct scull_dev * scull_devices;

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
		printk(KERN_NOTICE "Error %d adding scull %d\n",
				err,
				index);
		return;
	}

	printk(KERN_INFO "Added scull%d device.\n",index);
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
		size_t count, loff_t * f_pos){

	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr;

	int quantum = dev->quantum;
	int qset = dev->qset;

	int itemsize = quantum *qset; //bytes in the listitem
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if(down_interruptible(&dev->sem)){
		return -ERESTARTSYS;
	}
	if(*f_pos >= dev->size){
		goto out;
	}
	//read only to the end of the device
	if(*f_pos + count > dev->size){
		count = dev->size - *f_pos;
	}

	item = (long)*f_pos  / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	//seek to the right position;
	dptr = scull_follow(dev,item);
	
	if(dptr == NULL || !dptr->data || !dptr->data[s_pos]){
		goto out;
	}

	 //read only to the end of the quantum
	 if(count>quantum - q_pos){
		 count = quantum-q_pos;
	 }

	 if(copy_to_user(buff, dptr->data[s_pos] + q_pos,count)){
		 retval = -EFAULT;
		 goto out;
	 }

	 *f_pos +=count;
	 retval = count;


out:

	up(&dev->sem);
	return retval;
}

ssize_t scull_write(struct file * filp,
		const char __user *buff,
		size_t count, loff_t * f_pos){

	struct scull_dev *dev= filp->private_data;
	struct scull_qset * dptr;

	int quantum = dev->quantum;
	int qset = dev->qset;

	int itemsize = quantum *qset;
	int item, s_pos, q_pos,rest;
	ssize_t retval = -ENOMEM;

	if(down_interruptible(&dev->sem)){
		return -ERESTARTSYS;
	}

	//find item, qset index, and offset in quantum
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;

	s_pos = rest / quantum;
	q_pos = rest % quantum;

	//follow dev to the currect position
	dptr = scull_follow(dev,item);
	
	if(dptr==NULL){
		goto out;
	}
	if(!dptr->data){
		//allocate mem
		dptr->data = kmalloc(qset * sizeof(char*),
				GFP_KERNEL);
		if(!dptr->data){
			//fail -> no mem
			goto out;
		}

		//clear mem
		memset(dptr->data,0,qset * sizeof(char*));
	}

	if(!dptr->data[s_pos]){
		dptr->data[s_pos] = kmalloc(quantum,GFP_KERNEL);
		if(!dptr->data[s_pos]){
			goto out;
		}
	}

	//write up to the end of the quantum
	if(count>quantum-q_pos){
		count = quantum - q_pos;
	}

	if(copy_from_user(dptr->data[s_pos]+q_pos,buff,count)){
		retval = -EFAULT;
		goto out;
	}
	*f_pos+=count;
	retval = count;

	if(dev->size < *f_pos){
		dev->size = *f_pos;
	}


	

out:
	up(&dev->sem);
	return retval;
}
struct scull_qset * scull_follow(struct scull_dev *dev, int n){
	struct scull_qset *qs = dev->data;
	if(!qs){
		dev->data = kmalloc(sizeof(struct scull_qset),
				GFP_KERNEL);
		qs = dev->data;
		if(qs == NULL){
			return NULL;
		}
		memset(qs,0,sizeof(struct scull_qset));
	}

	while(n--){
		if(!qs->next){
			qs->next = kmalloc(
					sizeof(struct scull_qset),
						GFP_KERNEL);

			if(qs->next == NULL){
				return NULL;
			}
			memset(qs->next,
				0,
				sizeof(struct scull_qset));
		}
		qs=qs->next;
	}
		

	return qs;
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


	scull_devices = kmalloc(scull_nr_devs * sizeof(
				struct scull_dev),GFP_KERNEL);

	if(!scull_devices){
		result = -ENOMEM;
		goto fail;
	}
	memset(scull_devices,0,
			scull_nr_devs*sizeof(struct scull_dev));


	int i;
	for(i=0;i<scull_nr_devs;i++){
		scull_devices[i].quantum = scull_quantum;
		scull_devices[i].qset = scull_qset;
		scull_setup_dev(&scull_devices[i],i);

	}

	return 0;
fail:

	scull_cleanup();
	return result;
}



static void __exit scull_exit(void){

	scull_cleanup();

	printk(KERN_INFO "Scull exit\n");

}

void scull_cleanup(void){

	if(scull_devices){
		int i;
		for(i=0;i<scull_nr_devs;i++){
			scull_trim(scull_devices +i);
			cdev_del(&scull_devices[i].cdev);
		}

		kfree(scull_devices);
	}

	unregister_chrdev_region(dev,scull_nr_devs);
}

module_init(scull_init);
module_exit(scull_exit);

