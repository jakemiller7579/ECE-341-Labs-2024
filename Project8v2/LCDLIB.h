//LCD.h

void system_init();
void PMP_init();
void LCD_init();
void LCD_putc(char c);
char readLCD(unsigned int addr);
unsigned int busyLCD();
void writeLCD(unsigned int addr, char c);
void LCD_puts(char *char_string);
void sw_msDelay (unsigned int mS);
void clearLCD();


#define delayLCD 50

//Software Delay 
#define COUNTS_PER_MS 8891