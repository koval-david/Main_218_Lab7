/*
 * David Koval, Patrick
 * Professor Traver
 * Project 3
 * Feb. 14, 2017
 */

// #include for textbook library header files
#include "pic24_all.h"
#include "stdio.h"

// #defines for handy constants
#define EN_12 LATBbits.LATB12
#define p_1 255
#define p_2 332
#define p_3 383
#define p_4 434
#define p_5 510
#define PWM_PERIOD 510

/*********** GLOBAL VARIABLE AND FUNCTION DEFINITIONS *******/
uint16_t A1 = 0;
uint16_t A2 = 0;
uint16_t EdgeA = 0;
uint16_t EdgeD = 0;
uint16_t IC1_period = 0;
float speed = 0.0;

float motorSpeed(uint16_t period) {
    return 1.0/(1128.8 * period * 0.0000002);
}

void configIC1(void) {
    CONFIG_IC1_TO_RP(RB9_RP); //Map IC1 to RB9 (pin 18)
    T3CONbits.TON = 0; //Disable Timer3 when writing to IC control registers
    IC1CONbits.ICTMR = 0; //use timer 3
    IC1CONbits.ICM = 0b011; //capture every rising edge.
    IC1CONbits.ICI = 0b00; //interrupt every capture
}

void configOC(void) {
    T2CONbits.TON = 0;
    CONFIG_OC1_TO_RP(RB1_RP);
    CONFIG_OC2_TO_RP(RB2_RP);
    OC1RS = 0;
    OC1R = 0;
    OC2RS = 0;
    OC2R = 0;
    OC1CONbits.OCTSEL = 0;
    OC1CONbits.OCM = 0b110;
    OC2CONbits.OCTSEL = 0;
    OC2CONbits.OCM = 0b110;
}

void configTimer2(void) {
    T2CONbits.TON = 0;
    T2CONbits.TCKPS = 0b01;
    PR2 = PWM_PERIOD;
    TMR2 = 0;
    T2CONbits.TON = 1;
    _T2IF = 0;
}

void configTimer3(void){
    T3CONbits.TON = 0; //Disable Timer3 when writing to IC control registers
    T3CONbits.TCKPS=0b01; //prescale 1:8 - Timer 3 clock period = 0.2us
    PR3 = 0xFFFF; //Maximum Timer3 interval
    T3CONbits.TCS = 0; //(instruction clock = Tcy)
    TMR3 = 0;
    _T3IF = 0;
    T3CONbits.TON = 1;
}

void _ISR _T2Interrupt(void) {
    OC1RS = A1;
    OC2RS = A2;
    _T2IF = 0;
    
}

void _ISR _IC1Interrupt(void) {
    EdgeD = IC1BUF; //Read the IC1 buffer to capture the time of the most recent edge
    IC1_period = EdgeD - EdgeA; //Calculate the counts in one period by subtracting
    //most recent edge from the previous edge.
    EdgeA = EdgeD; //Store the most recent edge as the previous edge
    _IC1IF = 0; //Clear the IC1 interrupt flag so that interrupts can happen again
    
}

/********** MAIN PROGRAM LOOP********************************/
int main ( void )  //main function that....
{
    /* Define local variables */
    
    
    /* Call configuration routines */
    configClock();  //Sets the clock to 40MHz using FRC and PLL
    configHeartbeat(); //Blinks the LED on RA1
    configOC();
    configIC1();
    configTimer2();
    configTimer3();
    configUART1(230400);
    CONFIG_RB12_AS_DIG_OUTPUT();
    
    /* Initialize ports and other one-time code */
    A1 = p_1;
    A2 = 0;
    EN_12 = 1;
    _T2IE = 1;
    _IC1IE = 1;
    char input;
    uint16_t A3 = 0;
    
    /* Main program loop */
    while (1) {
        if (isCharReady()) {
            input = inChar();
            if (A1 != 0){
                if (input == 'r'){
                    A3 = A1;
                    A1 = A2;
                    A2 = A3;
                    outString("spinning right\r\n");
                }
                else if (input == '1'){
                    A1 = p_1;
                    outString("speed 1 \r\n");
                }
                else if (input == '2'){
                    A1 = p_2;
                    outString("speed 2 \r\n");
                }
                else if (input == '3'){
                    A1 = p_3;
                    outString("speed 3 \r\n");
                }
                else if (input == '4'){
                    A1 = p_4;
                    outString("speed 4 \r\n");
                }
                else if (input == '5'){
                    A1 = p_5;
                    outString("speed 5 \r\n");
                }
            }
            else if (A2 != 0){
                if (input == 'l'){
                    A3 = A1;
                    A1 = A2;
                    A2 = A3;
                    outString("spinning left\r\n");
                }
                else if (input == '1'){
                    A2 = p_1;
                    outString("speed 1 \r\n");
                }
                else if (input == '2'){
                    A2 = p_2;
                    outString("speed 2 \r\n");
                }
                else if (input == '3'){
                    A2 = p_3;
                    outString("speed 3 \r\n");
                }
                else if (input == '4'){
                    A2 = p_4;
                    outString("speed 4 \r\n");
                }
                else if (input == '5'){
                    A2 = p_5;
                    outString("speed 5 \r\n");
                }
            }
        }
        outString("The motor is rotating at ");
        speed = motorSpeed(IC1_period);
        printf("%.2f", speed);
        outString(" rev/s\r\n\n");
        DELAY_MS(1000); 
    }
}



