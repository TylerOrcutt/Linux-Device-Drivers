#ifndef _SCULL_PROC_H_
#define _SCULL_PROC_H_
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/proc_fs.h>


extern int init_scull_proc(void);
extern int remove_scull_proc(void);


#endif
