#include <msp430g2553.h>
#include <notes.h> // defining note frequencies

int *global_Notes; // variable for array of note frequency
int *global_Durations; // variable for array of note durations
int global_Length; // variable for length of song
int duration; // variable for current note duration
int which_period; // variable for position in song

void sound_init(void){
    // Configure Timer A0 to give us interrupts being driven by ACLK
    TA0CTL = TASSEL_1 + MC_1;  // ACLK, upmode, counter interrupt enable
    TA0CCR0 = 1500;  // Register 0 counter value to trigger interrupt
    TA0CCTL0 = CCIE;    // CCR0 interrupt enabled

    P2DIR |= BIT5;  // We need P2.5 to be output
    P2DIR |= BIT1; // Set P2.1 as output
//    P2REN |= BIT1;  // enabling resistor,
//    P2OUT &= ~BIT1;
    P2SEL |= BIT5;  // P2.5 is TA1.2 PWM output

    TA1CCTL2 = OUTMOD_7; // Setting for PWM
    TA1CTL = TASSEL_2 + MC_1; // SMCLK, upmode
}

// Sets which song is going to be played
void PlaySound(int *Notes, int *Durations, int Length)
{
    global_Notes = Notes; // setting notes
    global_Durations = Durations; // setting durations
    global_Length = Length; // setting length
    duration = 0; // setting initial note duration as 0
    which_period = 0; // starting song from beginning
    TA1CCR0 = Notes[0]; // setting frequency for first note
    TA1CCR2 = Notes[0]>>1; // divide by 2
}

// Sets the buzzer to play a single note forever
// 0 means the buzzer is off
void PlayNote(int Note)
{
    TA1CCR0 = Note; // setting frequency for note
    TA1CCR2 = Note>>1; // divide by 2
}


// Interrupt Service Routine to play song (moves to next note)
// returns 1 when song is finished
char PlaySoundISR()
{
    // Checking if song is finished
    if (which_period + 1 > global_Length){
        TA1CCR0 = 0; // turning off sound
        TA1CCR2 = 0;
        return 1; // returning 1 as done flag
    } else {
        duration += 1; // increment current note duration
        // check if current note duration is how long note should be played
        if (duration == global_Durations[which_period]){
            TA1CCR0 = 0; // turning off sound for break between notes
        // one interrupt after duration is completed, switch to the next note
        } else if (duration == (global_Durations[which_period] +1)){
            which_period = (which_period + 1) % global_Length; // going to next period
            TA1CCR0 = global_Notes[which_period]; // switching frequency
            TA1CCR2 = global_Notes[which_period]>>1; // making duty cycle 50%
            duration =0; // resetting current note duration
        }
    }
    return 0; // returning 0 as song is not finished
}

// stops song no matter current which period position
void StopSong()
{
    which_period = global_Length; // setting period to past song length to stop song
}
