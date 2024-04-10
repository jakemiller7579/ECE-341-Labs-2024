#define OC_configure_bits (OC_ON | OC_TIMER_MODE16 | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE)
#define LEDS (LEDA | LEDB | LEDC | LEDD) 
#define ISRWait 20

//Software Delay 
#define COUNTS_PER_MS 8891

int pwm_set(int dutyCycle);
int pwm_init(int dutyCycle, int cycleFrequency);
void initBits();
void ISRINIT();
void sw_msDelay (unsigned int mS);
