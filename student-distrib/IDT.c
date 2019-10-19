/* idt.c - Functions to display exceptions and initialize interrupt descriptor table
author - Filip Cakulev
https://wiki.osdev.org/Interrupt_Descriptor_Table
https://wiki.osdev.org/Exceptions
 */
#include "idt.h"
#include "x86_desc.h"

//ecxeption handling for giant switch case
void div_by_0(){exceptions(0);}
void debug(){exceptions(1);}
void NMI(){exceptions(2);}
void breakpoint(){exceptions(3);}
void overflow(){exceptions(4);}
void BRE(){exceptions(5);}
void inavlid_opcode(){exceptions(6);}
void device_not_available(){exceptions(7);}
void double_fault(){exceptions(8);}
void CP_seg_overrun(){exceptions(9);}
void invalid_tss(){exceptions(10);}
void seg_not_present(){exceptions(11);}
void stack_seg_fault(){exceptions(12);}
void general_protection(){exceptions(13);}
void page_fault(){exceptions(14);}
void reserved0(){exceptions(15);}
void floating_point(){exceptions(16);}
void alignment_check(){exceptions(17);}
void machine_check(){exceptions(18);}
void simd_floating_point(){exceptions(19);}
void virtualization(){exceptions(20);}
void reserved1(){exceptions(21);}
void reserved2(){exceptions(22);}
void reserved3(){exceptions(23);}
void reserved4(){exceptions(24);}
void reserved5(){exceptions(25);}
void reserved6(){exceptions(26);}
void reserved7(){exceptions(27);}
void reserved8(){exceptions(28);}
void reserved9(){exceptions(29);}
void security(){exceptions(30);}
void reserved10(){exceptions(31);}

/*
 * exceptions
 *   DESCRIPTION: handles all 32 exceptions for IDT
 *   INPUTS: exception_num - number corresponding to which exception was raised
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets exception flag to 1 and prints exception to screen
 */

void exceptions(int exception_num)
{
  switch(exception_num)
  {
    case 0:
      printf("Divide By Zero Error Exception \n");
      break;

    case 1:
      printf("Debug exception \n");
      break;

    case 2:
      printf("Non-maskable Interrupt Exception \n");
      break;

    case 3:
      printf("Breakpoint Exception \n");
      break;

    case 4:
      printf("Overflow Exception \n");
      break;

    case 5:
      printf("Bound Range Exceeded Exception \n");
      break;

    case 6:
      printf("Invalid Opcode Exception \n");
      break;

    case 7:
      printf("Device Not Available Exception \n");
      break;

    case 8:
      printf("Double Fault Exception \n");
      break;

    case 9:
      printf("Coprocessor Segment Overrun Exception \n");
      break;

    case 10:
      printf("Invalid TSS Exception \n");
      break;

    case 11:
      printf("Segment Not Present Exception \n");
      break;

    case 12:
      printf("Stack-Segment Fault Exception \n");
      break;

    case 13:
      printf("General Protection Fault Exception \n");
      break;

    case 14:
      printf("Page Fault Exception \n");
      break;

    case 15:
      printf("I don't think you should be here. This is RESERVED 15 \n");
      break;

    case 16:
      printf("x87 Floating-Point Exception \n");
      break;

    case 17:
      printf("Alignment Check Exception \n");
      break;

    case 18:
      printf("Machine Check Exeption \n");
      break;

    case 19:
      printf("SIMD Floating_Point Exception \n");
      break;

    case 20:
      printf("Virtualiation Exception \n");
      break;

    case 21:
      printf("I don't think you should be here. This is RESERVED 21 \n");
      break;

    case 22:
      printf("I don't think you should be here. This is RESERVED 22 \n");
      break;

    case 23:
      printf("I don't think you should be here. This is RESERVED 23 \n");
      break;

    case 24:
      printf("I don't think you should be here. This is RESERVED 24 \n");
      break;

    case 25:
      printf("I don't think you should be here. This is RESERVED 25 \n");
      break;

    case 26:
      printf("I don't think you should be here. This is RESERVED 26 \n");
      break;

    case 27:
      printf("I don't think you should be here. This is RESERVED 27 \n");
      break;

    case 28:
      printf("I don't think you should be here. This is RESERVED 28 \n");
      break;

    case 29:
      printf("I don't think you should be here. This is RESERVED 29 \n");
      break;

    case 30:
      printf("Security Exception \n");
      break;

    case 31:
      printf("I don't think you should be here. This is RESERVED 31 \n")
      break;
  }

  exception_flag = 1;
}


 void idt_init()
 {
    int i;
    for(i = 0; i < )
 }
