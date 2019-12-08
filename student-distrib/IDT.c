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
#include "syscall_wrapper.h"
#include "syscalls.h"
#include "types.h"
#include "pit.h"
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
  //clear();

  switch(exception_num)
  {
    case 0:
      {uint8_t error_buf0[] = "Divide By Zero Error Exception\n";
      term_write(1, error_buf0, ERROR_SIZE);
      break;}

    case 1:
      {uint8_t error_buf1[] = "Debug Exception\n";
      term_write(1, error_buf1, ERROR_SIZE);
      break;}

    case 2:
      {uint8_t error_buf2[] = "Non-maskable Interrupt Exception \n";
      term_write(1, error_buf2, ERROR_SIZE);
      break;}

    case 3:
      {uint8_t error_buf3[] = "Breakpoint Exception \n";
      term_write(1, error_buf3, ERROR_SIZE);
      break;}

    case 4:
    {  uint8_t error_buf4[] = "Overflow Exception \n";
      term_write(1, error_buf4, ERROR_SIZE);
      break;}

    case 5:
      {uint8_t error_buf5[] = "Bound Range Exceeded Exception \n";
      term_write(1, error_buf5, ERROR_SIZE);
      break;}

    case 6:
      {uint8_t error_buf6[] = "Invalid Opcode Exception \n";
      term_write(1, error_buf6, ERROR_SIZE);
      break;}

    case 7:
    {  uint8_t error_buf7[] = "Device Not Available Exception \n";
      term_write(1, error_buf7, ERROR_SIZE);
      break;}

    case 8:
    {  uint8_t error_buf8[] = "Double Fault Exception \n";
      term_write(1, error_buf8, ERROR_SIZE);
      break;}

    case 9:
      {uint8_t error_buf9[] = "Coprocessor Segment Overrun Exception \n";
      term_write(1, error_buf9, ERROR_SIZE);
      break;}

    case 10:
      {uint8_t error_buf10[] = "Invalid TSS Exception \n";
      term_write(1, error_buf10, ERROR_SIZE);
      break;}

    case 11:
      {uint8_t error_buf11[] = "Segment Not Present Exception \n";
      term_write(1, error_buf11, ERROR_SIZE);
      break;}

    case 12:
      {uint8_t error_buf12[] = "Stack-Segment Fault Exception \n";
      term_write(1, error_buf12, ERROR_SIZE);
      break;}

    case 13:
      {uint8_t error_buf13[] = "General Protection Fault Exception \n";
      term_write(1, error_buf13, ERROR_SIZE);
      break;}

    case 14:
      {uint8_t error_buf14[] = "Page Fault Exception \n";
      term_write(1, error_buf14, ERROR_SIZE);
      break;}

    case 15:
      {uint8_t error_buf15[] = "I don't think you should be here. This is RESERVED 15 \n";
      term_write(1, error_buf15, ERROR_SIZE);
      break;}

    case 16:
      {uint8_t error_buf16[] = "x87 Floating-Point Exception \n";
      term_write(1, error_buf16, ERROR_SIZE);
      break;}

    case 17:
      {uint8_t error_buf17[] = "Alignment Check Exception \n";
      term_write(1, error_buf17, ERROR_SIZE);
      break;}

    case 18:
      {uint8_t error_buf18[] = "Machine Check Exeption \n";
      term_write(1, error_buf18, ERROR_SIZE);
      break;}

    case 19:
      {uint8_t error_buf19[] = "SIMD Floating_Point Exception \n";
      term_write(1, error_buf19, ERROR_SIZE);
      break;}

    case 20:
      {uint8_t error_buf20[] = "Virtualiation Exception \n";
      term_write(1, error_buf20, ERROR_SIZE);
      break;}

    case 21:
      {uint8_t error_buf21[] = "I don't think you should be here. This is RESERVED 21 \n";
      term_write(1, error_buf21, ERROR_SIZE);
      break;}

    case 22:
      {uint8_t error_buf22[] = "I don't think you should be here. This is RESERVED 22 \n";
      term_write(1, error_buf22, ERROR_SIZE);
      break;}

    case 23:
    {  uint8_t error_buf23[] = "I don't think you should be here. This is RESERVED 23 \n";
      term_write(1, error_buf23, ERROR_SIZE);
      break;}

    case 24:
      {uint8_t error_buf24[] = "I don't think you should be here. This is RESERVED 24 \n";
      term_write(1, error_buf24, ERROR_SIZE);
      break;}

    case 25:
      {uint8_t error_buf25[] = "I don't think you should be here. This is RESERVED 25 \n";
      term_write(1, error_buf25, ERROR_SIZE);
      break;}

    case 26:
      {uint8_t error_buf26[] = "I don't think you should be here. This is RESERVED 26 \n";
      term_write(1, error_buf26, ERROR_SIZE);
      break;}

    case 27:
      {uint8_t error_buf27[] = "I don't think you should be here. This is RESERVED 27 \n";
      term_write(1, error_buf27, ERROR_SIZE);
      break;}

    case 28:
    {
      uint8_t error_buf28[] = "I don't think you should be here. This is RESERVED 28 \n";
      term_write(1, error_buf28, ERROR_SIZE);
      break;
    }

    case 29:
    {
      uint8_t error_buf29[] = "I don't think you should be here. This is RESERVED 29 \n";
      term_write(1, error_buf29, ERROR_SIZE);
      break;
    }

    case 30:
    {
      uint8_t error_buf30[] = "Security Exception \n";
      term_write(1, error_buf30, ERROR_SIZE);
      break;
    }

    case 31:
    {
      uint8_t error_buf31[] = "I don't think you should be here. This is RESERVED 31 \n";
      term_write(1, error_buf31, ERROR_SIZE);
      break;
    }
  }

  exception_flag = 1;
  halt(-1);
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
    SET_IDT_ENTRY(idt[SYS_CALL_VECT], syscall_handler); //not sure if needed right now
    SET_IDT_ENTRY(idt[TIMERCHIP], pit_handler); //not sure if needed right now
    //SET_IDT_ENTRY(idt[0x2C], mouse_handler);
    SET_IDT_ENTRY(idt[KEYBOARD_IRQ], keyboard_handler);
    SET_IDT_ENTRY(idt[IRQ8], RTC_handler);

    lidt(idt_desc_ptr);
    return;
 }
