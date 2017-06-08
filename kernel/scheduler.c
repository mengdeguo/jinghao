#include "kernel_header.h"

static struct ready_task_rb ready_task = {
    .root           = RB_ROOT,
    .thread_count   = 0,
};

/**************************************************************/
void * get_tcb_container(struct rb_node * node)
{
    tcb * data = container_of(node,tcb,node);
    return (void *)data;
}

int get_tcb_key(void * container)
{
    return ((tcb *)container)->priority;
}

struct rb_node * get_tcb_rb_node(void * container)
{
    return &((tcb *)container)->node;
}

/**************************************************************/

int add_tcb_to_rb_tree(struct rb_root * root,tcb * tcb)
{
    return int_key_insert(root,(void*)tcb,
            get_tcb_container,get_tcb_key,get_tcb_rb_node);
}

tcb * remove_tcb_from_rb_tree(struct rb_root * root,tcb *tcb)
{
    return (struct tcb_t *)int_key_remove(root,tcb->priority,
            get_tcb_container,get_tcb_key,get_tcb_rb_node);
}

tcb * get_highest_priority_tcb(struct rb_root * root)
{
    struct rb_node * node = root->rb_node;

    if(!node) {
        return NULL;
    }

    while(node->rb_left) {
        node = node->rb_left;
    }

    return get_tcb_container(node);
}

/***************************************************************/

int add_to_ready_rb_tree(tcb * tcb)
{
    ready_task.thread_count ++;

    return add_tcb_to_rb_tree(&ready_task.root,tcb);
}

tcb * remove_from_ready_rb_tree(tcb * tcb)
{
    ready_task.thread_count --;

    return remove_tcb_from_rb_tree(&ready_task.root,tcb);
}

tcb * get_from_ready_rb_tree(int priority)
{
    return int_key_search(&ready_task.root,priority,
            get_tcb_container,get_tcb_key);
}

tcb * get_highest_ready_tcb(void)
{
    struct rb_node * node = ready_task.root.rb_node;
    
    if(!node){
        //error, idle thread should be always in ready rb_tree
        return get_from_ready_rb_tree(IDLE_THREAD_PRIORITY);
    }

    return get_highest_priority_tcb(&ready_task.root);
}


