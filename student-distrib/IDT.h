//descriptors for idtc.c - INCLUDES EXCEPTION_FLAG WHICH WILL PROBABLY BE USED IN OTHER
//FILES. sSimply include IDT.h to get access to exception_flag
#ifndef _IDT_H
#define _IDT_H

#define EXCEPTIONS_AMOUNT   32        //first 32 in idt
#define MASTER_AND_SLAVE    48        //each PIC takes up 1 byte in idt
#define SYS_CALL_VECT       0x80      // interrupt vector for system calls
#define IRQ8                0x28      //real time clock
#define KEYBOARD_IRQ        0x21      //keyboard interrupt vector

int exception_flag;

extern void div_by_0();
extern void debug();
extern void NMInt();
extern void breakpoint();
extern void overflow();
extern void BoundRE();
extern void invalid_opcode();
extern void device_not_available();
extern void double_fault();
extern void CP_seg_overrun();
extern void invalid_tss();
extern void seg_not_present();
extern void stack_seg_fault();
extern void general_protection();
extern void page_fault();
extern void reserved0();
extern void floating_point();
extern void alignment_check();
extern void machine_check();
extern void simd_floating_point();
extern void virtualization();
extern void reserved1();
extern void reserved2();
extern void reserved3();
extern void reserved4();
extern void reserved5();
extern void reserved6();
extern void reserved7();
extern void reserved8();
extern void reserved9();
extern void security();
extern void reserved10();

extern void exceptions(int exception_num);

extern void idt_init();
#endif
