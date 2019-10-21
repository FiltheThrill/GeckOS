/* idt.c - Functions to display exceptions and initialize interrupt descriptor table
Author - Filip Cakulev
https://wiki.osdev.org/Interrupt_Descriptor_Table
https://wiki.osdev.org/Exceptions
 */
#include "IDT.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
//ecxeption handling for giant switch case
void div_by_0(){exceptions(0);}
void debug(){exceptions(1);}
void NMInt(){exceptions(2);}
void breakpoint(){exceptions(3);}
void overflow(){exceptions(4);}
void BoundRE(){exceptions(5);}
void invalid_opcode(){exceptions(6);}
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
  clear();
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
      printf("I don't think you should be here. This is RESERVED 31 \n");
      break;
  }

  exception_flag = 1;
  while(1){}
}

/*
 * idt_init
 *   DESCRIPTION: inializes the interrupt descriptor table.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets dpl, present, and type values accordingly for systemcalls,
                  exceptions, keyboard, mouse, rtc, and pit (?)
 */
 void idt_init()
 {
    int i;
    for(i = 0; i < EXCEPTIONS_AMOUNT; i++)
    {
      idt[i].present = 1; //enable

      idt[i].dpl  = 0;        //set privilege level to handle exceptions
      //sets type to 01110 (32-bit interrupt gate) see page 22 of mp3 documentation
      idt[i].reserved0 = 0;
      idt[i].size = 1;
      idt[i].reserved1 = 1;
      idt[i].reserved2 = 1;
      idt[i].reserved3 = 0;
      idt[i].reserved4 = 0;   //unused?

      idt[i].seg_selector = KERNEL_CS;   //selector set to kernel
    }

    //set up exception handling entries
    SET_IDT_ENTRY(idt[0], div_by_0);
    SET_IDT_ENTRY(idt[1], debug);
    SET_IDT_ENTRY(idt[2], NMInt);
    SET_IDT_ENTRY(idt[3], breakpoint);
    SET_IDT_ENTRY(idt[4], overflow);
    SET_IDT_ENTRY(idt[5], BoundRE);
    SET_IDT_ENTRY(idt[6], invalid_opcode);
    SET_IDT_ENTRY(idt[7], device_not_available);
    SET_IDT_ENTRY(idt[8], double_fault);
    SET_IDT_ENTRY(idt[9], CP_seg_overrun);
    SET_IDT_ENTRY(idt[10], invalid_tss);
    SET_IDT_ENTRY(idt[11], seg_not_present);
    SET_IDT_ENTRY(idt[12], stack_seg_fault);
    SET_IDT_ENTRY(idt[13], general_protection);
    SET_IDT_ENTRY(idt[14], page_fault);
    SET_IDT_ENTRY(idt[15], reserved0);      //not sure if these need to handled
    SET_IDT_ENTRY(idt[16], floating_point);
    SET_IDT_ENTRY(idt[17], alignment_check);
    SET_IDT_ENTRY(idt[18], machine_check);
    SET_IDT_ENTRY(idt[19], simd_floating_point);
    SET_IDT_ENTRY(idt[20], virtualization);
    SET_IDT_ENTRY(idt[21], reserved1);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[22], reserved2);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[23], reserved3);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[24], reserved4);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[25], reserved5);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[26], reserved6);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[27], reserved7);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[28], reserved8);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[29], reserved9);       //not sure if these need to handled
    SET_IDT_ENTRY(idt[30], security);
    SET_IDT_ENTRY(idt[31], reserved10);       //not sure if these need to handled

    for(i = EXCEPTIONS_AMOUNT; i < MASTER_AND_SLAVE; i++){

        idt[i].present = 1; //enable

        idt[i].dpl  = 0;        //set privilege level to handle exceptions
        //sets type to 01110 (32-bit interrupt gate) see page 22 of mp3 documentation
        idt[i].reserved0 = 0;
        idt[i].size = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 0;
        idt[i].reserved4 = 0;   //unused?

        idt[i].seg_selector = KERNEL_CS;   //selector set to kernel
    }
    for(i = MASTER_AND_SLAVE; i < NUM_VEC; i++)
    {
        if(i == SYS_CALL_VECT)
        {
          idt[i].present = 1; //enable
          idt[i].dpl  = 3;      // set dpl to 3 for system calls
          //sets type to 01111 (32-bit trap gate)
          idt[i].reserved0 = 0;
          idt[i].size = 1;
          idt[i].reserved1 = 1;
          idt[i].reserved2 = 1;
          idt[i].reserved3 = 1;
          idt[i].reserved4 = 0;   //unused?

          idt[i].seg_selector = KERNEL_CS;   //selector set to kernel
        }
        else
        {
            idt[i].present = 0; //disable
            idt[i].dpl  = 0;
            //sets type to 01110 (32-bit interrupt gate) see page 22 of mp3 documentation
            idt[i].reserved0 = 0;
            idt[i].size = 1;
            idt[i].reserved1 = 1;
            idt[i].reserved2 = 1;
            idt[i].reserved3 = 0;
            idt[i].reserved4 = 0;   //unused?

            idt[i].seg_selector = KERNEL_CS;   //selector set to kernel
            SET_IDT_ENTRY(idt[i], 0);

        }

    }
    //SET_IDT_ENTRY(idt[SYS_CALL_VECT], somehandler); //not sure if needed right now
    //SET_IDT_ENTRY(idt[timerchip], pithandler); //not sure if needed right now
    //SET_IDT_ENTRY(idt[0x2C], mouse_handler);
    SET_IDT_ENTRY(idt[KEYBOARD_IRQ], keyboard_handler);
    SET_IDT_ENTRY(idt[IRQ8], RTC_handler);

    lidt(idt_desc_ptr);
    return;
 }
