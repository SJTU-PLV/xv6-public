//
// assembler macros to create x86 segments
//

#define SEG_NULLASM                                             \
        .word 0, 0;                                             \
        .byte 0, 0, 0, 0

// The 0xC0 means the limit is in 4096-byte units
// and (for executable segments) 32-bit mode.
// 
// ------------------------------------------------------------
// Comment by: Yuting Wang
// Date: Wed Apr  2 12:47:58 PM UTC 2025
// ------------------------------------------------------------
// 1. This marcro creates an GDT entry (64-bit), starting from
//    lower bits to higher ones.
// 2. lim is 32-bit limit with the page granularity. Therefore,
//    (lim >> 12) should be the limit stored in a GDT entry.
//    (limt >> 12) is split into lower 16-bits and higher 8-bits.
// 3. base is 32-bit and split into severl places
// 4. type is 4-bit and specifies the type of this entry
// 5. 0x90 indicates that
//       P = 1, entry is present
//       DPL = 0, requires ring 0 privilige to access
//       S = 1, this is a code/data segment
// 6. 0xC0 indicates that
//       G = 1, page granuality
//       D/B = 1, default operand size is 32-bit (a 32-bit segment)
#define SEG_ASM(type,base,lim)                                  \
        .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);      \
        .byte (((base) >> 16) & 0xff), (0x90 | (type)),         \
                (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

#define STA_X     0x8       // Executable segment
#define STA_W     0x2       // Writeable (non-executable segments)
#define STA_R     0x2       // Readable (executable segments)
