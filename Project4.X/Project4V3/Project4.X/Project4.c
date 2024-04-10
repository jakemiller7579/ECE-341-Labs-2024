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
#include "Project4.h"

int main()
{
	unsigned int button_delay, step_delay, flagdelay, buttons, delay, dir, mode,temp;
    
	button_delay=btndelay;
	flagdelay=1;
    temp=0;
	step_delay=RPM15HS;
	system_init();
	
	while(1){
		if (button_delay ==0){
			buttons=read_buttons();
			decode_buttons(buttons, &delay, &dir, &mode);
			LATBINV=LEDB;
			button_delay = btndelay;
		}
		if (step_delay==0){
			temp=sw_fsm(dir, mode);
			output_sm_code(temp);
			LATBINV=LEDC;
			step_delay = delay;  //
		}
			Timer1_delay(&button_delay, &step_delay);
	}
		
}

void Timer1_delay(unsigned int *button_delay,unsigned int *step_delay)
{
//	while(1--)
	//{
		while(!mT1GetIntFlag()); // Wait for interrupt flag to be set
			mT1ClearIntFlag(); // Clear the interrupt flag
 			LATBINV=LEDA;      //FLICKER LEDA
			*button_delay-=1;   //DECREMENT BTN DELAY AND RETURN TO MAIN
			*step_delay-=1;    //DECREMENT STEP DELAY AND RETURN TO MAIN
			

 //}
}



void system_init(void)
{
    // Setup processor board
    Cerebot_mx7cK_setup();
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));// INPUT TO BUTTONS
    PORTSetPinsDigitalOut(IOPORT_B,(LEDABC | SM_COILS)); //OUTPUT TO MOTOR AND LEDS
    LATBCLR = (LEDABC | SM_COILS);/* Clear values */
    mT1ClearIntFlag();//CLEAR INTERUPT FLAG TIMER1
    OpenTimer1(T1_ON | T1_PS_1_1, (T1_TICK-1)); //clocked at 10MHz, initialized period of 1ms
}

unsigned int read_buttons(void)
{
	//Use BTN1 and BTN2 in PortG
    unsigned int buttons=mPORTGReadBits(BTN1 | BTN2);
    return buttons;
}


void decode_buttons(unsigned int buttons, unsigned int *step_delay, unsigned int *dir, unsigned int *mode) 
{   
    //step_delay goes to address delay
	switch(buttons)
	{
		default:  //CW, HS, 15RPM
			*dir=CW;
			*mode=HS;
			*step_delay=RPM15HS;
			break;
			
		case BTN1:   //CW, FS, 15RPM
			*dir=CW; 
			*mode=FS;
			*step_delay=RPM15FS;
			break;
			
		case BTN2:   //CCW, HS, 10RPM
			*dir=CCW; 
			*mode=HS;
			*step_delay=RPM10HS;
			break;
			
		case (BTN1 + BTN2):   //CCW, FS, 25 RPM
			*dir=CCW; 
			*mode=FS;
			*step_delay=RPM25FS;
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