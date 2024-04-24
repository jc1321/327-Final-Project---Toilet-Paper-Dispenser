#include <msp430g2553.h>
#include <stdint.h>
#include <stdbool.h>
#include "rgb_interface.h"

volatile unsigned int i;
volatile char led_flag = 0;
/*
 * Variables
 */
// Defining basic colors
uint8_t red[] = {0xF0, 0, 0, 10};
uint8_t blue[] = {0xF0, 10, 0, 0};
uint8_t green[] = {0xF0, 0, 10, 0};
uint8_t yellow[] = {0xF0, 0, 10, 10};
uint8_t off[] = {0xE0, 0, 0, 0};
// Defining rainbow colors
uint8_t deepRed[] = {0xF0, 0, 1, 30};
uint8_t ruby[] = {0xF0, 2, 2, 30};
uint8_t orangeRed[] = {0xF0, 0, 5, 25};
uint8_t orange[] = {0xF0, 0, 5, 50};
uint8_t orangeYellow[] = {0xF0, 0, 15, 50};
uint8_t yellowPlus[] = {0xF0, 0, 20, 20};
uint8_t pear[] = {0xF0, 0, 15, 10};
uint8_t yellowGreen[] = {0xF0, 0, 20, 5};
uint8_t lime[] = {0xF0, 0, 20, 10};
uint8_t lightGreen[] = {0xF0, 0, 25, 0};
uint8_t mint[] = {0xF0, 5, 25, 5};
uint8_t turqoise[] = {0xF0, 5, 15, 1};
uint8_t cyan[] = {0xF0, 10, 20, 0};
uint8_t teal[] ={0xF0, 10, 10, 0};
uint8_t aqua[] = {0xF0, 20, 10, 0};
uint8_t deepBlue[] = {0xF0, 10, 1, 0};
uint8_t purpleBlue[] =   {0xF0, 30, 0, 2};
uint8_t bluePink[] = {0xF0, 20, 0, 1};
uint8_t indigo[] = {0xF0, 15, 0, 5};
uint8_t violet[] = {0xF0, 20, 0, 10};
uint8_t purple[] = {0xF0, 10, 0, 10};
uint8_t magenta[] = {0xF0, 20, 0, 20};
uint8_t peach[] = {0xF0, 5, 5, 20};
uint8_t hotpink[] = {0xF0, 10, 0, 20};
uint8_t pink[] = {0xF0, 5, 1, 20};
uint8_t lightPink[] = {0xF0, 2, 2, 15};
uint8_t whitePink[] = {0xF0, 5, 5, 25};
uint8_t rose[] = {0xF0, 10, 5, 25};
uint8_t amethyst[] = {0xF0, 15, 5, 15};
uint8_t lavender[] = {0xF0, 15, 5, 20};
uint8_t white[] = {0xF0, 10, 10, 10};

// Array of 35 colors in rainbow order, used to set LEDs
uint8_t* colors[35] = {red, deepRed, ruby, orangeRed, orange, orangeYellow, yellow, yellowPlus, pear, yellowGreen,
lime, lightGreen, green, mint, turqoise, cyan, teal, aqua, deepBlue, blue, purpleBlue, bluePink, indigo, violet,
purple, magenta, peach, hotpink, pink, lightPink, whitePink, rose, amethyst, lavender, indigo};
// Array to turn off all 35 LEDs
uint8_t* offArray[35] = {off, off, off, off, off, off, off, off, off, off, off, off, off, off, off, off, off, off, off, off,
                         off, off, off, off, off, off, off, off, off, off, off, off, off, off, off};

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;    // Stop WDT
    BCSCTL3 |= LFXT1S_2;         // ACLK = VLO
    BCSCTL1 = CALBC1_1MHZ;    // Set the DCO to 1 MHz
    DCOCTL = CALDCO_1MHZ;     // And load calibration data


    P1DIR |=  BIT6; // Setting 1.6 as output (red LED on board)
    P1OUT &= ~BIT6; // Turning 1.6 off

    P2DIR &= ~BIT4; // Set P2.4 as input (motion sensor #1)
    P2IE |= BIT4;   // Enable interrupt
    P2IES |= BIT4;  // Set with high-to-low transition

    rgb_init_spi(); // Initialize rgb
    _enable_interrupts();                       // enable global interrupts
    __bis_SR_register(GIE + LPM0_bits); // enabling interrupts and entering LPM3
}

// Watchdog Timer interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(WDT_VECTOR))) watchdog_timer (void)
#else
#error Compiler not supported!
#endif
{
    __bic_SR_register_on_exit(LPM3_bits); // exit LPM3 when returning to program (clear LPM3 bits)
}

// Port 2 service routine used for button press
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void port2 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) port2 (void)
#else
#error Compiler not supported!
#endif
{
    // motion sensor #1 interrupt from P2.4
    if ((P2IFG & BIT4) == BIT4){
        led_flag ^= 1; // toggle flag to turn on or off LEDs
        if (led_flag == 1){
            rgb_set_LEDs(colors); // turning on LEDs to rainbow colors
        } else {
            rgb_set_LEDs(offArray); // turning off LEDs
        }
        P1OUT ^= BIT6; // toggling P1.6 (red LED) for debugging purposes
    }
    P2IFG &= ~BIT4; // Clear Interrupt Flag for P1.7
  __bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)
}
