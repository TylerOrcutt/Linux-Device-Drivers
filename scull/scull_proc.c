#include "scull.h"
#include "scull_proc.h"

static struct proc_dir_entry *entry;

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

static ssize_t  scull_proc_read(
			struct file* fi,
			char __user *buf,
			size_t count,
			loff_t * pos){

	int len=0;
	if(!is_read){
		len = sprintf(buf,"Hello world\n");
	}
	is_read=!is_read;
	
	return len;

}


