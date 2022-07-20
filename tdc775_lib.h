// V792N -> V775N , adc792 -> tdc775 , a792 -> t775 , V792N_IPED -> V775N_FSR
#include <vector>
#ifndef _V775N_HEADER_
#define _V775N_HEADER_

#define V775N_ADDRESS               0xbb220000
#define V775N_CHANNEL               16

#define V775N_REG1_STATUS           0x100e
#define V775N_REG2_STATUS           0x1022
#define V775N_REG1_CONTROL          0x1010
#define V775N_BIT_SET1              0x1006
#define V775N_BIT_CLEAR1            0x1008
#define V775N_BIT_SET2              0x1032
#define V775N_BIT_CLEAR2            0x1034
#define V775N_OUTPUT_BUFFER         0x0000
#define V775N_MCST_CBLT_ADDRESS     0x1004
#define V775N_INTERRUPT_LEVEL       0x100a
#define V775N_INTERRUPT_VECTOR      0x100c
#define V775N_ADER_HIGH             0x1012
#define V775N_ADER_LOW              0x1014
#define V775N_MCST_CBLT_CTRL        0x101a
#define V775N_EVENT_TRIGGER_REG     0x1020
#define V775N_EVENT_COUNTER_L       0x1024
#define V775N_EVENT_COUNTER_H       0x1026
#define V775N_FCLR_WINDOW           0x102e
#define V775N_W_MEM_TEST_ADDRESS    0x1036
#define V775N_MEM_TEST_WORD_HIGH    0x1038
#define V775N_CRATE_SELECT          0x103c
#define V775N_FSR                   0x1060
#define V775N_R_TEST_ADDRESS        0x1064
#define V775N_SS_RESET_REG          0x1016

//#define tdc775_OVFSUP    0 // overflow suppression ON
#define tdc775_OVFSUP    0x8 // overflow suppression OFF

//#define tdc775_ZROSUP    0 // zero suppression ON
#define tdc775_ZROSUP    0x10 // zero suppression OFF

#define tdc775_STASTOP    0 // Common START
//#define tdc775_STASTOP    0x400 // Common STOP

#define tdc775_LSB    240 // LSB=ns per count, roughly LSB(ns)=8.9/tdc775_LSB 
#define tdc775_THR    0 // minimum value to be stored -SET to 0 

#define tdc775_debug    0

unsigned short init_tdc775(int32_t BHandle);
unsigned short read_tdc775_simple(int32_t BHandle, int *pDataAdc);
std::vector<int> read_tdc775(int32_t BHandle, short int* pstatus);
std::vector<int> readFasttdc775(int32_t BHandle, short int* pstatus);

struct t775_shift{
  int datareg;
  int statusreg1;
  int statusreg2;
} ;
static const struct t775_shift tdc775_shift=
{0x0000,0x100e,0x1022};

struct t775_bitmask{
  unsigned long rdy;
  unsigned long busy;
  unsigned long full;
  unsigned long empty;
};
static const struct t775_bitmask tdc775_bitmask=
{0x1,0x4,0x4,0x2};
#endif
