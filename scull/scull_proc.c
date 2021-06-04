#include "scull.h"
#include "scull_proc.h"

static struct proc_dir_entry *entry;

static int scull_proc_open(struct inode *,
	       	struct file*);


static struct scull_qset * scull_nav_to_item(struct scull_dev *dev,
		int c);
static ssize_t  scull_proc_read(
			struct file* fi,
			char __user *buf,
			size_t count,
			loff_t * pos);

static struct proc_ops sp_ops = {
	.proc_open 	= scull_proc_open,
	.proc_read 	= scull_proc_read,
};

int init_scull_proc(void){

	printk(KERN_INFO "scull: starting proc file\n");

	//deprecated procfile method
//	create_proc_read_entry("scullmem",//name
//			0,//mode
//			NULL,//base dir
//			scull_read_proc,//read func
//			NULL);//client data

	entry = proc_create("scullmem",0,NULL,&sp_ops);

	if(!entry){

		printk(KERN_ERR "scull: failed to create procfile entry\n");
		return -ERESTARTSYS;

	}

	printk(KERN_INFO "scull: added procfile entry\n");
	return 0;
}

int remove_scull_proc(void){
	
	if(entry){
		proc_remove(entry);
	}

//	remove_proc_entry("scullseq",//name
//			 NULL//parrent dir
//			);

	return 0;
}

static int scull_proc_open(struct inode *inode, struct file *fops){


	return 0;
}

static bool is_read=0;

static struct scull_qset * scull_nav_to_item(struct scull_dev *dev, int c){

	struct scull_qset * qset = dev->data;

	while(c>=0){

		if(!qset){
			goto fail;
		}

		if(c==0){
			return qset;
		}

		qset=qset->next;
		c--;
	}

fail:
	return NULL;
}
static ssize_t  scull_proc_read(
			struct file* fi,
			char __user *buf,
			size_t count,
			loff_t * pos){

	int len=0;
	struct scull_dev *dev;
	int item,quantum, qset;
	int itemsize=0,rest,s_pos,q_pos;

	dev = &scull_devices[0];
	quantum = dev->quantum;
	qset = dev->qset;

	itemsize = quantum *qset;
	item = (long)*pos / itemsize;
	rest = (long)*pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;



	if(mutex_lock_interruptible(&dev->mu)){
		printk(KERN_ERR "scull: unable to aquire scull0 lock\n");
		return 0;
	}

	struct scull_qset * sqset = scull_nav_to_item(dev,item);
	if(sqset == NULL){
		//no data
		printk(KERN_INFO "scull: dev0 has no data at position:%d\n",(int)*pos);

	//	if(!is_read){
	//		len = sprintf(buf,
	//		"Found %d scull devices\nItemSize:%d\n,count%d\n",
	//		       	scull_nr_devs,
	//		       	itemsize,
	//			(int)count);
	//	}
		goto out;
	}

	if(!sqset->data  || !sqset->data[s_pos]){
		goto out;
	}

	printk(KERN_INFO "scull: found data in scull0\n");

	if(count>quantum - q_pos){
		count = quantum -q_pos;
	}

	if(copy_to_user(buf,sqset->data[s_pos] + q_pos,count)){
		 len = -EFAULT;
		 goto out;
	}
	*pos +=count;
	len = count;
	
	
out:
	//is_read=!is_read;
	mutex_unlock(&dev->mu);
	return len;


}


