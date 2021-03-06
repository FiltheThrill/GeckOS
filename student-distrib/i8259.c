/* i8259.c - Functions to interact with the 8259 interrupt controller
  Author - Filip Cakulev
  https://wiki.osdev.org/PIC#Code_Examples
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/*
 * i8259_init
 *   DESCRIPTION: initializes both master and slave PIC. Informs both on their
                  vector interrupt offset. Tells master where the slave is and
                  the slave its cascading irqnum.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: sets irq mask on both to mask all irqs
 */
/* Initialize the 8259 PIC */
void i8259_init(void) {
  unsigned long flags;
  printf("enabled\n");

  cli_and_save(flags);    //not sure if needed

  master_mask = BYTE1;     //mask out interrupts
  slave_mask = BYTE1;

  outb(ICW1, MASTER_8259_PORT);             // initialize master PIC
  outb(ICW2_MASTER, MASTER_8259_PORT + 1);  // Master PIC vector offset
  outb(ICW3_MASTER, MASTER_8259_PORT + 1);  // Tell master there is slave at IRQ2
  outb(ICW4, MASTER_8259_PORT + 1);          // 8086 mode?

  outb(ICW1, SLAVE_8259_PORT);            // initialize slave PIC
  outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);  // Slave PIC vector offset
  outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);  // tell slave PIC its cascade identity (0010)
  outb(ICW4, SLAVE_8259_PORT + 1);        // 8086 mode?


  // restore interrupt mask
  outb(master_mask, MASTER_8259_PORT + 1);
  outb(slave_mask, SLAVE_8259_PORT + 1);

  restore_flags(flags);
  sti();
}

/*
 * enable_irq
 *   DESCRIPTION: enables a given irq by changing the irq mask accordingly
 *   INPUTS: irq_num - irq port to be enabled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes irq mask
 */
/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
  uint8_t int_mask;

  int_mask = (irq_num & 0x07);      // get bottom 3 bits of either PIC interrupt
  int_mask = ~(1 << int_mask);        //shift 1 over to correct unmasking location and then flip it

  if(irq_num >= 8)
  {
    slave_mask = slave_mask & int_mask;         //update slave mask
    outb(slave_mask, SLAVE_8259_PORT + 1);
    master_mask = master_mask & ENABLE_IRQ2;    //enable irq2 on master
    outb(master_mask, MASTER_8259_PORT + 1);
  }
  else
  {
    master_mask = master_mask & int_mask;       //update master mask
    outb(master_mask, MASTER_8259_PORT + 1);
  }
}
/*
 * disable_irq
 *   DESCRIPTION: disables a given irq by changing the irq mask accordingly
 *   INPUTS: irq_num - irq port to be disabled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes irq mask
 */
/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
  uint8_t int_mask;
  unsigned long flags;


  int_mask = (irq_num & 0x07);      // get bottom 3 bits of either PIC interrupt
  int_mask = 1 << int_mask;        //shift 1 over to correct masking location

  cli_and_save(flags);      //not sure if needed
  if(irq_num >= 8)
  {
    slave_mask = slave_mask | int_mask;         //update slave mask
    outb(slave_mask, SLAVE_8259_PORT + 1);

  }
  else
  {
    master_mask = master_mask | int_mask;       //update master mask
    outb(master_mask, MASTER_8259_PORT + 1);
  }
  restore_flags(flags);

}
/*
 * send_eoi
 *   DESCRIPTION: sends an eoi signal to a given irq port to tell the PIC
                  that the interrupt has been handled
 *   INPUTS: irq_num - irq port to send eoi to
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
  cli();      //not sure if needed
  asm volatile(
    "pushal\n"
    :
    :);

  if(irq_num < 8)
  {
    outb(EOI | irq_num, MASTER_8259_PORT);
  }
  else
  {
    outb(EOI | (irq_num & 0x07), SLAVE_8259_PORT);   //make sure to send eoi to master irq2 as well
    outb(EOI | 0x02, MASTER_8259_PORT);
  }
  asm volatile(
    "popal\n"
    :
    :);

  sti();
}
