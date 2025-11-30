// Index 0 for normal memory
// Index 1 for device memory
// TCR_EL1
#define TBI1 0b0
#define TBI0 0b0
#define AS 0b0
#define IPS 0b010
#define TG1 0b10
#define SH1 0b11
#define ORGN1 0b01
#define IRGN1 0b01
#define EPD1 0b0
#define A1 0b0
#define T1SZ 16
#define TG0 0b00
#define SH0 0b11
#define ORGN0 0b01
#define IRGN0 0b01
#define EPD0 0b0
#define T0SZ 16

//TCR_EL1
#define TBI1 0b0
#define TBI0 0b0
#define AS 0b0
#define IPS 0b010
#define TG1 0b10
#define SH1
#define ORGN1
#define IRGN1
#define EPD1
#define A1
#define T1SZ
#define TG0
#define SH0
#define ORGN0
#define IRGN0
#define EPD0
#define T0SZ

.equ TCR_CFG, (0 | (TBI1 << 38) | (TBI0 << 37) | (AS << 36)    | (IPS << 32) \
                | (TG1 << 30)  | (SH1 << 28)  | (ORGN1 << 26) | (IRGN1 << 24)\
                | (EPD1 << 23) | (A1 << 22)   | (T1SZ << 16)  | (TG0 << 14) \
                | (SH0 << 12)  | (ORGN0 << 10)| (IRGN0 << 8)  | (EPD0 << 7) \
                | (T0SZ))

mov x1, =TCR_CFG
msr TCR_CFG, x1

