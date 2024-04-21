#include <plib.h>
#include "PWM.h"
#include "LCDLIB.h"
#include "CerebotMX7cK.h"
#include <stdio.h>
#include <string.h>

#define MTR_SA BIT_9
#define MTR_SB BIT_10
void IC_init();

//global variables
int PR; 
int DC=40;

float RPS=0; //IC global var (rev per second))

int main() 
{
    int freq=1000;
    pwm_init(DC,freq);
    char rpsstr[16];
    while(1) 
    {
        sw_msDelay(100);
        mCNIntEnable(0); //disable CN interrupts
        clearLineLCD(2); //clears 2nd line
        sprintf(rpsstr,"RPS=%.2f",RPS);        
        LCD_puts(rpsstr);
        mCNIntEnable(1); // Enable CN interrupts
    }
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

int pwm_set(int dutyCycle)
{
    if ((dutyCycle<0) | (dutyCycle>100))
    {
        return 1;
    }
    OC3RS=(dutyCycle*(PR+1)/100);
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
     
//    int OC3R;
    PR=(FPB/cycleFrequency)-1;
    OC3RS=(dutyCycle*(PR+1)/100);
    mOC3ClearIntFlag(); // Clear output compare interrupt flag (not using this interrupt)
    //Timer and Interrupt Initialization
	OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, PR);
    OpenOC3(OC_configure_bits, OC3RS,OC3RS );
    // Setup processor board
    ISRINIT();
    PMP_init(); 
    LCD_init(); 
    T3_init();
    IC_init();
    
    pwm_set(dutyCycle);
    LCD_puts("PWM=40%\n");
    return 0;
}
 
void initBits()
{
    //I/O Bits Cleared and Set
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));// INPUT TO BUTTONS
    PORTSetPinsDigitalOut(IOPORT_D,BIT_7); //OUTPUT TO DC MOTOR
    PORTSetPinsDigitalOut(IOPORT_B,LEDS);
    LATBCLR=LEDS; 
    LATDCLR = BIT_7;/* Clear values */
}


void ISRINIT()
{
    //T2 INT Priorities:
    mT2SetIntPriority(2); // Group priority range: 1 to 7
    mT2SetIntSubPriority(1); // Subgroup priority range: 0 to 3
    
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

void T3_init()
{ //Timer 3 
    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_256, 0xFFFF);
    mT3SetIntPriority(2); // Group priority range: 1 to 7
    mT3SetIntSubPriority(2); // Subgroup priority range: 0 to 3
    mT3IntEnable(1); //enable t3
}

//Timer 3 ISR
void __ISR(_TIMER_3_VECTOR, IPL2) Timer3Handler(void)
{
    LATBINV=LEDC;
    mT3ClearIntFlag();
}

//T2 ISR
void __ISR(_TIMER_2_VECTOR, IPL2) Timer2Handler(void)
{

 	LATBINV=LEDA;      //FLICKER LEDA
    mT2ClearIntFlag(); // Clear the interrupt flag
}

//Input Capture
void IC_init()
{
    PORTSetPinsDigitalIn(IOPORT_D, (MTR_SA | MTR_SB));
    mIC5ClearIntFlag();
    unsigned int c1, c2, c3, c4, c5, c6, c7, ic1, ic2, ic3;
    c1=IC_ON; //Enable input capture
    c2=IC_CAP_16BIT; //Capture 16 bit timer count
    c3=IC_IDLE_STOP; //Stop input capture during debug
    c4=IC_FEDGE_FALL; //Initial capture on falling edge
    c5=IC_TIMER3_SRC; //Use Timer 3 as time to capture
    c6=IC_INT_1CAPTURE; //Generate interrupt on each capture
    c7= IC_EVERY_FALL_EDGE; //capture time on each falling edge
    OpenCapture5(c1 |c2 | c3 | c4 | c5 | c6 | c7);
    
    ic1=IC_INT_ON;//Enable IC interrupt
    ic2=IC_INT_PRIOR_3; //priority lvl 3
    ic3=IC_INT_SUB_PRIOR_0; //sub priority lvl 0
    ConfigIntCapture5(ic1 | ic2 |ic3);
}

void __ISR(_INPUT_CAPTURE_5_VECTOR, IPL3) Capture5(void)
{

    static unsigned short t_old=0;
    unsigned int fifo_vals[4];
    unsigned short t_new, tdiff;
    t_new=fifo_vals[0];
    tdiff=t_new-t_old;
    t_old=t_new;
    float ftimer=39062.5;// Hz --New Freq (10MHz/256)
    //calculate speed using moving average
    static unsigned short tdiffarray[16];
    static unsigned int counter=0;
    LATBINV=LEDD;
    ReadCapture5(fifo_vals);
    tdiffarray[counter]=tdiff;
    counter++;
    if (counter%16==0)
    {
        counter=0;
        unsigned int i;
        unsigned short calc=0;
        for (i=0; i<=16; i++)
        {
            calc=calc+tdiffarray[i];
        }
        RPS=ftimer/(calc/16);
    }
    mIC5ClearIntFlag(); //could also use INTClearFlag(INT_IC5);
    //INTClearFlag(INT_IC5);
}