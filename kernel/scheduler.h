#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <stdint.h>
#include "rb_int_operation.h"


/*
 * rb-tree which stores all the ready task
 * */
struct ready_task_rb
{
    struct rb_root root;
    size_t thread_count;
};


/*
 * tool functions for rb-tree operation 
 * */
void * get_tcb_container(struct rb_node * node);
int get_tcb_key(void * container);
struct rb_node * get_tcb_rb_node(void * container);

int add_tcb_to_rb_tree(struct rb_root * root,tcb * tcb);
tcb * remove_tcb_from_rb_tree(struct rb_root * root,tcb *tcb);
tcb * get_highest_priority_tcb(struct rb_root * root);

int add_to_ready_rb_tree(tcb * tcb);
tcb* remove_from_ready_rb_tree(tcb * tcb);
tcb* get_highest_ready_tcb(void);
tcb* get_from_ready_rb_tree(int priority);

#endif //_SCHEDULER_H_
