/*
 * David Koval, Patrick Aung
 * Professor Traver
 * Project 3
 * Feb. 14, 2017
 */

// #include for textbook library header files
#include "pic24_all.h"
#include "stdio.h"

// #defines for handy constants
#define EN_12 LATBbits.LATB12           // Pin RB12 for enabling H-Bridge Driver
#define p_1 255                         // 50% duty cycle
#define p_2 332                         // 65% duty cycle
#define p_3 383                         // 75% duty cycle
#define p_4 434                         // 85% duty cycle
#define p_5 510                         // 100% duty cycle
#define PWM_PERIOD 510                  // 102 microsecond period

// GLOBAL VARIABLE AND FUNCTION DEFINITIONS
uint16_t A1 = 0;                        // A1 input on H-Bridge Driver
uint16_t A2 = 0;                        // A2 input on H-Bridge Driver
uint16_t EdgeA = 0;                     // Most previous edge from IC1 buffer
uint16_t EdgeD = 0;                     // Most recent edge from IC1 buffer
uint16_t IC1_period = 0;                // IC1 interrupt period, difference between EdgeD and EdgeA
float speed = 0.0;                      // Speed of motor in revolutions per second

// Calculates the speed of a motor (rev/s) with a 94.1:1 gear ratio (PPR = 1128.8), provided the period.
float motorSpeed(uint16_t period) {
    return 1.0/(1128.8 * period * 0.0000002);
}

// Initializes the Input Capture Peripheral.
void configIC1(void) {
    CONFIG_IC1_TO_RP(RB9_RP);           // Map IC1 to RB9 (pin 18)
    T3CONbits.TON = 0;                  // Disable Timer 3 when writing to IC control registers
    IC1CONbits.ICTMR = 0;               // Use timer 3
    IC1CONbits.ICM = 0b011;             // Capture every rising edge
    IC1CONbits.ICI = 0b00;              // Interrupt every capture
}

// Configures Output Compare Modules 1 and 2
void configOC(void) {
    T2CONbits.TON = 0;                  // Turns Timer 2 OFF
    CONFIG_OC1_TO_RP(RB1_RP);           // Maps the OC1 output to the remappable pin RB1
    CONFIG_OC2_TO_RP(RB2_RP);           // Maps the OC2 output to the remappable pin RB2
    OC1RS = 0;                          // Clears the OC1RS register
    OC1R = 0;                           // Clears the OC1R register
    OC2RS = 0;                          // Clears the OC2RS register
    OC2R = 0;                           // Clears the OC2R register
    OC1CONbits.OCTSEL = 0;              // Sets the output compare module 1 to use Timer 2 as the clock source
    OC1CONbits.OCM = 0b110;             // Sets it to operate in PWM mode with fault pin disabled
    OC2CONbits.OCTSEL = 0;              // Sets the output compare module 2 to use Timer 2 as the clock source
    OC2CONbits.OCM = 0b110;             // Sets it to operate in PWM mode with fault pin disabled
}

// Configures Timer 2
void configTimer2(void) {
    T2CONbits.TON = 0;                  // Turns Timer 2 OFF
    T2CONbits.TCKPS = 0b01;             // Sets the prescale 1:8
    PR2 = PWM_PERIOD;                   // Sets the period to 102 microseconds
    TMR2 = 0;                           // Clears the Timer 2 register
    T2CONbits.TON = 1;                  // Turns Timer 2 ON
    _T2IF = 0;                          // Clears the flag, T2IF
}

// Configures Timer 3
void configTimer3(void){
    T3CONbits.TON = 0;                  // Disable Timer 3 when writing to IC control registers
    T3CONbits.TCKPS=0b01;               // Prescale 1:8 - Timer 3 clock period = 0.2us
    PR3 = 0xFFFF;                       // Maximum Timer3 interval
    T3CONbits.TCS = 0;                  // (instruction clock = Tcy)
    TMR3 = 0;                           // Clears Timer 3 register
    _T3IF = 0;                          // Clears the flag, T3IF
    T3CONbits.TON = 1;                  // Enables Timer 3
}

// Configures the Timer 2 Interrupt and loads its registers
void _ISR _T2Interrupt(void) {
    OC1RS = A1;                         // Load OC1RS register with A1 (Pin RB1) pulse width
    OC2RS = A2;                         // Load OC2RS register with A2 (Pin RB2) pulse width
    _T2IF = 0;                          // Clears the Timer 2 Interrupt flag, the last instruction in ISR
    
}

// Configures the Input Capture Interrupt Service routine
void _ISR _IC1Interrupt(void) {
    EdgeD = IC1BUF;                     // Read the IC1 buffer to capture the time of the most recent edge
    IC1_period = EdgeD - EdgeA;         // Calculate the counts in one period by subtracting
                                        // Most recent edge from the previous edge.
    EdgeA = EdgeD;                      // Store the most recent edge as the previous edge
    _IC1IF = 0;                         // Clear the IC1 interrupt flag so that interrupts can happen again
    
}

/********** MAIN PROGRAM LOOP********************************/
int main ( void )  //main function that....
{
    /* Define local variables */
    
    
    /* Call configuration routines */
    configClock();                      // Sets the clock to 40MHz using FRC and PLL
    configHeartbeat();                  // Blinks the LED on RA1
    configOC();                         // Configures Output Compare Module 1 & 2
    configIC1();                        // Initializes the Input Capture Peripheral
    configTimer2();                     // Configures Timer 2
    configTimer3();                     // Configures Timer 3
    configUART1(230400);                // The baudrate to match Terminal settings
    CONFIG_RB12_AS_DIG_OUTPUT();        // Initialize PIN RB12 as digital output
    
    /* Initialize ports and other one-time code */
    A1 = p_1;                           // Set motor to initialy rotate left at minimum speed (50% duty cycle)
    A2 = 0;                             // Set A2 to zero to allow motor to turn left
    EN_12 = 1;                          // Enable H-Bridge Driver
    _T2IE = 1;                          // Enable Timer 2 Interrupt
    _IC1IE = 1;                         // Enable Input 1 Capture Interrupt
    char input;                         // User input character
    uint16_t A3 = 0;                    // Temp variable to store previous A1 duty cycle
    
    /* Main program loop */
    while (1) {
        if (isCharReady()) {            // Determine if a character is ready to be read from the serial port
            input = inChar();           // Capture the input character
            if (A1 != 0){               // Check to see if motor is spinning in Left direction
                if (input == 'r'){      // Make motor turn in the Right direction
                    A3 = A1;            // Store speed of motor before switching direction
                    A1 = A2;            // Assign Left direction to 0% duty cycle
                    A2 = A3;            // Assign Right direction to speed of motor going in the previous Left dir.
                    outString("spinning right\r\n");
                }
                else if (input == '1'){
                    A1 = p_1;           // Set motor speed to 50% duty cycle
                    outString("speed 1 \r\n");
                }
                else if (input == '2'){
                    A1 = p_2;           // Set motor speed to 65% duty cycle
                    outString("speed 2 \r\n");
                }
                else if (input == '3'){
                    A1 = p_3;           // Set motor speed to 75% duty cycle
                    outString("speed 3 \r\n");
                }
                else if (input == '4'){
                    A1 = p_4;           // Set motor speed to 85% duty cycle
                    outString("speed 4 \r\n");
                }
                else if (input == '5'){
                    A1 = p_5;           // Set motor speed to 100% duty cycle
                    outString("speed 5 \r\n");
                }
            }
            else if (A2 != 0){          // Check to see if motor is spinning in the Right direction
                if (input == 'l'){      // Make motor turn in the Left direction
                    A3 = A1;            // Store speed of motor before switching direction
                    A1 = A2;            // Assign Right direction to 0% duty cycle
                    A2 = A3;            // Assign Left direction to speed of motor going in the previous Right dir.
                    outString("spinning left\r\n");
                }
                else if (input == '1'){
                    A2 = p_1;           // Set motor speed to 50% duty cycle
                    outString("speed 1 \r\n");
                }
                else if (input == '2'){
                    A2 = p_2;           // Set motor speed to 65% duty cycle
                    outString("speed 2 \r\n");
                }
                else if (input == '3'){
                    A2 = p_3;           // Set motor speed to 75% duty cycle
                    outString("speed 3 \r\n");
                }
                else if (input == '4'){
                    A2 = p_4;           // Set motor speed to 85% duty cycle
                    outString("speed 4 \r\n");
                }
                else if (input == '5'){
                    A2 = p_5;           // Set motor speed to 100% duty cycle
                    outString("speed 5 \r\n");
                }
            }
        }
        outString("The motor is rotating at ");     // Output string to Terminal
        speed = motorSpeed(IC1_period);             // Calculate speed of motor in rev/s from captured period
        printf("%.2f", speed);                      // Display speed to Terminal
        outString(" rev/s\r\n\n");                  // Display units of speed
        DELAY_MS(1000);                             // 1 second delay
    }
}



