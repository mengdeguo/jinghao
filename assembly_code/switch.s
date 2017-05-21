.extern  cur_tcb_ptr 
.extern  highest_priority_tcb_ptr 

@define function
.global switch_to_thread_mode 
.global PendSV_IRQHandler

.equ NVIC_INT_CTRL,   0xE000ED04                              @ Interrupt control state register.
.equ NVIC_SYSPRI14,   0xE000ED22                              @ System priority register (priority 14).
.equ NVIC_PENDSV_PRI, 0xFF                                    @ PendSV priority value (lowest).
.equ NVIC_PENDSVSET,  0x10000000                              @ Value to trigger PendSV exception.

.text
.align 2
.thumb
.syntax unified


@ used to switch_to_thread_mode from the start code of the system

.thumb_func
switch_to_thread_mode:
    LDR     R0, =NVIC_SYSPRI14                                   
    LDR     R1, =NVIC_PENDSV_PRI
    STRB    R1, [R0]

    MOVS    R0, #0                                              
    MSR     PSP, R0

    LDR     R0, =NVIC_INT_CTRL                                  
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    
    CPSIE   I

DEADLOOP:                                                       
    B       DEADLOOP


@ pendsv handler: used to do context switch

.thumb_func
PendSV_IRQHandler:
    CPSID   I
    MRS     R0, PSP
    CBZ     R0, OS_CPU_PendSVHandler_nosave

    SUBS    R0, R0, #0x20
    STM     R0, {R4-R11}

    LDR     R1, =cur_tcb_ptr
    LDR     R1, [R1]
    STR     R0, [R1]

                                                                
OS_CPU_PendSVHandler_nosave:

    LDR     R0, =cur_tcb_ptr
    LDR     R1, =highest_priority_tcb_ptr
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     R0, [R2]
    LDM     R0, {R4-R11}
    ADDS    R0, R0, #0x20
    MSR     PSP, R0
    ORR     LR, LR, #0x04
    CPSIE   I
    BX      LR

.end
