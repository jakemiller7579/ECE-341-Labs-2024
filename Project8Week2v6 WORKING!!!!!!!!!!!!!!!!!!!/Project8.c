#include <plib.h>
#include "CerebotMX7cK.h"
#include "LCDLIB.h"
#include "I2C_EEPROM_LIB.h"
#include "Project8.h"

void system_init()
{
    //initialize resources
    Cerebot_mx7cK_setup(); //setup
    bit_init();
    init_EEPROM(Fsck);
    PMP_init();
    LCD_init();
}

void bit_init()
{
    //I/O Bits Cleared and Set
   // PORTSetPinsDigitalIn(IOPORT_G,(BTN1 | BTN2));// INPUT TO BUTTONS
    PORTSetPinsDigitalOut(IOPORT_G,(BRD_LEDS)); 
    //LATGR = (BRD_LEDS);/* Clear values */
}

int main(void)
{
    int len=200;
    int SlaveAddress=0x50;
    char i2cDatain[len];
    char i2cDataout[len];
    unsigned int mem_addr=0x0000; 

    system_init(); 
    //create i2c frame
    int i;  
    for (i=0;i<len;i++)
    {
        i2cDatain[i]=i+1;    
    }
     
   I2CWriteEEPROM(SlaveAddress, mem_addr, i2cDatain,len );
   I2CReadEEPROM(SlaveAddress, mem_addr, i2cDataout, len);
   compare(i2cDataout,i2cDatain, len);
   // TestEEPROM();
    while(1){}
    return (1);
}