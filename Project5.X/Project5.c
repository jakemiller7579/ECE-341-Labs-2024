/*
1. Using the macro ?mT1GetIntFlag()?, wait until the Timer 1 interrupt flag is set.
2. Clear the Timer 1 interrupt flag using the macro ?mT1ClearIntFlag()?. 
3. Toggle LEDA.
4. Decrement the software variable used to count the number of milliseconds delay for 
the stepper motor step operation.
5. Decrement the software variable used to count the number of milliseconds delay for 
timing the delay between successive samples of the button status. (100 ms)
6. Return (by reference) the values of the counters to the task manager, main().
*/


#include <plib.h>
#include "Project5.h"

//global variables
unsigned int step_delay;
unsigned int dir;
unsigned int mode;

int main()
{
	unsigned int button_delay, step_delay, buttons, delay, dir, mode,temp;
    
	button_delay=btndelay;
    temp=0;
	delay,step_delay=RPM15HS;
	system_init();
	timer1_interrupt_initialize();
	
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
			*button_delay=*button_delay-1;   //DECREMENT BTN DELAY AND RETURN TO MAIN
			*step_delay=*step_delay-1;    //DECREMENT STEP DELAY AND RETURN TO MAIN
			

 //}
}



void change_notice_ISR(_CHANGE_NOTICE_VECTOR, IPL1) CNIntHandler(void)
{
	mCNClearIntFlag(); // Macro function--Clear IF
	LATBSET=LEDC;
	sw_msDelay(ISRWait); //Wait for debounce
	mCNClearIntFlag(); // Macro function--Clear IF
	
	//Read BTN1 and BTN2 in PortG
    unsigned int buttons=mPORTGReadBits(BTN1 | BTN2);
	
	//Decode buttons--Use Global Variables to return to main
   //step_delay goes to address delay
	switch(buttons)
	{
		default:  //CW, HS, 15RPM
			dir=CW;
			mode=HS;
			step_delay=RPM15HS;
			break;
			
		case BTN1:   //CW, FS, 15RPM
			dir=CW; 
			mode=FS;
			step_delay=RPM15FS;
			break;
			
		case BTN2:   //CCW, HS, 10RPM
			dir=CCW; 
			mode=HS;
			step_delay=RPM10HS;
			break;
			
		case (BTN1 + BTN2):   //CCW, FS, 25 RPM
			dir=CCW; 
			mode=FS;
			step_delay=RPM25FS;
			break;			
	}
	
	mCNClearIntFlag(); // Macro function--Clear IF
	LATBCLR=LEDC;
}

void sw_msDelay (unsigned int mS)
{
    int i;
    while(mS --) // SW Stop breakpoint
    {
        for (i = 0; i< COUNTS_PER_MS; i++) // 1 ms delay loop
    {
            // do nothing
    }
}
/* Use code from Listing 3 */
}


void system_init(void)
{
    // Setup processor board
    Cerebot_mx7cK_setup();
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));// INPUT TO BUTTONS
    PORTSetPinsDigitalOut(IOPORT_B,(LEDABC | SM_COILS)); //OUTPUT TO MOTOR AND LEDS
    LATBCLR = (LEDABC | SM_COILS);/* Clear values */
	
	//Timer and Interupt Initializatrion
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, T1_INTR_RATE-1);//Interupt every 1ms
	// Enable CN for BTN1 and BTN2	
	mCNOpen(CN_ON,(CN8_ENABLE | CN9_ENABLE), 0); //CN detection
	mCNSetIntPriority(2); // Group priority (1 to 7)
	mCNSetIntSubPriority(0); // Subgroup priority (0 to 3)
	unsigned int dummy = PORTReadBits(IOPORT_G, BTN1 | BTN2); //read port to clear differences
	mT1ClearIntFlag();//CLEAR INTERUPT FLAG TIMER1
	INTEnableSystemMultiVectoredInt();
	mCNIntEnable(1); // Enable CN interrupts
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
            break;
            
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
            break;
 
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
            break;
            
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
            break;
            
            
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
            break;
            
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
            break;
            
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
            break;
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