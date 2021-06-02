#ifndef _SCULL_PROC_H_
#define _SCULL_PROC_H_
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/proc_fs.h>


extern int init_scull_proc(void);
extern int remove_scull_proc(void);

static int scull_proc_open(struct inode *,
	       	struct file*);

static ssize_t  scull_proc_read(
			struct file* fi,
			char __user *buf,
			size_t count,
			loff_t * pos);



#endif
