#include <plib.h>
#include "CerebotMX7cK.h"
#include "LCDLIB.h"
#include "I2C_EEPROM_LIB.h"
extern unsigned char read;
extern unsigned char write;
void bit_init();

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
    LATGCLR = (BRD_LEDS);/* Clear values */
}

int main(void)
{
    int SlaveAddress=0x50;
    char i2cData[200];
    int mem_addr=0x003F;
    int len=2;
    system_init();
    //create i2c frame
    int i; 
    for (i=0;i<len;i++)
    {
        i2cData[i]=i; 
    }
     
   I2CWriteEEPROM(SlaveAddress, mem_addr, i2cData,len );
   I2CReadEEPROM(SlaveAddress, mem_addr, i2cData, len);
   // TestEEPROM();
    while(1){}
    return (1);
}