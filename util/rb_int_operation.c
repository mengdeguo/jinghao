#include "rb_int_operation.h"

void *int_key_search(struct rb_root * root, const int key, get_container_t get_container,get_key_t get_key)
{
	struct rb_node *node = root->rb_node;

	while (node) {
        int result;
		void * container = (*get_container)(node);

        result = (*get_key)(container);

        if (key < result)
                node = node->rb_left;
        else if (key > result)
                node = node->rb_right;
        else
                return container;
    }
    
    return NULL;
}

int int_key_insert(struct rb_root * root,void *container,get_container_t get_container,get_key_t get_key, get_rb_node_t get_rb_node)
{
	struct rb_node **new = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new) {
		void *this = (*get_container)(*new);
        int key = (*get_key)(container);
		int result = (*get_key)(this);
        
        parent = *new;
		
        if (key < result)
			new = &((*new)->rb_left);
		else if (key > result)
			new = &((*new)->rb_right);
		else
			return -1;
	}

	/* Add new node and rebalance tree. */
	rb_link_node((*get_rb_node)(container), parent, new);
	rb_insert_color((*get_rb_node)(container), root);
    
    return 0;
}

void *int_key_remove(struct rb_root * root,const int key,get_container_t get_container,get_key_t get_key,get_rb_node_t get_rb_node)
{
    void * container = int_key_search(root,key,get_container,get_key);
    if (container) {
      rb_erase((*get_rb_node)(container), root);
      return container;
    }

    return NULL;

}


