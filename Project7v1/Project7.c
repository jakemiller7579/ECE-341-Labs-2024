#include "LCDLIB.h"
#include <stdio.h>
#include <string.h>
#include "comm.h"
#include "Project7.h"
#include <plib.h>

#define baud 19200
  void bit_init();
  void T1_CN_init();
 unsigned int step_delay;
unsigned int dir;
unsigned int mode;
unsigned int delay=RPM15HS;
unsigned int LCDTimer=1000;

int main()
{
    dir=CW;
	mode=HS;
	step_delay=RPM15HS;
	system_init();
    char str[21]; // Buffer size is set to hold the maximum expected
        // number of characters per line plus the NULL terminator
    int speed; //motor speed characters
    char modestr[5], dirstr[5]; //mode and dir string 
    unsigned int readstr;
    while(1)
    {


        mCNIntEnable(0); // Disable CN interrupts 
        clearLCD();//clear LCD
        while(!getstrU1(str, sizeof(str)));
        sscanf(str,"%d %s %s", &speed, modestr, dirstr ); //user input   
        //critical section  
        LCD_puts(str); //write to LCD screen
                //critical section
 
        
        readstr=strcmp(dirstr,"CCW");
        if(readstr==0)
        {
           dir=CCW; 
        }
        else
        {
            dir=CW;
        }
        readstr=strcmp(modestr,"FULL");
        if (readstr== 0)
        {
            mode=FS;
        }
        else
        {
            mode=HS;
        }
        //from 0-30 RPM
        delay=60000/(speed*100*mode);
        mCNIntEnable(1); // Enable CN interrupts  
    }
}



void system_init()
{
	Cerebot_mx7cK_setup();
	PMP_init();
	LCD_init();
    initialize_uart1(baud, ODD_PARITY);
    bit_init();
    T1_CN_init();
}
//Project 5
void bit_init()
{
    //I/O Bits Cleared and Set
    PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));// INPUT TO BUTTONS
    PORTSetPinsDigitalOut(IOPORT_B,(LEDABC | SM_COILS)); //OUTPUT TO MOTOR AND LEDS
    LATBCLR = (LEDABC | SM_COILS);/* Clear values */
}

void T1_CN_init()
{
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
            writeLCD(0,0xC0);
            LCD_puts("CW HS 15 RPM\n"); //write to LCD screen
            printf("CW HS 15 RPM\n");
			break;
			
		case BTN1:   //CW, FS, 15RPM
			dir=CW; 
			mode=FS;
			delay=RPM15FS;
            writeLCD(0,0xC0);
            LCD_puts("CW FS 15 RPM\n"); //write to LCD screen
            printf("CW FS 15 RPM\n");            
			break;
			
		case BTN2:   //CCW, HS, 10RPM
			dir=CCW; 
			mode=HS;
			delay=RPM10HS;
             writeLCD(0,0xC0);
            LCD_puts("CCW HS 10 RPM\n"); //write to LCD screen
            printf("CCW HS 10 RPM\n");              
			break;
			
		case (BTN1 + BTN2):   //CCW, FS, 25 RPM
			dir=CCW; 
			mode=FS;
			delay=RPM25FS;
            writeLCD(0,0xC0);
            LCD_puts("CCW FS 25 RPM\n"); //write to LCD screen
            printf("CCW FS 25 RPM\n"); 
			break;			
	}
        if ((dir==CW) & (mode==HS) &  (delay==RPM15HS))
        {
          writeLCD(0,0xC0);
          LCD_puts("CW HS 15 RPM\n"); //write to LCD screen
          printf("CW HS 15 RPM\n");
        }
    
	mCNClearIntFlag(); // Macro function--Clear IF
	LATBCLR=LEDC;
}

 
//T1 ISR
void __ISR(_TIMER_1_VECTOR, IPL2) Timer1Handler(void)
{
    mT1ClearIntFlag(); // Clear the interrupt flag
    unsigned int temp=0;
//	while(!mT1GetIntFlag()); // Wait for interrupt flag to be set

 	LATBINV=LEDA;      //FLICKER LEDA
	//button_delay=button_delay-1;   //DECREMENT BTN DELAY AND RETURN TO MAIN
	step_delay--;    //DECREMENT STEP DELAY AND RETURN TO MAIN
    LCDTimer--;    
    if (step_delay==0){
		temp=sw_fsm(dir, mode);
		output_sm_code(temp);
		step_delay = delay;  
		}
    if(LCDTimer==0)
    {
        clearLCD();
        LCDTimer=1000;
    }
        mT1ClearIntFlag();
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
