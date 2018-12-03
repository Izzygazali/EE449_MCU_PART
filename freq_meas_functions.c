/*
 * Engineer(s): Ezzeddeen Gazali and Tyler Starr
 * Create Date: 10/14/2018
 */
#include "freq_meas_functions.h"

//Definitions for flags:
//BIT 1 -> Wave Freq Captured
//BIT 5 -> DC flag
volatile uint16_t flags = 0;


//variables for frequency determination
volatile uint16_t captured_freq = 0;
uint32_t freq_conv_factor = 0;


/*
 * Function that returns current flags status.
 * INPUT:   NONE
 * RETURN:  uint16_t flags = current flags status.
 */
uint16_t get_flags(void)
{
    //return flags
    return flags;
}

/*
 * Function that resets specified flags to 0.
 * INPUT:   uint16_t flags = flags to reset.
 * RETURN:  NONE
 */
void reset_flags(uint16_t r_flags)
{
    //reset flags
    flags &= ~r_flags;
}

/*
 * Function that sets specified flags to 1.
 * INPUT:   uint16_t flags = flags to set.
 * RETURN:  NONE
 */
void set_flags(uint16_t s_flags)
{
    //set flags
    flags |= s_flags;
}

/*
 * Function that initializes clocks used in TIMER A
 * INPUT:   NONE
 * RETURN:  NONE
 */
void init_clock(void)
{
    //unlock CS register
    CS ->KEY = CS_KEY_VAL;
    //clear CS register
    CS->CTL0 = 0;
    //set DCO to 24MHz
    CS ->CTL0 |= CS_CTL0_DCORSEL_4;
    //set REFO at 128kHz and SMCLK, MCLK to 24MHz
    CS ->CLKEN |= CS_CLKEN_ACLK_EN | CS_CLKEN_REFOFSEL;
    //divide ACLK by four for 32kHz clock
    CS->CTL1 |= CS_CTL1_SELA__REFOCLK | CS_CTL1_DIVA_2 |
                CS_CTL1_SELS__DCOCLK | CS_CTL1_SELM__DCOCLK;
    //lock CS register
    CS ->KEY = 0;
    return;
}

/*
 * Function sets the factor that converts counter "counts" into frequency
 * values.
 * INPUT:   NONE
 * RETURN:  NONE
 */
void set_freq_conversion(uint32_t input_conv_factor)
{
    freq_conv_factor = input_conv_factor;
    return;
}

/*
 * Function returns the frequency of the input wave in Hz.
 * INPUT:   NONE
 * RETURN:  uint16_t frequency in Hz
 */
uint16_t get_captured_freq(void)
{
    //return the frequency of the input square wave
    return freq_conv_factor/captured_freq;
}

/*
 * Function that sets relevant parameters for timing signals.
 * INPUT:   NONE
 * RETURN:  NONE
 */
void set_freq_clk(void)
{
    TIMER_A0->CTL = 0;
    TIMER_A0->EX0 = 0;
     //set parameters for a 32kHz clock sourced from ACLK
    TIMER_A0->CTL |= TIMER_A_CTL_SSEL__ACLK |
                     TIMER_A_CTL_MC__CONTINUOUS |
                     TIMER_A_CTL_CLR;
    return;
}

/*
 * Function that sets and starts the capture mode timer which
 * determines the frequency from the square wave produced by the
 * external hardware.
 * INPUT:   NONE
 * RETURN:  NONE
 */
void init_freq_timer(void)
{
    //initialize pin used for capture from circuit
    P2->SEL0 |= INPUT_FREQ;
    P2->SEL1 &= ~INPUT_FREQ;
    P2->DIR  &= ~INPUT_FREQ;

    //setup capture mode parameters
    TIMER_A0->CCTL[2] |= TIMER_A_CCTLN_CAP | TIMER_A_CCTLN_CM_1 |
                         TIMER_A_CCTLN_CCIS_0 | TIMER_A_CCTLN_CCIE |
                         TIMER_A_CCTLN_SCS;
    //enable interrupts for capture mode on (2)
    TIMER_A0->CCTL[2] &= ~TIMER_A_CCTLN_CCIFG;
    NVIC->ISER[0] = 1 << ((TA0_N_IRQn) & 31);
    return;
}

//-------------------------------------------------------------------------------------------------
//--------------------------------------Interrupt Handler-----------------------------------------
//-------------------------------------------------------------------------------------------------
void TA0_N_IRQHandler(void)
{
    //variable for "counting" frequency of input wave
    static volatile uint32_t captureCount = 0;
    static volatile uint16_t captureValues[2] = {0,0};

    if ((TIMER_A0->CCTL[2] & TIMER_A_CCTLN_CCIFG))
    {
        //save value for 2 points
        captureValues[captureCount] = TIMER_A0->CCR[2];
        captureCount++;
        //if 2 points have been collected
        if (captureCount >= 2)
        {
            //to indicate frequency is found, set wave_freq_flag
            if ((captureValues[1] - captureValues[0]) > 0)
            {
                captured_freq = (captureValues[1] - captureValues[0]);

                P1 ->OUT ^= BIT0;

                flags |= wave_freq_flag;

                captureValues[0] = 0;
                captureValues[1] = 0;
            }
            captureCount = 0;
        }
    }
    //reset timer flag
    TIMER_A0->CCTL[2] &= ~TIMER_A_CCTLN_CCIFG;
}




