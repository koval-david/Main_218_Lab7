/*   
 * David Koval, Patrick
 * Professor Traver
 * Lab 7
 * Feb. 14, 2017
 */

/*********** COMPILER DIRECTIVES *********/

// #include for textbook library header files
#include "adc.h"                // header for simple adc library
#include "pic24_all.h"

// #defines for handy constants
//#define BlackOUT    LATBbits.LATB1      //set pin RB1 (5-black wire) to output
//#define RedIN       LATBbits.LATB2      //set pin RB2 (6-red wire) to output
#define ENABLE      LATBbits.LATB12     //set pin RB12 (23) to enable driver chip

#define PWM_PERIOD 510      //102 microseconds,
#define P_MIN 255           //51 microseconds, encoder period Te(S) min-50%
#define P_MID 383           //                , encoder period Te(S) mid-75%
#define P_MAX 510           //102 microseconds, encoder period Te(S) max-100%

/*********** GLOBAL VARIABLE AND FUNCTION DEFINITIONS *******/
uint16_t Black_pulse_width;         //pulse width for pin 5
uint16_t Red_pulse_width;         //pulse width for pin 6

uint16_t EdgeA;
uint16_t EdgeD;
uint16_t IC1_period;


// Configures Output compare module 1 for continuous Ratation Servo
void configOC1() {
    T2CONbits.TON = 0;        //Turns timer 2 off.
    CONFIG_OC1_TO_RP(RB1_RP); //Maps the OC1 output to the remappable pin, RB1.
    OC1RS = 0;                //Clears the RS register
    OC1R = 0;                 //Clears the R register
    OC1CONbits.OCTSEL = 0;    //Sets the output compare module to use Timer 2 as the clock source.
    OC1CONbits.OCM = 0b110;   //Sets it to operate in PWM mode with fault pin disabled.
}

// Configures Output compare module 2 for positional Servo
void config0C2(void){
    T2CONbits.TON = 0;          //Turns timer 2 off.
    CONFIG_OC2_TO_RP(RB2_RP);   //Maps the OC2 output to the remappable pin, RB2.
    OC2RS = 0;                  //Clears the OC2RS register
    OC2R = 0;                   //Clears the OC2R register
    OC2CONbits.OCTSEL = 0;      //Sets the output compare module to use Timer 2 as the clock source.
    OC2CONbits.OCM = 0b110;     //Sets it to operate in PWM mode with fault pin disabled.
}

void configIC1(void) {
    CONFIG_IC1_TO_RP(RB9_RP); //Map IC1 to RB9 (pin 18)
    T3CONbits.TON = 0; //Disable Timer3 when writing to IC control registers
    IC1CONbits.ICTMR = 0; //use timer 3
    IC1CONbits.ICM = 0b011; //capture every rising edge.
    IC1CONbits.ICI = 0b00; //interrupt every capture
}

//Configures Timer 2
void configTimer2() {
    T2CON = 0x0010;             //Configs timer 2, with presacle 1:8, equivalent to 0b01=8
    PR2 = PWM_PERIOD;           //Sets period
    TMR2 = 0;                   //Clears the timer 2 register.
    _T2IF = 0;                  //Clears the flag, T2IF
}

//Configures Timer 3
void configTimer3(void){
    T3CONbits.TON = 0; //Disable Timer3 when writing to IC control registers
    T3CONbits.TCKPS=0b01; //prescale 1:8 - Timer 3 clock period = 0.2us
    PR3 = 0xFFFF; //Maximum Timer3 interval
    T3CONbits.TCS = 0; //(instruction clock = Tcy)
    TMR3 = 0;
    _T3IF = 0;
}

//Scales adcvalue to proportional value in the range of p_min and p_max
//float scale(uint16_t u16_x, uint16_t p_min, uint16_t p_max){
//    return (float)(((u16_x/4095.0)*(p_max-p_min)) + p_min);
//}

//Configures the timer 2 interupt and loads its registers
void _ISR _T2Interrupt(void) {
    OC1RS = Black_pulse_width;      //Load OC1RS register with Continuous pulse width
    OC2RS = Red_pulse_width;      //Load OC2RS register with Positional pulse width
    _T2IF = 0;                  //Clears the Timer 2 interrupt flag, the last instruction in ISR
}

void _ISR _IC1Interrupt(void){
    EdgeD = IC1BUF; //Read the IC1 buffer to capture the time of the most recent edge
    IC1_period = EdgeD - EdgeA; //Calculate the counts in one period by subtracting
    //most recent edge from the previous edge.
    EdgeA = EdgeD; //Store the most recent edge as the previous edge
    _IC1IF = 0; //Clear the IC1 interrupt flag so that interrupts can happen again
}

//void turnLeft(void){
//    BlackOUT = 0;
//    RedIN = 1;
//}
//void turnRight(void){
//    BlackOUT = 1;
//    RedIN = 0;
//}
/********** MAIN PROGRAM LOOP********************************/
int main ( void )  //main function that....
{ 
/* Define local variables */


/* Call configuration routines */
	configClock();  //Sets the clock to 40MHz using FRC and PLL
  	configHeartbeat(); //Blinks the LED on RA1
    configOC1(); //Configures output compare module 1 for Continuous motor
    config0C2(); //Configures output compare module 2 for Positional motor
    configIC1();
    configTimer2(); //Configures timer 2
    configTimer3();
    configUART1(230400); //Configure UART1 for 230400 baud
    
    

/* Initialize ports and other one-time code */
    
    _T2IE = 1;              //Enables the timer 2 interrupts
    T2CONbits.TON = 1;      //Turns timer 2 on
    
    TRISB = 0;  //set all B ports to output
    ENABLE = 1; //set enable to high
    
/* Main program loop */
    while (1) {
//        DELAY_MS(500);
//        Red_pulse_width=0;
//        for (Black_pulse_width=P_MIN; Black_pulse_width < P_MAX; Black_pulse_width++) {
//            DELAY_MS(50);
//        }
//        for (Black_pulse_width=P_MAX; Black_pulse_width > P_MIN; Black_pulse_width--) {
//            DELAY_MS(50);
//        }
//        Black_pulse_width=0;
//        for (Red_pulse_width=P_MIN; Red_pulse_width < P_MAX; Red_pulse_width++) {
//            DELAY_MS(50);
//        }
//        for (Red_pulse_width=P_MAX; Red_pulse_width > P_MIN; Red_pulse_width--) {
//            DELAY_MS(50);
//        }

        Black_pulse_width = (P_MAX-P_MIN)/2+P_MIN;
        Red_pulse_width = 0;
        
        //PART A
//        BlackOUT = 0;
//        RedIN = 1;
//        DELAY_MS(5000);
//        BlackOUT = 0;
//        RedIN = 0;
//        DELAY_MS(1000);
//
//        BlackOUT = 1;
//        RedIN = 0;
//        DELAY_MS(5000);
//        BlackOUT = 0;
//        RedIN = 0;
//        DELAY_MS(1000);
        
    }
        
}
