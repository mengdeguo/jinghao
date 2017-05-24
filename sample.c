#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "thread.h"
#include "sys_tick.h"
#include "mutex.h"
#include "time.h"
#include "message_queue.h"
#include "cross_thread_copier.h"
#include "timer.h"
#include "platform.h"


tcb * first = NULL;
tcb * second = NULL;
tcb * third = NULL;

struct message_queue loop_queue;

int test_cross_thread_task(int arg1,int arg2,int arg3)
{
    t_printf("idle percentage is %d \r\n",get_idle_percentage());
    return arg1 + arg2 + arg3;
}

void test_cross_thread_task_result(int ret,int para_count,int arg1, int arg2, int arg3) 
{
    t_printf("ret is %d, para_count is %d, arg1 is %d ,arg2 is %d ,arg3 is %d \r\n",
            ret,para_count,arg1,arg2,arg3);
}

struct timer tm1,tm2,tm3,tm4;

void test_thread1(void * arg)
{
    t_printf("great, now in thread1 & arg is %d \r\n",(uint32_t)arg);

    start_timer(&tm1);
    start_timer(&tm2);
    start_timer(&tm3);
    start_timer(&tm4);

    while(1) {
        //t_printf("in thread1 will delay \r\n");
        
        int i = 100000;
        int j = 100000;
        int k = 100000;
        int m = 100000;

        for(;i >0;--i)
            for(;j>0;--j)
                for(;k>0;--k)
                    for(;m>0;--m);


        delay_ms(300);
    }
}

void test_thread2(void * arg)
{

    t_printf("now in thread2 & arg is %d \r\n",(uint32_t)arg);
    
    while(1) {
        t_printf("in thread2 will delay \r\n");
        delay_ms(2000);

        call_task_on_thread_queue(&loop_queue,(general_cross_thread_task)test_cross_thread_task,(task_result)test_cross_thread_task_result,3,1,2,3);
    }

}

void test_timer_handle(struct timer * tm)
{
    t_printf("%s time out \r\n",(const char *)tm->param);
}


void main()
{
    platform_init();

    sys_tick_init();

    init_timer(&tm1,test_timer_handle,"tm1",0,1000,ONE_SHOT);
    init_timer(&tm2,test_timer_handle,"tm2",2000,3000,REPEAT);
    init_timer(&tm3,test_timer_handle,"tm3",0,2000,ONE_SHOT);
    init_timer(&tm4,test_timer_handle,"tm4",3000,1000,REPEAT);

    third = create_thread(512,&thread_loop,20,(void *)&loop_queue,"test3");

    second = create_thread(256,&test_thread2,10,(void*) 100,"test2");

    //test create thread
    first = create_thread(256,&test_thread1,11,(void *)10,"test1");
    if(first) {
        t_printf("create thread success \r\n");

        start_os();
    }

    while(1) {
    
    }
}
