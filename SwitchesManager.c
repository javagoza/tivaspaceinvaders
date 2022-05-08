

// *************************** SwitchesManager ***************************
#define SWITCHES_PE1_PE0 (*((volatile unsigned long *)0x4002400C))
volatile unsigned long right;
volatile unsigned long left;

void PE1_PE0_Switches_Init(void) {
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE; // enable clock
    right = 0; // also delay to let clock to stabilize
    left = 0;
    GPIO_PORTE_DIR_R &= ~0x03; // PE(1-0) input low;
    GPIO_PORTE_AFSEL_R &= ~0x03; // PE(1-0) Alternate function disabled
    GPIO_PORTE_AMSEL_R &= ~0x03; // PE(1-0) Analog function disabled
    GPIO_PORTE_PDR_R &= ~0x03; // PE(1-0) Pulldown resistor disabled positive logic
    GPIO_PORTE_DEN_R |= 0x03; // PE(1-0) Digital enable
    GPIO_PORTE_PCTL_R &= ~0x000000FF; // PB(1-0) regular function

    GPIO_PORTE_IS_R &= ~0x03; // PE(1-0) is edge-sensitive
    GPIO_PORTE_IBE_R &= ~0x03; // PE(1-0) is not both edges
    GPIO_PORTE_IEV_R &= ~0x03; // PE(1-0) falling edge event
    GPIO_PORTE_ICR_R = 0x03; // clear flag PE(1-0)
    GPIO_PORTE_IM_R |= 0x03; // arm interrupt on PE(1-0)
    NVIC_PRI1_R = (NVIC_PRI1_R & 0xFFFFFF00) | 0x000000A0; // (g) priority 5
    NVIC_EN0_R |= 1 << 4; //  enable interrupt 29 in NVIC


}

void GPIOPortE_Handler(void) {
    if (GPIO_PORTE_RIS_R & 0x01) {
        GPIO_PORTE_ICR_R = 0x01;
        right = 1;
    }
    if (GPIO_PORTE_RIS_R & 0x02) {
        GPIO_PORTE_ICR_R = 0x02;
        left = 1;
    }
}

unsigned long Switches_In(void) {
    return SWITCHES_PE1_PE0 & 0x03;
}

void Clear_Switches(void) {
    right = 0;
    left = 0;
}

unsigned char isRightPressed(void) {
    if (right) {
        right = 0;
        return 1;
    } else {
        return 0;
    }
}

unsigned char isLeftPressed(void) {
    if (left) {
        left = 0;
        return 1;
    } else {
        return 0;
    }
}

