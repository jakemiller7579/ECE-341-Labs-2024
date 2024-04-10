#include <plib.h>
#include "CerebotMX7cK.h"
#include "LCDLIB.h"
#include "I2C_EEPROM_LIB.h"


void compare(char *read, char *write,int len)
{
    int i;
    int testpass=0;
    
    for (i=0;i<len;i++)
    {
        if(write[i]==read[i])
        {
            testpass++;
        }

    }
    if (testpass==len)
    {
        PORTSetBits(IOPORT_G, LED1);
        LCD_puts("TEST PASSED\n");
    }
    else if (testpass!=len)
    {
        PORTSetBits(IOPORT_G, LED2);
        LCD_puts("TEST FAILED\n");
    }
}

int I2CWriteEEPROM(unsigned int SlaveAddress, unsigned int mem_addr, char *i2cData, int len)
{
    unsigned char write_err=0;
    unsigned int memMSB=(mem_addr) &~ (BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
    memMSB=memMSB>>8;
    unsigned int memLSB=(mem_addr) &~ (BIT_15 | BIT_14 | BIT_13 | BIT_12 | BIT_11 | BIT_10 | BIT_9 | BIT_8);
    StartI2C2(); //non-block --Start symbol
    IdleI2C2(); //block
    unsigned int controlbyte=((SlaveAddress <<1) | 0); //control byte
    write_err |= MasterWriteI2C2(controlbyte);
    write_err |= MasterWriteI2C2(memMSB);
    write_err |= MasterWriteI2C2(memLSB);
    
    int i=0;
    int numpages=mem_addr;
    while(1)
    {
        MasterWriteI2C2(*i2cData);
        i2cData++;
        len--; 
        numpages++;
        if ((numpages%64)==0) //is address divisible by 64?
        {
            write_err=0;
            //stop
            StopI2C2();
            IdleI2C2();  
            //wait
            wait_i2c_xfer(SlaveAddress); 

            StartI2C2(); //non-block --Start symbol
            IdleI2C2(); //block
            controlbyte=((SlaveAddress <<1) | 0);
            write_err |= MasterWriteI2C2((SlaveAddress <<1) | 0);
            write_err |= MasterWriteI2C2(((numpages) & (0xFF00))>>8);
            write_err |= MasterWriteI2C2((numpages) & (0x00FF));
//            if (write_err)
//            {
//                return 1;
//            }
        }

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


    }
}

int I2CReadEEPROM(unsigned int SlaveAddress, unsigned int mem_addr, char *i2cData, int len)
{
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
    RestartI2C2();
    IdleI2C2();
    controlbyte=((SlaveAddress <<1) | 1);
    int i=0;
    MasterWriteI2C2(controlbyte); //Read control byte
    while(1)
    {
        *i2cData=MasterReadI2C2();
        i2cData++; 
        len--;
        if (len>0)
       {
            AckI2C2();
            IdleI2C2();
        }
        else 
        {
            NotAckI2C2();
            IdleI2C2();
            StopI2C2();
            IdleI2C2();
            return 0;
        }
//        if ( (&i2cData<0x0000) | (&i2cData>0xFFFF) )
//        {
//            return 1; //invalid memory address
//        }
        if (*i2cData=='\0')
        {
            return 2; //null pointer
        }
        if(len<0)
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
        
        while(MasterWriteI2C2(controlbyte))//if(ack==0) //NACK
        {
            RestartI2C2();
            IdleI2C2();
        }
            StopI2C2();
            IdleI2C2();   
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