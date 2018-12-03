/* Engineer(s): Ezzeddeen Gazali and Tyler Starr
 * Create Date: 10/14/2018
 * Description: This library file implements useful functions
 *              for measuring the frequency of a square wave
 *              input signal.
 * Pin Assignments: P2.5 -> input signal
 */
#include "msp.h"

//Define Values for Inputs
#define INPUT_FREQ BIT5
#define INPUT_WAVE BIT4


//Bits of the flag variable defined
#define wave_freq_flag BIT1
#define dc_flag BIT5

//function prototypes
uint16_t get_flags(void);
void reset_flags(uint16_t flags);
void set_flags(uint16_t flags);
void init_clock(void);
void set_freq_clk(void);
uint16_t get_captured_freq(void);
void init_freq_timer(void);
void set_freq_conversion(uint32_t input_conv_factor);


