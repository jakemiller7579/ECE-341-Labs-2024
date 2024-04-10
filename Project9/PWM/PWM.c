#include <plib.h>
#include "PWM.h"
#include "LCDLIB.h"
#include "CerebotMX7cK.h"
#include <stdio.h>
#include <string.h>


//global variables
int PR; 
int DC=40;

int main()
{
    int freq=1000;
    pwm_init(DC,freq);
    while(1);
}

//CN ISR:
void __ISR(_CHANGE_NOTICE_VECTOR, IPL1) CNIntHandler(void)
{
    char str[10];
	mCNClearIntFlag(); // Macro function--Clear IF
	LATBSET=LEDB;
	sw_msDelay(ISRWait); //Wait for de-bounce
    
    //Read BTN1 and BTN2 in PortG
    unsigned int buttons=mPORTGReadBits(BTN1 | BTN2);
    
    //decode buttons
    switch(buttons)
	{
		default:  //40%
            DC=40;
			break;
			
		case BTN1:   //65%
            DC=65;
			break;
			
		case BTN2:   //80%
            DC=80;
			break;
			
		case (BTN1 | BTN2):   //95%
            DC=95;
			break;			
	}
    clearLCD();
    sprintf(str,"PWM=%2d%%\n",DC);
    LCD_puts(str);
    pwm_set(DC);
	mCNClearIntFlag(); // Macro function--Clear IF
	LATBCLR=LEDB;
}


//T2 ISR
void __ISR(_TIMER_2_VECTOR, IPL2) Timer2Handler(void)
{
	mT2ClearIntFlag(); // Clear the interrupt flag
 	LATBINV=LEDA;      //FLICKER LEDA
    mT2ClearIntFlag();
}

int pwm_set(int dutyCycle)
{
    if ((dutyCycle<0) | (dutyCycle>100))
    {
        return 1;
    }
    int OC3RS=(dutyCycle*(PR+1)/100);
    SetDCOC3PWM(OC3RS);
    return 0;
} 

int pwm_init(int dutyCycle, int cycleFrequency)
{
    Cerebot_mx7cK_setup();
    initBits();
    

    if ((dutyCycle<0) | (dutyCycle>100))
    {
        return 1;
    }
    
    int OC3R;
    OC3R=(dutyCycle*(PR+1)/100);
    PR=(FPB/cycleFrequency)-1;
    mOC3ClearIntFlag(); // Clear output compare interrupt flag (not using this interrupt)
    //Timer and Interrupt Initialization
	OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, PR);
    OpenOC3(OC_configure_bits, OC3R,OC3R );
    // Setup processor board
    ISRINIT();
    PMP_init();
    LCD_init();
    SetDCOC3PWM(OC3R);
    return 0;
}

void initBits()
{
    //I/O Bits Cleared and Set
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));// INPUT TO BUTTONS
    PORTSetPinsDigitalOut(IOPORT_D,BIT_7); //OUTPUT TO DC MOTOR
    PORTSetPinsDigitalOut(IOPORT_B,LEDS);
    LATBCLR; 
    
    LATDCLR = BIT_7;/* Clear values */
}


void ISRINIT()
{
    //T2 INT Priorities:
    mT2SetIntPriority(2); // Group priority range: 1 to 7
    mT2SetIntSubPriority(0); // Subgroup priority range: 0 to 3
    
    //CN INT Priorities: 
	mCNOpen(CN_ON,(CN8_ENABLE | CN9_ENABLE), 0); //CN Interrupt every time BTN pressed
	mCNSetIntPriority(1); // CN priority (1 to 7)
	mCNSetIntSubPriority(0); // CN Subgroup priority (0 to 3)
    
    //Clear port and flags
	unsigned int dummy = PORTReadBits(IOPORT_G, BTN1 | BTN2); //read port to clear differences
	mT2ClearIntFlag();//CLEAR T1 INTERUPT FLAG TIMER1
    mCNClearIntFlag();//CLEAR CN INTERUPT FLAG TIMER1
    
    //Enable CN and T2 INT
	INTEnableSystemMultiVectoredInt();
    mT2IntEnable(1);
	mCNIntEnable(1); // Enable CN interrupts
}

