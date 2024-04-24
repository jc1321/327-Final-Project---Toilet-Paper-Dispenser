#include <msp430.h>

#define RS BIT0
#define EN BIT1
#define D4 BIT2
#define D5 BIT3
#define D6 BIT4
#define D7 BIT5

volatile char lcd_flag = 0;

void delay(unsigned int ms) {
    while (ms--) {
        __delay_cycles(1000); // Adjust according to your clock settings
    }
}

void pulseEnable(void) {
    P1OUT &= ~EN;
    delay(1);
    P1OUT |= EN;
    delay(1);
    P1OUT &= ~EN;
    delay(1);
}

void send4Bits(unsigned char data) {
    P1OUT = (P1OUT & ~(D4 + D5 + D6 + D7)) | ((data << 2) & (D4 + D5 + D6 + D7)); // Adjusted shift for correct pin mapping
    pulseEnable();
}

void sendInstruction(unsigned char cmd) {
    P1OUT &= ~RS;
    send4Bits(cmd >> 4);
    send4Bits(cmd & 0xF);
    delay(1);
}

void sendData(unsigned char data) {
    P1OUT |= RS;
    send4Bits(data >> 4);
    send4Bits(data & 0xF);
    delay(1);
}

void initLCD(void) {
    P1DIR |= RS + EN + D4 + D5 + D6 + D7;
    P1OUT &= ~(RS + EN + D4 + D5 + D6 + D7);

    delay(20);
    sendInstruction(0x28);
    sendInstruction(0x0C);
    sendInstruction(0x01);
    delay(1);
    sendInstruction(0x06);
}

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;


    P1DIR |= BIT6;
    P1OUT &= ~BIT6;
    P2DIR &= ~BIT4;
    P2IE |=  BIT4;
    P2IES |= BIT4;
    P2IFG &= ~BIT4;

    initLCD();

    const char *initialMsg1 = "____Welcome!____";
    const char *initialMsg2 = "Enjoy_your_stay!";
    const char *onMsg1 = "____Goodbye!____";
    const char *onMsg2 = "Have_a_nice_day!";

    _enable_interrupts();

    while (1) {
        if (lcd_flag){
            const char *msg1 = initialMsg1;  // Reset pointer to start of msg1
            sendInstruction(0x80);           // Set cursor at line 0, position 0
            while (*msg1) {
                sendData(*msg1++);
            }

            const char *msg2 = initialMsg2;  // Reset pointer to start of msg2
            sendInstruction(0xC0);           // Line 2
            while (*msg2) {
                sendData(*msg2++);
            }

            delay(200);  // Delay between messages for visibility
        }
        else {
            const char *msg1 = onMsg1;  // Reset pointer to start of msg1
            sendInstruction(0x80);           // Set cursor at line 0, position 0
            while (*msg1) {
                sendData(*msg1++);
            }

            const char *msg2 = onMsg2;  // Reset pointer to start of msg2
            sendInstruction(0xC0);           // Line 2
            while (*msg2) {
                sendData(*msg2++);
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
    lcd_flag = ~lcd_flag;
    P1OUT ^= BIT6;
    P2IFG &= ~BIT4; // Clear Interrupt Flag for P1.7
  __bic_SR_register_on_exit(LPM3_bits);     // Clear LPM3 bits from 0(SR)
}
