// Boot loader.
//
// Part of the boot block, along with bootasm.S, which calls bootmain().
// bootasm.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.

#include "types.h"
#include "elf.h"
#include "x86.h"
#include "memlayout.h"

#define SECTSIZE  512

void readseg(uchar*, uint, uint);

void
bootmain(void)
{
  struct elfhdr *elf;
  struct proghdr *ph, *eph;
  void (*entry)(void);
  uchar* pa;

  elf = (struct elfhdr*)0x10000;  // scratch space

  // Read 1st page off disk
  readseg((uchar*)elf, 4096, 0);

  // Is this an ELF executable?
  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error

  // Load each program segment (ignores ph flags).
  ph = (struct proghdr*)((uchar*)elf + elf->phoff);
  eph = ph + elf->phnum;
  for(; ph < eph; ph++){
    pa = (uchar*)ph->paddr;
    // Read a program segment pointed by 'ph' from the kernel ELF file
    // on the disk (starting from Sector 1 where the ELF file begins)
    // into its physical address 'pa'. The reading starts at the
    // offset 'ph->off' and spans 'ph->filesz' bytes.
    readseg(pa, ph->filesz, ph->off);
    // If the size of memory image is bigger than the file size (e.g.
    // for .bss segments), the extra space is zeroed.
    if(ph->memsz > ph->filesz)
      stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
  }

  // Call the entry point from the ELF header.
  // Does not return!
  entry = (void(*)(void))(elf->entry);
  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready.
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
void
readsect(void *dst, uint offset)
{
  // Issue command.
  // 0x1F2 - 0x1F7 are control registers
  // 0x1F2: how many sectors to read
  // 0x1F3: the sector number register
  // 0x1F4: cylinder low register
  // 0x1F5: cylinder high register
  // 0x1F6: drive/head register
  // 0x1F7: status port AND command register 
  waitdisk();
  outb(0x1F2, 1);   // count = 1
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  // Read data.
  // Wait for the disk to be ready for reading a sector
  waitdisk();
  // Read a sector from the data port 0x1F0 into the
  // destination memory address 'dst'. Note that because
  // we are reading long words (ints),
  // the count is the size divided by 4.
  insl(0x1F0, dst, SECTSIZE/4);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
//
//                  sector boundary in memory
//                  |   pa
//                  |   |
//                  V   V
// +----------------+---+---------------+
// |                |OFS|               |  Physical memory
// +----------------+---+---------------+
//
//  Sector 0          Sector i
// +--------+--------+---+----+--------+
// |        | ...... |OFS|    | ...... |   Hard disk
// +--------+--------+---+----+--------+
//          <---offset--->
//
// According to this graph, 'readseg' begins from the i-th sector.
// Sector 0 contains the boot loader. The kernel binary is in the ELF
// format and stored starting from Sector 1. 'offset' is the offset
// into this ELF file. Therefore, it starts from Sector 1.
// OFS = offset % SECTSIZE. It is used to adjust 'pa'
// so that it starts from the boundary for sector i in memory.
// 
void
readseg(uchar* pa, uint count, uint offset)
{
  uchar* epa;

  epa = pa + count;

  // Round down to sector boundary.
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  offset = (offset / SECTSIZE) + 1;

  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
