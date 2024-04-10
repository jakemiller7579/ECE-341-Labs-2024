/*
1. Using the macro “mT1GetIntFlag()”, wait until the Timer 1 interrupt flag is set.
2. Clear the Timer 1 interrupt flag using the macro “mT1ClearIntFlag()”. 
3. Toggle LEDA.
4. Decrement the software variable used to count the number of milliseconds delay for 
the stepper motor step operation.
5. Decrement the software variable used to count the number of milliseconds delay for 
timing the delay between successive samples of the button status. (100 ms)
6. Return (by reference) the values of the counters to the task manager, main().
*/


#include <plib.h>
#include "CerebotMX7cK.h"
#include "Project4.h"

int main()
{
	unsigned int button_delay, step_delay, flagdelay, buttons, delay, dir, mode;
    
	button_delay=btndelay;
	flagdelay=1;
	step_delay=20;
	system_init();
	
	while(1){
		if (button_delay ==0){
			read_buttons();
			decode_buttons(buttons, &delay, &dir, &mode);
			LATBINV=LEDB;
			button_delay = btndelay;
		}
		if (step_delay==0){
			sw_fsm();
			output_sm_code();
			LATBINV=LEDC;
			step_delay = delay;
		}
			Timer1_delay(flagdelay,&button_delay, &step_delay);
	}
		
}

void Timer1_delay(unsigned int flagdelay,unsigned int &button_delay,unsigned int &step_delay)
{
	while(flagdelay--)
	{
		while(!mT1GetIntFlag()); // Wait for interrupt flag to be set
			mT1ClearIntFlag(); // Clear the interrupt flag
 			LATBINV=LEDA;
			*button_delay--;
			*step_delay--;
			

 }
}



void system_init(void)
{
    // Setup processor board
    Cerebot_mx7cK_setup();
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));
    PORTSetPinsDigitalOut(IOPORT_B,(BIT_2 | BIT_3 | BIT_4 | BIT_7 | BIT_8 | BIT_9 | BIT_10));
    LATBCLR = (BIT_2 | BIT_3 | BIT_4 | BIT_7 | BIT_8 | BIT_9 | BIT_10);/* Turn off */
    mT1ClearIntFlag();//CLEAR INTERUPT FLAG TIMER1
    OpenTimer1(T1_ON | T1_PS_1_1, (T1_TICK-1));
}

unsigned int read_buttons(void)
{
	//Use BTN1 and BTN2 in PortG
    unsigned int buttons=mPORTGReadBits(BTN1 | BTN2);
    return buttons;
}


void decode_buttons(unsigned int buttons, unsigned int *step_delay, unsigned int *dir, unsigned int *mode)
{
	//Use PORTB
	//LEDA-LEDD, Pin 1-4
	//Winding 2b-1a, 7-10 (LEDE-LEDH)

	switch(buttons)
	{
		default: //case (~(BTN1 | BTN2)):   //Clockwise, Fullstep
			*dir=CW;
			*mode=HS;
			*step_delay=RPM15;
			break;
			
		case BTN1:   //Clockwise, Halfstep
			*dir=CW; 
			*mode=HS;
			*step_delay=HSDelay;
			break;
			
		case BTN2:   //Counter-clockwise, HalfStep
			*dir=CCW; 
			*mode=HS;
			*step_delay=HSDelay;
			break;
			
		case (BTN1 + BTN2):   //Counter-clockwise, Fullstep
			*dir=CCW; 
			*mode=FS;
			*step_delay=FSDelay;
			break;			
	}
}



unsigned int sw_fsm(unsigned int dir, unsigned int mode)
{
	enum {S0_5=0, S1, S1_5, S2, S2_5, S3, S3_5,S4};
	//
	static unsigned int pstate;
	const unsigned int sm_code[] = {0x0A, 0x08, 0x09, 0x01, 0x05, 0x04, 0x06, 0x02};
	
	switch(pstate)
	{
		case S0_5:
			if (dir== CW)
			{
				if(mode==HS)
				{
					pstate=S1; //HS CW
				}
				else
				{
					pstate=S1_5; //FS CW
				}
			}
			
			else
			{
				if(mode==HS)
				{
					pstate= S4; //HS CCW
				}
				else
				{
					pstate= S3_5; //FS CCW
				}
			}
			break;
            
        case S1:
            if (dir==CW)
            {
               if (mode==HS)
               {
                   pstate=S1_5;
               }
               else
               {
                   pstate=S2;
               }
            }
            else
            {
                if (mode==HS)
                {
                    pstate=S0_5;
                }
                else
                {
                    pstate=S4;
                }
            }
         
            
        case S1_5:
            if (dir==CW)
            {
               if (mode==HS)
               {
                   pstate=S2;
               }
               else
               {
                   pstate=S2_5;
               }
            }
            else
            {
                if (mode==HS)
                {
                    pstate=S1;
                }
                else
                {
                    pstate=S0_5;
                }
            }            
            
 
        case S2:
            if (dir==CW)
            {
               if (mode==HS)
               {
                   pstate=S2_5;
               }
               else
               {
                   pstate=S3;
               }
            }
            else
            {
                if (mode==HS)
                {
                    pstate=S1_5;
                }
                else
                {
                    pstate=S1;
                }
            }            
            
           case S2_5:
            if (dir==CW)
            {
               if (mode==HS)
               {
                   pstate=S3;
               }
               else
               {
                   pstate=S3_5;
               }
            }
            else
            {
                if (mode==HS)
                {
                    pstate=S2;
                }
                else
                {
                    pstate=S1_5;
                }
            }         
            
            
        case S3:
            if (dir==CW)
            {
               if (mode==HS)
               {
                   pstate=S3_5;
               }
               else
               {
                   pstate=S4;
               }
            }
            else
            {
                if (mode==HS)
                {
                    pstate=S2_5;
                }
                else
                {
                    pstate=S2;
                }
            }            
            
        case S3_5:
            if (dir==CW)
            {
               if (mode==HS)
               {
                   pstate=S4;
               }
               else
               {
                   pstate=S0_5;
               }
            }
            else
            {
                if (mode==HS)
                {
                    pstate=S3;
                }
                else
                {
                    pstate=S2_5;
                }
            }            
            
        case S4:
            if (dir==CW)
            {
               if (mode==HS)
               {
                   pstate=S0_5;
               }
               else
               {
                   pstate=S1;
               }
            }
            else
            {
                if (mode==HS)
                {
                    pstate=S3_5;
                }
                else
                {
                    pstate=S3;
                }
            }                       
        }	
	return sm_code[pstate];
}

void output_sm_code(unsigned int sm_code)
{
    unsigned int temp;
    sm_code=sm_code<<7;
    temp=PORTB;
    temp=temp & (~SM_COILS);
    temp=temp | sm_code;
    LATB=temp;
}