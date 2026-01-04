// Index 0 for normal memory
// Index 1 for device memory
// MAIR_EL1
#define MAIR_ATTR_NORMAL_WBWA   0xff   // Normal, inner/outer WB WA
#define MAIR_ATTR_DEVICE_nGnRnE 0x00   // Device nGnRnE

// TCR_EL1
#define TBI1 0b0
#define TBI0 0b0
#define AS 0b0
#define IPS 0b010
#define TG1 0b10 // Page size of TTBR1
#define SH1 0b11
#define ORGN1 0b01
#define IRGN1 0b01
#define EPD1 0b1
#define A1 0b0
#define T1SZ 24
#define TG0 0b00 // Page size of TTBR0
#define SH0 0b11
#define ORGN0 0b01
#define IRGN0 0b01
#define EPD0 0b0
#define T0SZ 24

//TTBRx_EL1

.equ MAIR_CFG_macro, ((MAIR_ATTR_DEVICE_nGnRnE << (8 * 1)) | (MAIR_ATTR_NORMAL_WBWA << (8 * 0)))
.equ TCR_EL1_macro, (0 | (TBI1 << 38) | (TBI0 << 37) | (AS << 36)    | (IPS << 32) \
                | (TG1 << 30)  | (SH1 << 28)  | (ORGN1 << 26) | (IRGN1 << 24)\
                | (EPD1 << 23) | (A1 << 22)   | (T1SZ << 16)  | (TG0 << 14) \
                | (SH0 << 12)  | (ORGN0 << 10)| (IRGN0 << 8)  | (EPD0 << 7) \
                | (T0SZ))


    .text
    .global init_simple_mmu

init_simple_mmu:
    ldr x1, =MAIR_CFG_macro
    ldr x2, =TCR_EL1_macro
    msr mair_el1, x1
    msr tcr_el1, x2
    isb
    ret

