//LCDLIB.c
#include <plib.h>
#include "LCDLIB.h"
#include "CerebotMX7cK.h"

void writeLCD(unsigned int addr, char c)
{
    while(busyLCD());
    PMPSetAddress(addr); 
    PMPMasterWrite(c);
}

void clearLCD()
{
    writeLCD(0,0x01);//clear display
    writeLCD(0,0x02);//return home
}

char readLCD(unsigned int addr)
{
    PMPSetAddress(addr); //set LCD RS control
    mPMPMasterReadByte();//dummy read
    return mPMPMasterReadByte();//read data
}

unsigned int busyLCD()
{
    unsigned int dummy = readLCD(0);
    if(dummy >= BIT_7)
    {
        return 1; //busy 
    }
    else
    {
        return 0; //not busy
    }
}


void LCD_putc(char c)
{
    while(busyLCD());
    if(busyLCD()==0)//not busy
    {
        unsigned int a=readLCD(0);
      
        if ((c=='\n')|(c=='\r')) //if control character
        {
            //proceed on
            //\n is newline
            //\r is return to beginning of current line
        }
        
        else //if no control characters
        {
            if ((a > 0x0f) & (a < 0x40)) //if off to the right on L1, but off to left on L2
            {    
                writeLCD(0,0xC0); //move address to start of next line 
            }
            else if (a > 0x4f) //If off to the right of L2
            {
                writeLCD(0,0x80); //move address to top line              
            }
            writeLCD(1,c); //write character to LCD
        }
    }
}

void LCD_puts(char *char_string) 
{
    while(*char_string) //looks for NULL character
    {
        LCD_putc(*char_string); //write character to LCD
        char_string++; //increment address
    }
}

void system_init()
{
	Cerebot_mx7cK_setup();
	PMP_init();
	LCD_init();
}

void PMP_init()
{
	int config1= PMP_ON | PMP_READ_WRITE_EN | PMP_READ_POL_HI | PMP_WRITE_POL_HI;
	int config2=PMP_DATA_BUS_8 | PMP_MODE_MASTER1 |
                PMP_WAIT_BEG_4 | PMP_WAIT_MID_15 | PMP_WAIT_END_4; //replaced wait time with 4,15,4
	int config3=PMP_PEN_0; //only PMA0 enabled
	int config4= PMP_INT_OFF; //no interrupts used
	mPMPOpen(config1, config2, config3, config4);
}

void LCD_init()
{
	//LCD Initialize
	//Power On
	sw_msDelay(delay); //wait 
	//Set Function
	PMPSetAddress(0);
	PMPMasterWrite(0x38);
	//
	sw_msDelay(delay);
	//Set Display
	PMPSetAddress(0);
	PMPMasterWrite(0x0f);
	sw_msDelay(delay);
	//Clear Display
	PMPSetAddress(0);
	PMPMasterWrite(0x01);
	sw_msDelay(5);
	//return
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