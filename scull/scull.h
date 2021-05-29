#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/init.h>
#include<linux/module.h>
#include<linux/errno.h> 
#include<linux/fs.h> 
#include<linux/kdev_t.h>//dev_t
#include<linux/moduleparam.h> //module params
#include<linux/cdev.h>//cdev
#include<linux/kernel.h>
#include<linux/slab.h>//kfree+kmalloc

#define SCULL_MAJOR 	0
#define SCULL_MINOR 	0
#define SCULL_NR_DEVS 	4
#define SCULL_QUANTUM 	4000
#define SCULL_QSET 	1000

extern int scull_major;
extern int scull_minor;
extern int scull_nr_devs;

extern int scull_quantum;
extern int scull_qset;
extern struct scull_dev * scull_devices;


struct scull_dev{
	struct scull_qset *data;//pointer to first set
	int quantum;
	int qset;
	unsigned long size;
	unsigned int access_key;
	struct semaphore sem;
	struct cdev cdev;
};

struct scull_qset{
	void **data;
	struct scull_qset *next;
};


static int __init scull_init(void);
static void __exit scull_exit(void);

static void  scull_setup_dev(struct scull_dev * dev,
		int index);


int scull_open(struct inode *inode, struct file *filp);
int scull_release(struct inode *,struct file*);


ssize_t scull_read(struct file *,
		char __user *,
		size_t,
		loff_t *);

ssize_t scull_write(struct file *,
		const char __user *,
		size_t,
		loff_t *);
struct scull_qset * scull_follow(struct scull_dev *, int);


int scull_trim(struct scull_dev *);

void scull_cleanup(void);

#endif
