/* Engineer(s): Ezzeddeen Gazali and Tyler Starr
 * Create Date: 10/14/2018
 * Description: Program that implements a frequency measurement system
 * using the MSP432 MCU. The MCU is capable of measuring the frequency of a
 * square wave input signal between 0V and 3.3V.
 */
#include "msp.h"
#include "freq_meas_functions.h"
#include "LCD.h"

/*
 * Function that converts a binary number to a BCD number
 * INPUTS:      uint16_t binary_number = binary number to be converted
 * RETURN:      uint16_t bcd_number = BCD representation of the input binary number
 */
uint16_t binary_to_bcd(uint16_t binary_number)
{
    //variable initialization.
    uint16_t bcd_number = 0;
    uint8_t digit = 0;

    //algorithm which converts binary to BCD.
    while (binary_number > 0)
    {
        bcd_number |= (binary_number % 10) << (digit++ << 2);
        binary_number /= 10;
    }
    return bcd_number;
}

/*
 * Function that prints a given frequency onto the LCD.
 * INPUTS:      uint16_t frequency = frequency to be printed to LCD
 * RETURN:      NONE
 */
void freq_to_LCD(uint16_t frequency)
{
    //variable initialization
    char frequency_string[] = {'0','0','0','0','\0'};
    uint16_t bcd_num = 0;
    uint16_t temp = 0;
    uint8_t shift = 12;
    uint8_t i;


    //convert input frequency to a BCD number
    bcd_num = binary_to_bcd(frequency);

    //algorithm for copying the BCD frequency to character array
    for(i = 0; i < 4;i++)
    {
        temp = (bcd_num & (0xF000 >> (12 - shift))) >> shift;
        frequency_string[i] = temp + 48;
        shift -= 4;
    }

    //The following code segment replaces all leading zero's with the space character ' '
    //For example: 1000 => 1000, 0100 => 100, 0010 => 10, and 0001 => 1
    if(frequency < 1000 && frequency >= 100)
    {
        frequency_string[0] = ' ';
    }
    else if(frequency < 100 && frequency >= 10)
    {
        frequency_string[0] = ' ';
        frequency_string[1] = ' ';
    }
    else if(frequency < 10)
    {
        frequency_string[0] = ' ';
        frequency_string[1] = ' ';
        frequency_string[2] = ' ';
    }

    if (frequency == 0xFFFF)
    {
        frequency_string[0] = '-';
        frequency_string[1] = '-';
        frequency_string[2] = '-';
        frequency_string[3] = '-';
        frequency_string[4] = '\0';
    }

    //print frequency on LCD
    LCD_CLR();
    LCD_HOME();
    WRITE_STR_LCD(frequency_string);
    WRITE_STR_LCD(" Hz");
    return;
}

/*
 * Function that prints a given foot-candle onto the LCD.
 * INPUTS:      uint16_t frequency = foot-candle to be printed to LCD
 * RETURN:      NONE
 */
void ftCandle_to_LCD(uint16_t ftCandle)
{
    //variable initialization
    char ftCandle_String[] = {'0','0','0','0','\0'};
    uint16_t bcd_num = 0;
    uint16_t temp = 0;
    uint8_t shift = 12;
    uint8_t i;


    //convert input foot-candle to a BCD number
    bcd_num = binary_to_bcd(ftCandle);

    //algorithm for copying the BCD foot-candle to character array
    for(i = 0; i < 4;i++)
    {
        temp = (bcd_num & (0xF000 >> (12 - shift))) >> shift;
        ftCandle_String[i] = temp + 48;
        shift -= 4;
    }

    //The following code segment replaces all leading zero's with the space character ' '
    //For example: 1000 => 1000, 0100 => 100, 0010 => 10, and 0001 => 1
    if(ftCandle < 1000 && ftCandle >= 100)
    {
        ftCandle_String[0] = ' ';
    }
    else if(ftCandle < 100 && ftCandle >= 10)
    {
        ftCandle_String[0] = ' ';
        ftCandle_String[1] = ' ';
    }
    else if(ftCandle < 10)
    {
        ftCandle_String[0] = ' ';
        ftCandle_String[1] = ' ';
        ftCandle_String[2] = ' ';
    }

    if (ftCandle == 0xFFFF)
    {
        ftCandle_String[0] = '-';
        ftCandle_String[1] = '-';
        ftCandle_String[2] = '-';
        ftCandle_String[3] = '-';
        ftCandle_String[4] = '\0';
    }

    //print foot-candle to LCD
    LCD_COMMAND(Second_Line);
    WRITE_STR_LCD(ftCandle_String);
    WRITE_STR_LCD(" Foot-Candle");
    return;
}

/*
 * Function that implements the logic for determining
 * the frequency of the input wave, or if the input
 * signal is DC.
 * INPUT:   NONE
 * RETURN:  NONE
 */
void wave_freq_logic(void)
{
    //define variable for frequency timeout
    uint32_t timeout_count = 0;
    //setup the timer used for determining the frequency
    set_freq_clk();
    //set the frequency conversion factor
    set_freq_conversion(32000);
    //start capture-mode timer to get frequency
    init_freq_timer();
    //wait until flag indicates frequency has been determined
    while((get_flags() & wave_freq_flag) == 0){
        //increment timeout count.
        timeout_count++;
        //if timeout count reaches this the wave is assumed to be DC
        if (timeout_count > 5000000)
        {
            //set the DC flag and the freq found flag
            set_flags(dc_flag | wave_freq_flag);
            //disable interrupts for frequency timer
            NVIC->ICER[0] = 1 << ((TA0_N_IRQn) & 31);
            return;
        }
    }
    //disable interrupts for frequency timer
    NVIC->ICER[0] = 1 << ((TA0_N_IRQn) & 31);
    return;
}


void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
    __enable_irq();                                 //enable interrupts globally

    //initialize LCD, clear screen and set cursor to home position
    LCD_init();
    LCD_CLR();
    LCD_HOME();


    P1 ->SEL0 &= ~BIT0;
    P1 ->SEL1 &= ~BIT1;
    P1 ->DIR |= BIT0;

    //initialize clocks
    init_clock();

    //variable initializations
    uint16_t freq = 0;
    uint16_t old_freq = 0;
    uint16_t foot_candle= 0;

    //continuously poll for new frequency measurement.
    while (1)
    {
        wave_freq_logic();                  //get frequency
        freq = get_captured_freq() + 1;     //tune measured frequency
        foot_candle = 3.25*freq + 1.75 ;          //formula for determining freq-> ft candle
        //If a DC signal is measured display invalid measurement.
        if(get_flags() & dc_flag)

        {
            LCD_CLR();
            LCD_HOME();
            WRITE_STR_LCD("    Invalid       ");
            LCD_COMMAND(Second_Line);
            WRITE_STR_LCD("   DC Signal");
        }

        //otherwise, if a new frequency is measured display the measured frequency and
        //foot candles
        else if(freq != old_freq)
        {
           freq_to_LCD(freq);
           ftCandle_to_LCD(foot_candle);
           old_freq = freq;

           //for frequencies not within 5-70Hz range, indicate that the foot candle
           //measurement is out of range.
           if (!(freq >= 1 && freq <= 61))
                  {
                      LCD_COMMAND(Second_Line);
                      WRITE_STR_LCD(Clear_Line);
                      LCD_COMMAND(Second_Line);
                      WRITE_STR_LCD(" Out of Range");
                  }
        }
        //reset all flags
        reset_flags(0xFF);
        __delay_cycles(1200000);
    }
}
