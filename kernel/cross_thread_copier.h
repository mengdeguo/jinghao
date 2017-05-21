#ifndef _CROSS_THREAD_COPIER_H_ 
#define _CROSS_THREAD_COPIER_H_

#include "message_queue.h"

typedef int (*general_cross_thread_task)();
typedef void (*task_result)(int ret,int para_count, ...);
typedef int (*general_invoke)();

typedef struct {
    general_invoke invoke;
}runtime_type_id;

typedef int (*cross_thread_task1)(void *para1);
typedef int (*cross_thread_task2)(void *para1,void *para2);
typedef int (*cross_thread_task3)(void *para1,void *para2,void* para3);
typedef int (*cross_thread_task4)(void *para1,void *para2,void* para3,void *para4);
typedef int (*cross_thread_task5)(void *para1,void *para2,void* para3,void *para4,void *para5);

typedef struct {
    runtime_type_id m_id;
    general_cross_thread_task task;
    task_result call_back;
}general_cross_thread_type;

typedef struct{
    runtime_type_id m_id;
    cross_thread_task1 task;
    task_result call_back;
    void *para1;
}cross_thread_type1;

typedef struct{
    runtime_type_id m_id;
    cross_thread_task2 task;
    task_result call_back;
    void *para1;
    void *para2;
}cross_thread_type2;

typedef struct{
    runtime_type_id m_id;
    cross_thread_task3 task;
    task_result call_back;
    void *para1;
    void *para2;
    void *para3;
}cross_thread_type3;

typedef struct{
    runtime_type_id m_id;
    cross_thread_task4 task;
    task_result call_back;
    void *para1;
    void *para2;
    void *para3;
    void *para4;
}cross_thread_type4;

typedef struct{
    runtime_type_id m_id;
    cross_thread_task5 task;
    task_result call_back;
    void *para1;
    void *para2;
    void *para3;
    void *para4;
    void *para5;
}cross_thread_type5;

/* thread framework */
void thread_loop(void * arg);
int call_task_on_thread_queue(struct message_queue * queue,general_cross_thread_task func,task_result call_back,int para_count, ...);

#endif //_CROSS_THREAD_COPIER_H_

