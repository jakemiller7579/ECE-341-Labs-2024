#include <plib.h>
#include "Project6.h"

//global variables
unsigned int step_delay;
unsigned int dir;
unsigned int mode;
unsigned int delay=RPM15HS;

int main()
{
    dir=CW;
	mode=HS;
	step_delay=RPM15HS;
	system_init();
	
	while(1){
	}
}

void system_init(void)
{
    // Setup processor board
    Cerebot_mx7cK_setup();
    
    //I/O Bits Cleared and Set
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));// INPUT TO BUTTONS
    PORTSetPinsDigitalOut(IOPORT_B,(LEDABC | SM_COILS)); //OUTPUT TO MOTOR AND LEDS
    LATBCLR = (LEDABC | SM_COILS);/* Clear values */
	
	//Timer and Interrupt Initialization
	OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, T1_TICK-1);//T1 Interrupt every 1ms	

    //T1 INT Priorities:
    mT1SetIntPriority(2); // Group priority range: 1 to 7
    mT1SetIntSubPriority(0); // Subgroup priority range: 0 to 3
    
    //CN INT Priorities: 
	mCNOpen(CN_ON,(CN8_ENABLE | CN9_ENABLE), 0); //CN Interrupt every time BTN pressed
	mCNSetIntPriority(1); // CN priority (1 to 7)
	mCNSetIntSubPriority(0); // CN Subgroup priority (0 to 3)
    
    //Clear port and flags
	unsigned int dummy = PORTReadBits(IOPORT_G, BTN1 | BTN2); //read port to clear differences
	mT1ClearIntFlag();//CLEAR T1 INTERUPT FLAG TIMER1
    mCNClearIntFlag();//CLEAR CN INTERUPT FLAG TIMER1
    
    //Enable CN and T1 INT
	INTEnableSystemMultiVectoredInt();
	mCNIntEnable(1); // Enable CN interrupts
    mT1IntEnable(1); // Enable T1 interrupts
    
    LCD_init();
}

void LCD_init(void)
{
    //Initialize LCD
    int cfg1 = PMP_ON|PMP_READ_WRITE_EN|PMP_READ_POL_HI|PMP_WRITE_POL_HI;
    int cfg2 = PMP_DATA_BUS_8 | PMP_MODE_MASTER1 |
    PMP_WAIT_BEG_1 | PMP_WAIT_MID_2 | PMP_WAIT_END_1;
    int cfg3 = PMP_PEN_0; // only PMA0 enabled
    int cfg4 = PMP_INT_OFF; // no interrupts used
    mPMPOpen(cfg1, cfg2, cfg3, cfg4);

    //Delay 20ms==>call sw_delay
    //LCD write 0x38, RS=0
    //delay 37ms
    //LCD write 0x0f, RS=0
    //delay 37ms
    //LCD write 0x01, RS=0
    //delay 1.52 ms ???
    
}

//CN ISR:
void __ISR(_CHANGE_NOTICE_VECTOR, IPL1) CNIntHandler(void)
{
	mCNClearIntFlag(); // Macro function--Clear IF
	LATBSET=LEDC;
	sw_msDelay(ISRWait); //Wait for de-bounce
	//mCNClearIntFlag(); // Macro function--Clear IF
	
	//Read BTN1 and BTN2 in PortG
    unsigned int buttons=mPORTGReadBits(BTN1 | BTN2);
	
	//Decode buttons--Use Global Variables to return to main
   //step_delay goes to address delay
	switch(buttons)
	{
		default:  //CW, HS, 15RPM
			dir=CW;
			mode=HS;
			delay=RPM15HS;
			break;
			
		case BTN1:   //CW, FS, 15RPM
			dir=CW; 
			mode=FS;
			delay=RPM15FS;
			break;
			
		case BTN2:   //CCW, HS, 10RPM
			dir=CCW; 
			mode=HS;
			delay=RPM10HS;
			break;
			
		case (BTN1 + BTN2):   //CCW, FS, 25 RPM
			dir=CCW; 
			mode=FS;
			delay=RPM25FS;
			break;			
	}
	mCNClearIntFlag(); // Macro function--Clear IF
	LATBCLR=LEDC;
}

 
//T1 ISR
void __ISR(_TIMER_1_VECTOR, IPL2) Timer1Handler(void)
{
    unsigned int temp=0;
//	while(!mT1GetIntFlag()); // Wait for interrupt flag to be set
	mT1ClearIntFlag(); // Clear the interrupt flag
 	LATBINV=LEDA;      //FLICKER LEDA
	//button_delay=button_delay-1;   //DECREMENT BTN DELAY AND RETURN TO MAIN
	step_delay--;    //DECREMENT STEP DELAY AND RETURN TO MAIN
        
    if (step_delay==0){
		temp=sw_fsm(dir, mode);
		output_sm_code(temp);
		step_delay = delay;  //
		}
        mT1ClearIntFlag();
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
// Use code from Listing 3
}




unsigned int sw_fsm(unsigned int dir, unsigned int mode)
{
    LATBINV=LEDB;
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

