#include <plib.h>
#include "CerebotMX7cK.h"
#include "LCDLIB.h"
#include "I2C_EEPROM_LIB.h"

int I2CWriteEEPROM(int SlaveAddress, unsigned int mem_addr, char *i2cData, int len)
{
    unsigned int memMSB=(mem_addr) &~ (BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
    memMSB=memMSB>>8;
    unsigned int memLSB=(mem_addr) &~ (BIT_15 | BIT_14 | BIT_13 | BIT_12 | BIT_11 | BIT_10 | BIT_9 | BIT_8);
    //int memMSB = 0x12;
    //int memLSB = 0x34;
    StartI2C2(); //non-block --Start symbol
    IdleI2C2(); //block
    unsigned int controlbyte=((SlaveAddress <<1) | 0); //control byte
    MasterWriteI2C2(controlbyte);
    MasterWriteI2C2(memMSB);
    MasterWriteI2C2(memLSB);
    unsigned int newpage=mem_addr%64; //sees if divisible by 64

    while(1)
    {
        MasterWriteI2C2(*i2cData);
        i2cData++;
        len--;
        mem_addr++;
        if (len==0)
        {
            //stop
            StopI2C2();
            IdleI2C2();   
            //wait
            wait_i2c_xfer(SlaveAddress);
            //stop
            StopI2C2();
            IdleI2C2();        
            return 0; //exits function
        }
        if (newpage==0) //is address divisible by 64?
        {
            //stop
            StopI2C2();
            IdleI2C2();  
            
            //wait
            MasterWriteI2C2(mem_addr);
            
        }
    }
}

int I2CReadEEPROM(int SlaveAddress, int mem_addr, char *i2cData, int len)
{
    unsigned int newpage=mem_addr%64; //sees if divisible by 64
    //clear values
    unsigned int memMSB=mem_addr & ~((BIT_7 |BIT_6 |BIT_5 |BIT_4 |BIT_3 | BIT_2 | BIT_1 | BIT_0));
    memMSB=memMSB>>8; //shift new values right
    //clear values
    unsigned int memLSB=mem_addr & ~((BIT_15 |BIT_14 |BIT_13 |BIT_12 |BIT_11 | BIT_10 | BIT_9 | BIT_8));
    unsigned int controlbyte=((SlaveAddress <<1) | 0); //control byte
    StartI2C2(); //non-block --Start symbol
    IdleI2C2(); //block

    MasterWriteI2C2(controlbyte);
    MasterWriteI2C2(memMSB);
    MasterWriteI2C2(memLSB);


    while(1)
    {
        MasterWriteI2C2(*i2cData);
        i2cData++;
        len--;
        mem_addr++;
        if (len==0)
        {
            //stop
            StopI2C2();
            IdleI2C2();   
            //wait
            wait_i2c_xfer(0x50);
            //stop
            StopI2C2();
            IdleI2C2();        
            return 0; //no error
        }
        if (!(0x0000<=(i2cData)<=0x7FFF))
        {
            return 1; //invalid memory address
        }
        if (*i2cData=='\0')
        {
            return 2; //null pointer
        }
        if( (len>200) | (len<0))
        {
            return 3; //invalid len arg
        }
        //I2C bus error 
    }
}
    

int wait_i2c_xfer(int SlaveAddress)
{
    unsigned int controlbyte=((SlaveAddress <<1) | 0);
    StartI2C2();
    IdleI2C2();
    while(1)
    {
        MasterWriteI2C2(controlbyte);
        if(I2CByteWasAcknowledged(0x50)==0)
        {
            return 0;
        }
        else
        {
            RestartI2C2();
            IdleI2C2();
        }
    }
}

char BusyI2C2(void)
{
    return(I2C2CONbits.SEN || I2C2CONbits.PEN || I2C2CONbits.RSEN ||
            I2C2CONbits.RCEN || I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);
}//return non-zero value if I2C controller is busy

void init_EEPROM(int SCK_FREQ)
{
    unsigned int BRG_VAL=((FPB/2/SCK_FREQ)-2); 
    OpenI2C2(I2C_EN, BRG_VAL); //enable i2c peripheral 
}