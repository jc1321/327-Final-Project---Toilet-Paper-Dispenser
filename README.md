# 327 Final Project - Toilet Paper Dispenser
## Project Overview <br>
For our final project, we have created an automatic toilet paper dispenser to create a luxurious bathroom experience. <br>
<br> 
As the user enters the bathroom, our motion sensors triggers a series of actions. One activates the buzzer, turns on the LED strip, and changes the message displayed on the LCD. The LED strip displays a line of 35 distinct colors, and the buzzer plays 3 songs. The user can press the button to change between the 3 songs played, and triggers the motion sensor to change the message displayed from "Welcome! Enjoy your stay!" to "Goodbye! Have a nice day!". <br>
The second motion sensor triggers a DC motor which is attached to the toilet paper roll. The user can wave the motion sensor off to stop the rolling, creating a hands-free bathroom experience. <br> 

## Hardware Architecture <br>
![327 Block-1 (1)](https://github.com/jc1321/327-Final-Project---Toilet-Paper-Dispenser/assets/90633740/9b190e98-d41f-4853-a8a3-fae16e274e6a)
3 MSP430s make up our hardware architecture. <br> 
* MSP430 #1 drives the buzzer, a DC motor, a button, and 2 motion sensors. P2.1 connects to the anode of the buzzer and P2.5 connects to the cathode side. P2.4 connects to motion sensor #1 that is driven by 5V, which controls the LED strip and the buzzer. P2.3 connects to IN1 of the motor driver which is driven by 3.3 V, OUT2 routes to a 5.1Ω resistor and the DC motor, and OUT1 directly connects to the other side of the DC motor. P2.0 connects a button that's also driven by 3.3V. P2.2 connects to motion sensor #2 that is driven by 5V. <br>
* MSP430 #2 drives the LED strip. The LED strip consists of 35 LEDs and 4 pins - VDD, Cl, Dl, and GND. VDD is connected to 5V, Cl to P1.5 and Dl to P1.7. <br>
* MSP430 #3 drives the LCD screen. VSS V0 and RW are grounded and VDD is connected to 5V. D4-D7 connects to P1.2-P1.5 on the MSP430. P2.4 is connected to motion sensor #1. <br> 

## Software Architecture <br>
* MSP430 #1 uses port 2 ISR to control motor and music being played. When P2.2 (motion sensor #2) is triggered, we toggle the motor to start/stop. When P2.4 (motion sensor #1) triggers the interrupt, a flag is toggled to start/stop playing music on the buzzer. And when P2.0 (button) is triggered, the current song is stopped and a new song is switched on. It also uses several helper source and header files. notes.h defines note frequencies and playsound.c defines helper functions to configure pins for the buzzer and move to the next note in the current song. A duration is defined to be 1/8th of a second, and a second is waited before repeating a song. <br>
* MSP430 #2 also uses port 2 ISR to control the LED strip. When P2.4 (motion sensor #1) is triggered, the LED strip is toggled on/off. The LED strip displays 35 distinct colors, and uses helper source and header files rgb_interface.c to display the LED colors through SPI on UCB0. <br>
* MSP430 #3 uses the same motion sensor (motion sensor #2) to toggle the message on the LCD screen. Whenever P2.4 interrupt is triggered, the message on the LCD screen is changed. <br> 
