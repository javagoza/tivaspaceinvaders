
// *************************** LedsManager ***************************

#define LEDS_PB5_PB4 (*((volatile unsigned long *)0x400050C0))

#define PB_OUTPUT_BITS 0x30  // bits  5-4

void PB5_PB4_Leds_Init(void) {
    volatile unsigned long delay;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // Enable GPIOB
    delay = SYSCTL_RCGC2_R; // force wait for GPIOB 
    GPIO_PORTB_DEN_R |= PB_OUTPUT_BITS; // PORTB DEN bits (5-4) high
    GPIO_PORTB_DIR_R |= PB_OUTPUT_BITS; // PORTB DIR bits (5-4) high
    GPIO_PORTB_AFSEL_R &= ~PB_OUTPUT_BITS; // PORTB AFSEL bits (5-4) low
    GPIO_PORTB_AMSEL_R &= ~PB_OUTPUT_BITS; // PORTB AMSEL bits (5-4) low
    GPIO_PORTB_PUR_R &= ~PB_OUTPUT_BITS; // PORTB PUR bits (5-4) low
    GPIO_PORTB_PCTL_R &= ~0xFF0000; // PORTB PCTL bits (23-15) are low
}

void setLed0On(void) {
    LEDS_PB5_PB4 |= 0x10;
}

void setLed0Off(void) {
    LEDS_PB5_PB4 &= ~0x10;
}

void setLed1On(void) {
    LEDS_PB5_PB4 |= 0x20;
}

void setLed1Off(void) {
    LEDS_PB5_PB4 &= ~0x20;
}

