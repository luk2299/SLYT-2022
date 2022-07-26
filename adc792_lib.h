#include <vector>
#ifndef _V792N_HEADER_
#define _V792N_HEADER_

#define NUMADBOARDS                 1
#define V792N_ADDRESS               0xee000000
#define V792N_ADDRESS2              0xee000000
#define V792N_ADDRESS3              0xee000000
#define V792N_CHANNEL               32

#define V792N_OUTPUT_BUFFER         0x0000
#define V792N_FIRMWARE_REVISION     0x1000
#define V792N_GEO_ADDRESS           0x1002
#define V792N_MCST_CBLT_ADDRESS     0x1004
#define V792N_BIT_SET1              0x1006
#define V792N_BIT_CLEAR1            0x1008
#define V792N_INTERRUPT_LEVEL       0x100a
#define V792N_INTERRUPT_VECTOR      0x100c
#define V792N_REG1_STATUS           0x100e
#define V792N_REG1_CONTROL          0x1010
#define V792N_ADER_HIGH             0x1012
#define V792N_ADER_LOW              0x1014
#define V792N_SS_RESET_REG          0x1016
#define V792N_MCST_CBLT_CTRL        0x101a
#define V792N_EVENT_TRIGGER_REG     0x1020
#define V792N_REG2_STATUS           0x1022
#define V792N_EVENT_COUNTER_L       0x1024
#define V792N_EVENT_COUNTER_H       0x1026
#define V792N_INCREMENT_EVENT       0x1028
#define V792N_INCREMENT_OFFSET      0x102a
#define V792N_LOAD_TEST_REGISTER    0x102c
#define V792N_FCLR_WINDOW           0x102e
#define V792N_BIT_SET2              0x1032
#define V792N_BIT_CLEAR2            0x1034
#define V792N_W_MEM_TEST_ADDRESS    0x1036
#define V792N_MEM_TEST_WORD_HIGH    0x1038
#define V792N_MEM_TEST_WORD_LOW     0x103a
#define V792N_CRATE_SELECT          0x103c
#define V792N_TEST_EVENT_WRITE      0x103e
#define V792N_EVENT_COUTER_RES      0x1040
#define V792N_IPED                  0x1060
#define V792N_R_TEST_ADDRESS        0x1064
#define V792N_SW_COMM               0X1068
#define V792N_AAD                   0x1070
#define V792N_BAD                   0x1072
#define V792N_SLIDE_CONSTANT        0X106a
#define V792N_THRESHOLDS            0x1080

#define IPED_VAL                    95
#define N_EVENTS                    1
#define INTERRUPT_LEVEL             0x1
#define adc792_debug                verbose

extern int verbose;

unsigned short init_adc792(int32_t BHandle, unsigned long Pmax);
int data_reset_adc792(int32_t BHandle);
int trigger_interrupt(int32_t BHandle, bool *ptrig);
std::vector<unsigned long> read_block_adc792(int32_t BHandle, int idB, int& status);
std::vector<int> read_adc792(int32_t BHandle, int &status);
std::vector<unsigned long> readFastadc792(int32_t BHandle, int idB, int& status);
std::vector<int> readFastNadc792(int32_t BHandle, int idB, int& status, int nevts, std::vector<int> &outW);
int read_scaler_adc792(int32_t BHandle);

struct a792_bitmask{
  unsigned long rdy;
  unsigned long busy;
  unsigned long full;
  unsigned long empty;
};
static const struct a792_bitmask adc792_bitmask=
{0x1,0x4,0x4,0x2};
#endif
