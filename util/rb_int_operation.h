#ifndef _RB_INT_OPERATION_H_
#define _RB_INT_OPERATION_H_

#include "rbtree_augmented.h"

typedef void * (*get_container_t)(struct rb_node * node);
typedef int (*get_key_t)(void * container);
typedef struct rb_node * (*get_rb_node_t)(void * container);

void *int_key_search(struct rb_root * root, const int key, get_container_t get_container,get_key_t get_key);
int int_key_insert(struct rb_root * root,void *container,get_container_t get_container,get_key_t get_key, get_rb_node_t get_rb_node);
void *int_key_remove(struct rb_root * root,const int key,get_container_t get_container,get_key_t get_key,get_rb_node_t get_rb_node);

#endif //_RB_INT_OPERATION_H_
