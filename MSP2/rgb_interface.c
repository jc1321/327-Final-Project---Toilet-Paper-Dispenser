/*
 * rgb_interface.c
 *
 *  Created on: Mar 12, 2021
 *      Author: ckemere
 */

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "rgb_interface.h"

uint8_t start_frame[] = {0,0,0,0};
uint8_t end_frame[] = {0xFF,0xFF,0xFF,0xFF};
uint8_t off_led[] = {0xE0, 0, 0, 0};
volatile unsigned int i;

void rgb_init_spi(void){
    // SPI should be set on UCB0
    // UCB0SIMO on 1.7, UCB0CLK on 1.5
    P1SEL |= BIT7|BIT5;
    P1SEL2 |= BIT7|BIT5;

    UCB0CTL1= UCSWRST;              //disable serial interface
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;    // data cap at 1st clk edge, MSB first, master mode, synchronous
    UCB0CTL1 |= UCSSEL_2;                           // select SMCLK
    UCB0BR0 = 0;                                    //set frequency
    UCB0BR1 = 0;                                    //
    UCB0CTL1 &= ~UCSWRST;           // Initialize USCI state machine

}
//writes a 32 bit frame to the spi buffer
void rgb_send_frame(const uint8_t *frame1, bool wait_for_completion){
    int byte1;
    for (byte1=0;byte1<4;byte1++){//send 32 bit frame in 8 bit chunks
        UCB0TXBUF=frame1[byte1];
        while (!(IFG2 & UCB0TXIFG));  // USCI_B0 TX buffer ready?
    }
    if (wait_for_completion)
        while (!(IFG2 & UCB0RXIFG));  // USCI_B0 RX buffer ready? (indicates transfer complete)
}

void rgb_send_start() {
    rgb_send_frame(start_frame, false);
}

void rgb_send_end() {
    rgb_send_frame(end_frame, true);
}

// Takes array of colors to set all 35 LEDs
void rgb_set_LEDs(uint8_t **LEDS) {
    rgb_send_start(); // start frame
    // loop for sending color for all 35 arrays
    for (i = 0; i < 35; i++) {
        rgb_send_frame(LEDS[i], false);
    }
    rgb_send_end(); // end frame
}
