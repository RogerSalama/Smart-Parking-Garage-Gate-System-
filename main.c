#include <stdint.h>

/* * ===========================================================
 * TM4C123 Register Definitions (Direct Access)
 * ===========================================================
 * Using direct memory addresses to ensure the code compiles 
 * regardless of your specific IDE header configuration.
 */

// System Control Registers
#define SYSCTL_RCGCGPIO_R      (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R        (*((volatile uint32_t *)0x400FEA08))

// Port B Registers (Base: 0x40005000)
#define GPIO_PORTB_DATA_R      (*((volatile uint32_t *)0x400053FC))
#define GPIO_PORTB_DIR_R       (*((volatile uint32_t *)0x40005400))
#define GPIO_PORTB_AFSEL_R     (*((volatile uint32_t *)0x40005420))
#define GPIO_PORTB_PDR_R       (*((volatile uint32_t *)0x40005514))
#define GPIO_PORTB_DEN_R       (*((volatile uint32_t *)0x4000551C))
#define GPIO_PORTB_AMSEL_R     (*((volatile uint32_t *)0x40005528))
#define GPIO_PORTB_PCTL_R      (*((volatile uint32_t *)0x4000552C))

// Port D Registers (Base: 0x40007000)
#define GPIO_PORTD_DATA_R      (*((volatile uint32_t *)0x400073FC))
#define GPIO_PORTD_DIR_R       (*((volatile uint32_t *)0x40007400))
#define GPIO_PORTD_AFSEL_R     (*((volatile uint32_t *)0x40007420))
#define GPIO_PORTD_PDR_R       (*((volatile uint32_t *)0x40007514))
#define GPIO_PORTD_DEN_R       (*((volatile uint32_t *)0x4000751C))
#define GPIO_PORTD_LOCK_R      (*((volatile uint32_t *)0x40007520))
#define GPIO_PORTD_CR_R        (*((volatile uint32_t *)0x40007524))
#define GPIO_PORTD_AMSEL_R     (*((volatile uint32_t *)0x40007528))
#define GPIO_PORTD_PCTL_R      (*((volatile uint32_t *)0x4000752C))

// Port E Registers (Base: 0x40024000)
#define GPIO_PORTE_DATA_R      (*((volatile uint32_t *)0x400243FC))
#define GPIO_PORTE_DIR_R       (*((volatile uint32_t *)0x40024400))
#define GPIO_PORTE_AFSEL_R     (*((volatile uint32_t *)0x40024420))
#define GPIO_PORTE_PDR_R       (*((volatile uint32_t *)0x40024514))
#define GPIO_PORTE_DEN_R       (*((volatile uint32_t *)0x4002451C))
#define GPIO_PORTE_AMSEL_R     (*((volatile uint32_t *)0x40024528))
#define GPIO_PORTE_PCTL_R      (*((volatile uint32_t *)0x4002452C))

// Port F Registers (Base: 0x40025000)
#define GPIO_PORTF_DATA_R      (*((volatile uint32_t *)0x400253FC))
#define GPIO_PORTF_DIR_R       (*((volatile uint32_t *)0x40025400))
#define GPIO_PORTF_AFSEL_R     (*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_PUR_R       (*((volatile uint32_t *)0x40025510))
#define GPIO_PORTF_DEN_R       (*((volatile uint32_t *)0x4002551C))
#define GPIO_PORTF_AMSEL_R     (*((volatile uint32_t *)0x40025528))
#define GPIO_PORTF_PCTL_R      (*((volatile uint32_t *)0x4002552C))

/* ===========================================================
 * Configuration Masks
 * =========================================================== */
#define LED_RED      (1U << 1)
#define LED_BLUE     (1U << 2)
#define LED_GREEN    (1U << 3)
#define LED_MASK     (LED_RED | LED_BLUE | LED_GREEN)

#define BTN_PF4      (1U << 4)
#define BTN_PE0      (1U << 0)
#define BTN_PE1      (1U << 1)
#define BTN_PB0      (1U << 0)
#define BTN_PB1      (1U << 1)
#define BTN_PD0      (1U << 0)
#define BTN_PD1      (1U << 1)

#define RCGCGPIO_ALL ((1U<<1)|(1U<<3)|(1U<<4)|(1U<<5)) // Ports B, D, E, F

/* ===========================================================
 * Initialization & Logic
 * =========================================================== */

static void GPIO_Init(void)
{
    SYSCTL_RCGCGPIO_R |= RCGCGPIO_ALL;
    while ((SYSCTL_PRGPIO_R & RCGCGPIO_ALL) != RCGCGPIO_ALL) { }

    // Port F: LEDs (Output) and SW1 (Input)
    GPIO_PORTF_AMSEL_R &= ~(BTN_PF4 | LED_MASK);
    GPIO_PORTF_PCTL_R  &= ~0x000FFFF0U;
    GPIO_PORTF_AFSEL_R &= ~(BTN_PF4 | LED_MASK);
    GPIO_PORTF_DIR_R   |=  LED_MASK;
    GPIO_PORTF_DIR_R   &= ~BTN_PF4;
    GPIO_PORTF_PUR_R   |=  BTN_PF4;
    GPIO_PORTF_DEN_R   |=  BTN_PF4 | LED_MASK;
    GPIO_PORTF_DATA_R  &= ~LED_MASK;

    // Port E: PE0, PE1 (Inputs)
    GPIO_PORTE_AMSEL_R &= ~(BTN_PE0 | BTN_PE1);
    GPIO_PORTE_PCTL_R  &= ~0x000000FFU;
    GPIO_PORTE_AFSEL_R &= ~(BTN_PE0 | BTN_PE1);
    GPIO_PORTE_DIR_R   &= ~(BTN_PE0 | BTN_PE1);
    GPIO_PORTE_PDR_R   |=  (BTN_PE0 | BTN_PE1);
    GPIO_PORTE_DEN_R   |=  (BTN_PE0 | BTN_PE1);

    // Port B: PB0, PB1 (Inputs)
    GPIO_PORTB_AMSEL_R &= ~(BTN_PB0 | BTN_PB1);
    GPIO_PORTB_PCTL_R  &= ~0x000000FFU;
    GPIO_PORTB_AFSEL_R &= ~(BTN_PB0 | BTN_PB1);
    GPIO_PORTB_DIR_R   &= ~(BTN_PB0 | BTN_PB1);
    GPIO_PORTB_PDR_R   |=  (BTN_PB0 | BTN_PB1);
    GPIO_PORTB_DEN_R   |=  (BTN_PB0 | BTN_PB1);

    // Port D: PD0 (Locked), PD1 (Inputs)
    GPIO_PORTD_LOCK_R   = 0x4C4F434B; // Unlock Port D
    GPIO_PORTD_CR_R    |= (BTN_PD0 | BTN_PD1); 
    GPIO_PORTD_AMSEL_R &= ~(BTN_PD0 | BTN_PD1);
    GPIO_PORTD_PCTL_R  &= ~0x000000FFU;
    GPIO_PORTD_AFSEL_R &= ~(BTN_PD0 | BTN_PD1);
    GPIO_PORTD_DIR_R   &= ~(BTN_PD0 | BTN_PD1);
    GPIO_PORTD_PDR_R   |=  (BTN_PD0 | BTN_PD1);
    GPIO_PORTD_DEN_R   |=  (BTN_PD0 | BTN_PD1);
}

static inline uint32_t Btn_PF4(void) { return (GPIO_PORTF_DATA_R & BTN_PF4) == 0; }
static inline uint32_t Btn_PE0(void) { return (GPIO_PORTE_DATA_R & BTN_PE0) != 0; }
static inline uint32_t Btn_PE1(void) { return (GPIO_PORTE_DATA_R & BTN_PE1) != 0; }
static inline uint32_t Btn_PB0(void) { return (GPIO_PORTB_DATA_R & BTN_PB0) != 0; }
static inline uint32_t Btn_PB1(void) { return (GPIO_PORTB_DATA_R & BTN_PB1) != 0; }
static inline uint32_t Btn_PD0(void) { return (GPIO_PORTD_DATA_R & BTN_PD0) != 0; }
static inline uint32_t Btn_PD1(void) { return (GPIO_PORTD_DATA_R & BTN_PD1) != 0; }

static void LED_Set(uint32_t color_mask)
{
    GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~LED_MASK) | (color_mask & LED_MASK);
}

static void Delay_ms(uint32_t ms)
{
    volatile uint32_t i;
    while (ms--) { for (i = 0; i < 4000; i++) { } }
}



int main(void)
{
    static const uint32_t color_tbl[7] = {
        LED_RED,                                    /* PF4 */
        LED_BLUE,                                   /* PE0 */
        LED_GREEN,                                  /* PE1 */
        LED_RED  | LED_BLUE,                        /* PB0 */
        LED_RED  | LED_GREEN,                       /* PB1 */
        LED_BLUE | LED_GREEN,                       /* PD0 */
        LED_RED  | LED_BLUE | LED_GREEN,            /* PD1 */
    };

    uint32_t prev[7] = {0};
    uint32_t led_state = 0;

    GPIO_Init();
    LED_Set(led_state);

    while (1)
    {
        uint32_t cur[7];
        cur[0] = Btn_PF4();
        cur[1] = Btn_PE0();
        cur[2] = Btn_PE1();
        cur[3] = Btn_PB0();
        cur[4] = Btn_PB1();
        cur[5] = Btn_PD0();
        cur[6] = Btn_PD1();

        for (int i = 0; i < 7; i++)
        {
            if (cur[i] && !prev[i]) // Rising edge detection
            {
                led_state ^= color_tbl[i]; // XOR toggle
                LED_Set(led_state);
            }
            prev[i] = cur[i];
        }

        Delay_ms(20); // Debounce delay
    }
}