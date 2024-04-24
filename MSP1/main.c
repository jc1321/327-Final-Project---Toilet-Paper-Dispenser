// Code for the main board, with motion sensor 1, motion sensor 2 , a button
// buzzer, and motor
// Motion sensor 1 is connected to the other two boards, and controls whether
// sound is played
// Motion sensor 2 activates the motor to unroll toliet paper
// The button changes which song is being played
#include <msp430g2553.h>
#include <notes.h>   // defining global constants (note frequencies)
#include "playsound.h"

#define EIGHTH_SEC 1500  // Timing constant for 1/8 second interrupts
#define ONE_SEC 12000    // Timing constant for one second interrupts

char start_play = 0; // flag to indicate start of song
char song = 0; // flag to indicate which song is being played
char wait = 0; // flag to indicate whether to wait before playing next song
char play = 0; // flag to indicate whether sound should be played or not

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;    // Stop WDT
    BCSCTL3 |= LFXT1S_2;         // ACLK = VLO
    BCSCTL1 = CALBC1_1MHZ;    // Set the DCO to 1 MHz
    DCOCTL = CALDCO_1MHZ;     // And load calibration data

    // P2.2 and P2.4 are for motion sensors (motor or sound and LEDs)
    // P2.0 is for button (switch song)
    P2DIR &= ~BIT2 & ~BIT4 & ~BIT0; // Set P2.0, P2.2, and P2.4 as inputs
    P2IE |= BIT2 + BIT4 + BIT0;   // Enable interrupt for P2.0, P2.2, and P2.4
    P2IES |= BIT2 +BIT4 + BIT0;  // Set with high-to-low transition

    P1DIR |= BIT6 + BIT0; // Output for debugging
    P2DIR |= BIT3; // Output pin for motor control
    sound_init(); // initializing sound
    _enable_interrupts();                       // enable global interrupts

    __bis_SR_register(GIE + LPM3_bits); // enabling interrupts and entering LPM3
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

// Timer 0 interrupt service routine used to control playing sound
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A0 (void)
#else
#error Compiler not supported!
#endif
{
    // Twinkle Twinkle Little Star
    int tNotes[] = {c,c,g,g,a,a,g,f,f,e,e,d,d,c,
                    g,g,f,f,e,e,d,g,g,f,f,e,e,d,
                    c,c,g,g,a,a,g,f,f,e,e,d,d,c}; // defining notes
    int tDurations[] = {2,2,2,2,2,2,4,2,2,2,2,2,2,4,
                        2,2,2,2,2,2,4,2,2,2,2,2,2,4,
                        2,2,2,2,2,2,4,2,2,2,2,2,2,4}; // defining length of notes in 1/8 seconds
    int tLength = 42; // defining length of song
    
    // Mary Had a Little Lamb
    int mNotes[] = {e,d,c,d,e,e,e,d,d,d,e,g,g,
                    e,d,c,d,e,e,e,e,d,d,e,d,c}; // defining notes
    int mDurations[] = {2,2,2,2,2,2,4,2,2,4,2,2,4,
                        2,2,2,2,2,2,2,2,2,2,2,2,8}; // defining length of notes in 1/8 seconds
    int mLength = 26; // defining length of song

    // London Bridge is Falling Down
    int lNotes[]={g,a,g,f,e,f,g,d,e,f,e,f,g,g,a,g,f,e,f,g,d,g,e,c};
    int lDurations[]= {2,1,2,2,2,2,4,2,2,4,2,2,4,2,1,2,2,2,2,4,4,4,2,6};
    int lLength=24;

    // if song should be played as triggered by motion sensor
    if (play == 1){
        // if start of the song
        if (start_play == 1){
            // setting which song will be played depending on variable song
            if (song == 0)
                // Playing twinkle twinkle little stars
                PlaySound(tNotes,tDurations,tLength);
            else if (song == 1)
                // Playing Mary had a little lamb
                PlaySound(mNotes,mDurations,mLength);
            else
                // Playing London Bridge is Falling Down
                PlaySound(lNotes, lDurations, lLength);
            // resetting start_play flag
            start_play = 0;
        }
        char done = PlaySoundISR(); // using play sound interrupt service routine
        // If song is finished, wait one additional interrupt before repeating
        if(done == 1 && wait == 0){
            TA0CCR0 = ONE_SEC; // wait for one second before starting
            wait = 1;  // setting wait flag as complete
        // triggers at next interrupt following song completion
        } else if (wait == 1) {
            wait = 0; // resetting wait flag
            TA0CCR0 = EIGHTH_SEC; // setting interrupt to trigger every 1/8 second
            start_play = 1; // mark the start of the song
        }
    } else {
        PlayNote(0); // turning sound off
        StopSong(); // stopping current song
    }
    
    __bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)
}

// Port 2 service routine used for motion sensors and button press
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void port2 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) port2 (void)
#else
#error Compiler not supported!
#endif
{
    // Motion sensor # 2 interrupt to control motor
    if ((P2IFG & BIT2) == BIT2) {
        P2OUT ^= BIT3;  // Toggle motor
        P2IFG &= ~BIT2; // Clear Interrupt Flag for P2.2
    }

    // Motion sensor # 1 interrupt to control sound and LEDs
    if ((P2IFG & BIT4) == BIT4){
        play ^= 1; // Flag to start playing music
        start_play ^= 1; // Flag to mark start of song
        P1OUT ^= BIT6; // Toggling Red LED for debugging purposes
        P2IFG &= ~BIT4; // Clear Interrupt Flag for P2.4
    }

    // Button interrupt to control which song is being played
    if((P2IFG & BIT0)== BIT0){
        P1OUT ^= BIT0; // Toggling Green LED on board for debugging purposes
        StopSong(); // Stopping the song
        PlayNote(0); // Turning sound off
        song = (song + 1) % 3; // Switching between the three songs
        start_play = 1; // Flag to mark start of song
        P2IFG &= ~BIT0; // Clear Interrupt Flag for P2.0
    }

  __bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)
}
