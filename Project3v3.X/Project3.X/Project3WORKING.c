//include statements
#include <plib.h>
#include "CerebotMX7cK.h"
#include "Project3.h"

int main(void){
	
	unsigned int buttons, dir, mode, sm_code, step_delay; 
	//buttons=>number that is read 
	//dir=>clockwise (CW) or counterclockwise (CCW)
	//mode=> Full step , half step 
	//sm_code=>stepper motor code (return from FSM)
	//step_delay=>delay of the motor
	
	system_init();  //initialize cerebot board%

	while(1)
	{
        LATBINV=LEDA;
        LATBINV=LEDB;
		buttons=read_buttons(); //reads buttons and returns the number associated with buttons
		decode_buttons(buttons, &step_delay, &dir, &mode); //takes this number and decodes it and returns direction, mode, and step_delay
		sm_code=sw_fsm(dir, mode); //takes the direction and mode and puts it in a FSM. This returns the next state. 
		output_sm_code(sm_code); //takes the next state and outputs what is needed
        LATBINV=LEDA; //MEASURE DELAY NOT PART OF DELAY FUNCTION
		sw_msDelay(step_delay); //software delay

	}
	return(1);
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
			*mode=FS;
			*step_delay=FSDelay;
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

//step control=>State, hex code
//winding voltage=>binary part

//            1a, 1b, 2a, 2b
//S0_5=>0x0A=>1010 
//S1=>0x08=>1000 
//S1_5=>0x09=>1001
//S2=>0x01=>0001
//S2_5=>0x05=>0101
//S3=>0x04=>0100
//S3_5=>0x06=>0110
//S4=>0x02=>0010

unsigned int sw_fsm(unsigned int dir, unsigned int mode)
{
	enum {S0_5=0, S1, S1_5, S2, S2_5, S3, S3_5,S4};
	//
	static unsigned int pstate;
	const unsigned int sm_code[] = {0x0A, 0x08, 0x09, 0x01, 0x05, 0x04, 0x06, 0x02};
	
	switch(pstate)
	{
		case S0_5:
        {
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
        } 
        case S1:
        {    
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
            break;
        }
            
        case S1_5:
        {
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
            break;
        }
 
        case S2:
        {
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
            break;
        }
            
           case S2_5:
           {
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
            break;
           }
            
            
        case S3:
        {
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
            break;
        }
            
        case S3_5:
        {
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
            break;
        }
            
        case S4:
        {
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
            break;
        }
    }
	return sm_code[pstate];
}

void system_init(void)
{
    // Setup processor board
    Cerebot_mx7cK_setup();
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));
    PORTSetPinsDigitalOut(IOPORT_B, SM_LEDS);
    LATBCLR = SM_LEDS; /* Turn off LEDA through LEDH */
}

void output_sm_code(unsigned int sm_code)
{
    unsigned int temp;
    sm_code=sm_code<<7; //shift left 7
    temp=PORTB; //read values into temp
    temp=temp & (~SM_COILS); //clear bits 7-10
    temp=temp | sm_code; //changed data
    LATB=temp; //wrote data
}

void sw_msDelay(unsigned int step_delay)
{
    int i;
    while(step_delay --) // SW Stop breakpoint
    {
        for (i = 0; i< COUNTS_PER_MS; i++) // 1 ms delay loop
        {
            // do nothing
        }
   // LATBINV = LEDA;
    }
}