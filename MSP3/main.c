#include <msp430.h>

// Code for LCD display, 16x2, integrated with the motion sensor on the main board

#define RS BIT0             // bit for register select
#define EN BIT1             // bit for enable
#define D4 BIT2             // bit for D4
#define D5 BIT3             // bit for D5
#define D6 BIT4             // bit for D6
#define D7 BIT5             // bit for D7

volatile char lcd_flag = 0;         // lcd flag is used to control what is displayed on the screen

void delay(unsigned int ms) {       // method to add delay to program execution
    while (ms--) {
        __delay_cycles(1000);
    }
}

void pulseEnable(void) {
    P1OUT &= ~EN;                   // clear enable pin
    delay(1);                       // short delay
    P1OUT |= EN;                    // raise enable
    delay(1);                       // short delay
    P1OUT &= ~EN;                   // clear enable
    delay(1);                       // short delay
}

void send4Bits(unsigned char data) {
    // Set data lines according to the data bits and pulse enable
    P1OUT = (P1OUT & ~(D4 + D5 + D6 + D7)) | ((data << 2) & (D4 + D5 + D6 + D7));
    pulseEnable();
}

void sendInstruction(unsigned char cmd) {
    // send command instructions to the LCD;
    P1OUT &= ~RS;                   // clear RS
    send4Bits(cmd >> 4);            // send higher nibble (four bits)
    send4Bits(cmd & 0xF);           // send lower nibble
    delay(1);
}

void sendData(unsigned char data) {
    // send character data to the display
    P1OUT |= RS;                    // raise RS
    send4Bits(data >> 4);           // send higher nibble
    send4Bits(data & 0xF);          // send lower nibble
    delay(1);
}

void initLCD(void) {
    P1DIR |= RS + EN + D4 + D5 + D6 + D7;           // set control and datalines as output
    P1OUT &= ~(RS + EN + D4 + D5 + D6 + D7);        // clear all output lines

    delay(20);
    sendInstruction(0x28);                          // Function: 4-bit mode, 2 lines, 5x8 dots
    sendInstruction(0x0C);                          // Display ON, Cursor OFF
    sendInstruction(0x01);                          // clear display
    delay(1);                                       // delay to allow clear to complete
    sendInstruction(0x06);                          // Entry mode: increment cursor
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;                       // stop watchdog timer
    BCSCTL1 = CALBC1_1MHZ;                          // set range
    DCOCTL = CALDCO_1MHZ;                           // set DCO step + modulation


    P1DIR |= BIT6;                                  // P1.6 as output LED for debugging
    P1OUT &= ~BIT6;                                 // turn off LED
    P2DIR &= ~BIT4;                                 // 2.4 is input from the motion sensor
    P2IE |=  BIT4;                                  // enable interrupt
    P2IES |= BIT4;                                  // set to trigger on high to low
    P2IFG &= ~BIT4;                                 // clear interrupt flag

    initLCD();

    const char *initialMsg1 = "____Welcome!____";   // welcome msg1
    const char *initialMsg2 = "Enjoy_your_stay!";   // welcome msg2
    const char *onMsg1 = "____Goodbye!____";        // leave msg1
    const char *onMsg2 = "Have_a_nice_day!";        // leave msg2

    _enable_interrupts();

    while (1) {
        if (lcd_flag){
            const char *msg1 = initialMsg1;  // Reset pointer to start of msg1
            sendInstruction(0x80);           // Set cursor at line 0, position 0
            while (*msg1) {
                sendData(*msg1++);           // send msg1
            }

            const char *msg2 = initialMsg2;  // Reset pointer to start of msg2
            sendInstruction(0xC0);           // Line 2
            while (*msg2) {
                sendData(*msg2++);           // send msg1
            }

            delay(200);  // Delay between messages for visibility
        }
        else {
            const char *msg1 = onMsg1;       // Reset pointer to start of msg1
            sendInstruction(0x80);           // Set cursor at line 0, position 0
            while (*msg1) {
                sendData(*msg1++);           // send msg1
            }

            const char *msg2 = onMsg2;       // Reset pointer to start of msg2
            sendInstruction(0xC0);           // Line 2
            while (*msg2) {
                sendData(*msg2++);           // send msg2
            }

            delay(200);  // Delay between messages for visibility
        }
    }
}


// Port 1 service routine used for button press
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void port2 (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) port2 (void)
#else
#error Compiler not supported!
#endif
{
    lcd_flag = ~lcd_flag;                   // toggle lcd_flag
    P1OUT ^= BIT6;                          // toggle LED
    P2IFG &= ~BIT4;                         // Clear Interrupt Flag for P2.4
  __bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)
}
