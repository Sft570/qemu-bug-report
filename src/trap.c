#include "os.h"
#include "riscv.h"
extern void trap_vector();
extern void virtio_disk_isr();
extern void do_syscall(struct context *ctx);

void trap_init()
{
  // set the machine-mode trap handler.
  w_mtvec((reg_t)trap_vector);
}

void external_handler()
{
  int irq = plic_claim();
  if (irq == UART0_IRQ)
  {
    lib_isr();
  }
  else if (irq == VIRTIO_IRQ)
  {
    lib_puts("Virtio IRQ\n");
    virtio_disk_isr();
  }
  else if (irq)
  {
    lib_printf("unexpected interrupt irq = %d\n", irq);
  }

  if (irq)
  {
    plic_complete(irq);
  }
}

reg_t trap_handler(reg_t epc, reg_t cause, struct context *ctx)
{
  reg_t return_pc = epc;
  reg_t cause_code = cause & 0xfff;

switch (cause_code)
{
case 3:
  /* software interruption */
  break;
case 7:
  /* timer interruption */
  // disable machine-mode timer interrupts.
  w_mie(r_mie() & ~(1 << 7));
  timer_handler();
  return_pc = (reg_t)&os_kernel;
  // enable machine-mode timer interrupts.
  w_mie(r_mie() | MIE_MTIE);
  break;
case 11:
  /* external interruption */
  external_handler();
  break;
default:
  lib_puts("unknown async exception!\n");
  break;
}
  
  return return_pc;
}
