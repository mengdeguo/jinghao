#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "cross_thread_copier.h"
#include "message_queue.h"
#include "platform.h"

static int general_run(general_cross_thread_task task)
{
    if(!task) {
        return -1;
    }

   return (*(task))();
}

static int run1(general_cross_thread_task task,void * para1)
{   
    if(!task) {
        return -1;
    }

    return (*((cross_thread_task1)task))(para1);
}

static int run2(general_cross_thread_task task,void * para1,void *para2)
{
    if(!task) {
        return -1;
    }

    return (*((cross_thread_task2)task))(para1,para2);
}

static int run3(general_cross_thread_task task,void * para1,void *para2,void *para3)
{
    if(!task) {
        return -1;
    }

    return (*((cross_thread_task3)task))(para1,para2,para3);
}

static int run4(general_cross_thread_task task,void * para1,void *para2,void *para3,void *para4)
{
    if(!task) {
        return -1;
    }

    return (*((cross_thread_task4)task))(para1,para2,para3,para4);
}

static int run5(general_cross_thread_task task,void * para1,void *para2,void *para3,void *para4,void *para5)
{
    if(!task) {
        return -1;
    }

    return (*((cross_thread_task5)task))(para1,para2,para3,para4,para5);
}

static void thread_loop_func(cross_thread_type5 * obj)
{
    if(!obj) {
        return;
    }

    int ret;

    if(obj->m_id.invoke == general_run) {
        ret = general_run(obj->task);
    } else if(obj->m_id.invoke == run1) {
        ret = run1(obj->task,obj->para1);
    } else if(obj->m_id.invoke == run2) {
        ret = run2(obj->task,obj->para1,obj->para2);
    } else if(obj->m_id.invoke == run3) {
        ret = run3(obj->task,obj->para1,obj->para2,obj->para3);
    } else if(obj->m_id.invoke == run4) {
        ret = run4(obj->task,obj->para1,obj->para2,obj->para3,obj->para4);
    } else if(obj->m_id.invoke == run5) {
        ret = run5(obj->task,obj->para1,obj->para2,obj->para3,obj->para4,obj->para5);
    } else {
        ret = -1;
    }
    
    if(obj->call_back) {
        if(obj->m_id.invoke == general_run) {
            (*(obj->call_back))(ret,0);
        } else if(obj->m_id.invoke == run1) {
            (*(obj->call_back))(ret,1,obj->para1);
        } else if(obj->m_id.invoke == run2) {
            (*(obj->call_back))(ret,2,obj->para1,obj->para2);
        } else if(obj->m_id.invoke == run3) {
            (*(obj->call_back))(ret,3,obj->para1,obj->para2,obj->para3);
        } else if(obj->m_id.invoke == run4) {
            (*(obj->call_back))(ret,4,obj->para1,obj->para2,obj->para3,obj->para4);
        } else if(obj->m_id.invoke == run5) {
            (*(obj->call_back))(ret,5,obj->para1,obj->para2,obj->para3,obj->para4,obj->para5);
        } else {
            (*(obj->call_back))(ret,0);
        }
    }

    deallocate(obj);
}


static int extract_param(cross_thread_type5 *obj,general_cross_thread_task func,task_result call_back,int para_count,va_list ap)
{
    int ret = 0;

    if(para_count == 0) {

        obj->task = func;
        obj->call_back = call_back;
        obj->m_id.invoke = (general_invoke)&general_run;

    } else if(para_count == 1) {

        obj->m_id.invoke = (general_invoke)&run1;
        obj->task = func;
        obj->call_back = call_back;
        obj->para1 = va_arg(ap,void *);

    } else if(para_count == 2) {

        obj->m_id.invoke = (general_invoke)&run2;
        obj->task = func;
        obj->call_back = call_back;
        obj->para1 = va_arg(ap,void *);
        obj->para2 = va_arg(ap,void *);

    } else if(para_count == 3) {

        obj->m_id.invoke = (general_invoke)&run3;
        obj->task = func;
        obj->call_back = call_back;
        obj->para1 = va_arg(ap,void *);
        obj->para2 = va_arg(ap,void *);
        obj->para3 = va_arg(ap,void *);

    } else if(para_count == 4) {

        obj->m_id.invoke = (general_invoke)&run4;
        obj->task = func;
        obj->call_back = call_back;
        obj->para1 = va_arg(ap,void *);
        obj->para2 = va_arg(ap,void *);
        obj->para3 = va_arg(ap,void *);
        obj->para4 = va_arg(ap,void *);

    } else if(para_count == 5){
        
        obj->m_id.invoke = (general_invoke)&run5;
        obj->task = func;
        obj->para1 = va_arg(ap,void *);
        obj->para2 = va_arg(ap,void *);
        obj->para3 = va_arg(ap,void *);
        obj->para4 = va_arg(ap,void *);
        obj->para5 = va_arg(ap,void *);
    
    } else {
        ret = -1;
    }

    return ret;
}

int call_task_on_thread_queue(struct message_queue * queue,general_cross_thread_task func,task_result call_back,int para_count, ...)
{
    int ret;
    
    if(!queue) {
        return -1;
    }

    if(!func) {
        return -1;
    }

    if((para_count < 0) || (para_count > 5)) {
        return -1; 
    }

    cross_thread_type5 *obj = (cross_thread_type5 *)allocate(sizeof(cross_thread_type5));
    if(!obj) {
        ret = -1;
        goto error_ret;
    }

    memset(obj,0,sizeof(cross_thread_type5));
    
    /* extract every para */
    va_list ap;
    va_start(ap,para_count);
    ret = extract_param(obj,func,call_back,para_count,ap);
    va_end(ap);

    if(0 != ret) {
        deallocate(obj);
        goto error_ret;
    }

    ret = message_queue_append(queue,(void *)obj);
error_ret:
    return ret;
}


void thread_loop(void * arg)
{
    int ret;
    cross_thread_type5 *msg = NULL; 

    /* params check, my os has thread safe return function, so just return, don't worry */
    if(!arg){
        return;
    }

    struct message_queue * queue = (struct message_queue *)arg;

    message_queue_init(queue);

    while(1) {
        
        ret = message_queue_get(queue, (void **)&msg,0);
        if(ret) {
            continue;
        }

        thread_loop_func(msg);
    }

    message_queue_destroy(queue);
}



