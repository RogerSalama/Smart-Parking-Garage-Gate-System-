#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <stdbool.h>
#include "gate_control.h"

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
	
/* Port B Interrupt Registers */
#define GPIO_PORTB_IS_R         (*((volatile uint32_t *)0x40005404))
#define GPIO_PORTB_IBE_R        (*((volatile uint32_t *)0x40005408))
#define GPIO_PORTB_IEV_R        (*((volatile uint32_t *)0x4000540C))
#define GPIO_PORTB_IM_R         (*((volatile uint32_t *)0x40005410))
#define GPIO_PORTB_RIS_R        (*((volatile uint32_t *)0x40005414))
#define GPIO_PORTB_ICR_R        (*((volatile uint32_t *)0x4000541C))

/* Port D Interrupt Registers */
#define GPIO_PORTD_IS_R         (*((volatile uint32_t *)0x40007404))
#define GPIO_PORTD_IBE_R        (*((volatile uint32_t *)0x40007408))
#define GPIO_PORTD_IEV_R        (*((volatile uint32_t *)0x4000740C))
#define GPIO_PORTD_IM_R         (*((volatile uint32_t *)0x40007410))
#define GPIO_PORTD_RIS_R        (*((volatile uint32_t *)0x40007414))
#define GPIO_PORTD_ICR_R        (*((volatile uint32_t *)0x4000741C))

/* Port E Interrupt Registers */
#define GPIO_PORTE_IS_R         (*((volatile uint32_t *)0x40024404))
#define GPIO_PORTE_IBE_R        (*((volatile uint32_t *)0x40024408))
#define GPIO_PORTE_IEV_R        (*((volatile uint32_t *)0x4002440C))
#define GPIO_PORTE_IM_R         (*((volatile uint32_t *)0x40024410))
#define GPIO_PORTE_RIS_R        (*((volatile uint32_t *)0x40024414))
#define GPIO_PORTE_ICR_R        (*((volatile uint32_t *)0x4002441C))

/* Port F Interrupt Registers */
#define GPIO_PORTF_IS_R         (*((volatile uint32_t *)0x40025404))
#define GPIO_PORTF_IBE_R        (*((volatile uint32_t *)0x40025408))
#define GPIO_PORTF_IEV_R        (*((volatile uint32_t *)0x4002540C))
#define GPIO_PORTF_IM_R         (*((volatile uint32_t *)0x40025410))
#define GPIO_PORTF_RIS_R        (*((volatile uint32_t *)0x40025414))
#define GPIO_PORTF_ICR_R        (*((volatile uint32_t *)0x4002541C))

/* NVIC Enable Register */
#define NVIC_EN0_R              (*((volatile uint32_t *)0xE000E100))

/* ===========================================================
 * Configuration Masks
 * =========================================================== */
#define LED_RED      (1U << 1)
#define LED_BLUE     (1U << 2)
#define LED_GREEN    (1U << 3)
#define LED_MASK     (LED_RED | LED_BLUE | LED_GREEN)

//#define BTN_PF4      (1U << 4)
//#define BTN_PE0      (1U << 0)
//#define BTN_PE1      (1U << 1)
//#define BTN_PB0      (1U << 0)
//#define BTN_PB1      (1U << 1)
//#define BTN_PD0      (1U << 0)
//#define BTN_PD1      (1U << 1)

#define BTN_PF4      (1U << 4) // Security Open (Moved to Port F)
#define BTN_PF0      (1U << 0) // Security Close (Moved to Port F)
#define BTN_PE0      (1U << 0) // Obstacle (Moved to Port E)
#define BTN_PE1      (1U << 1) // Unused or Other
#define BTN_PB0      (1U << 0) // Driver Open
#define BTN_PB1      (1U << 1) // Driver Close
#define BTN_PD0      (1U << 0) // Open Limit
#define BTN_PD1      (1U << 1) // Closed Limit

#define RCGCGPIO_ALL ((1U<<1)|(1U<<3)|(1U<<4)|(1U<<5)) // Ports B, D, E, F

#define TOTAL_BUTTONS 7

QueueHandle_t xButtonEventQueue;
SemaphoreHandle_t xGateStateMutex;
SemaphoreHandle_t xInputReadySemaphore;
SemaphoreHandle_t xObstacleSemaphore;
SemaphoreHandle_t xLedSemaphore;
TaskHandle_t xLedTaskHandle = NULL;

uint32_t ReadAllButtons(void);

/* ===========================================================
 * Initialization & Logic
 * =========================================================== */

static void GPIO_Init(void)
{
    SYSCTL_RCGCGPIO_R |= RCGCGPIO_ALL;
    while ((SYSCTL_PRGPIO_R & RCGCGPIO_ALL) != RCGCGPIO_ALL) { }

//    // Port F: LEDs (Output) and SW1 (Input)
//    GPIO_PORTF_AMSEL_R &= ~(BTN_PF4 | LED_MASK);
//    GPIO_PORTF_PCTL_R  &= ~0x000FFFF0U;
//    GPIO_PORTF_AFSEL_R &= ~(BTN_PF4 | LED_MASK);
//    GPIO_PORTF_DIR_R   |=  LED_MASK;
//    GPIO_PORTF_DIR_R   &= ~BTN_PF4;
//    GPIO_PORTF_PUR_R   |=  BTN_PF4;
//    GPIO_PORTF_DEN_R   |=  BTN_PF4 | LED_MASK;
//    GPIO_PORTF_DATA_R  &= ~LED_MASK;

		// Port F: LEDs (Output) and SW1 (Input)
    GPIO_PORTF_AMSEL_R &= ~(BTN_PF4 | BTN_PF0 | LED_MASK);
    GPIO_PORTF_PCTL_R  &= ~0x000FFFF0U;
    GPIO_PORTF_AFSEL_R &= ~(BTN_PF4 | BTN_PF0 | LED_MASK);
    GPIO_PORTF_DIR_R   |=  LED_MASK;
    GPIO_PORTF_DIR_R   &= ~(BTN_PF4 | BTN_PF0);
    GPIO_PORTF_PUR_R   |=  (BTN_PF4 | BTN_PF0);
    GPIO_PORTF_DEN_R   |=  (BTN_PF4 | BTN_PF0 | LED_MASK);
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
		
//		// 1. Port F (PF4) - Falling Edge (Active Low button)
//    GPIO_PORTF_IS_R  &= ~BTN_PF4;     // Edge sensitive
//    GPIO_PORTF_IBE_R |=  BTN_PF4;     // both edges
//    //GPIO_PORTF_IEV_R &= ~BTN_PF4;     // Falling edge
//    GPIO_PORTF_ICR_R  =  BTN_PF4;     // Clear prior flags
//    GPIO_PORTF_IM_R  |=  BTN_PF4;     // Unmask interrupt
		
		// 1. Port F (PF4) - Falling Edge (Active Low button)
    GPIO_PORTF_IS_R  &= ~(BTN_PF4 | BTN_PF0);     // Edge sensitive
    GPIO_PORTF_IBE_R |=  (BTN_PF4 | BTN_PF0);     // both edges
    //GPIO_PORTF_IEV_R &= ~BTN_PF4;     // Falling edge
    GPIO_PORTF_ICR_R  =  (BTN_PF4 | BTN_PF0);     // Clear prior flags
    GPIO_PORTF_IM_R  |=  (BTN_PF4 | BTN_PF0);     // Unmask interrupt

    // 2. Port E (PE0, PE1) - Rising Edge (Active High)
    GPIO_PORTE_IS_R  &= ~(BTN_PE0 | BTN_PE1);
    GPIO_PORTE_IBE_R |= (BTN_PE0 | BTN_PE1);
    //GPIO_PORTE_IEV_R |=  (BTN_PE0 | BTN_PE1); // Rising edge
    GPIO_PORTE_ICR_R  =  (BTN_PE0 | BTN_PE1);
    GPIO_PORTE_IM_R  |=  (BTN_PE0 | BTN_PE1);

    // 3. Port B (PB0, PB1) - Rising Edge (Active High)
    GPIO_PORTB_IS_R  &= ~(BTN_PB0 | BTN_PB1);
    GPIO_PORTB_IBE_R |=  (BTN_PB0 | BTN_PB1);
    //GPIO_PORTB_IEV_R |=  (BTN_PB0 | BTN_PB1);
    GPIO_PORTB_ICR_R  =  (BTN_PB0 | BTN_PB1);
    GPIO_PORTB_IM_R  |=  (BTN_PB0 | BTN_PB1);

    // 4. Port D (PD0, PD1) - Rising Edge (Active High)
    GPIO_PORTD_IS_R  &= ~(BTN_PD0 | BTN_PD1);
    GPIO_PORTD_IBE_R |=  (BTN_PD0 | BTN_PD1);
    //GPIO_PORTD_IEV_R |=  (BTN_PD0 | BTN_PD1);
    GPIO_PORTD_ICR_R  =  (BTN_PD0 | BTN_PD1);
    GPIO_PORTD_IM_R  |=  (BTN_PD0 | BTN_PD1);

    /* --- NVIC Enable --- */
    // Port B = IRQ 1, Port D = IRQ 3, Port E = IRQ 4, Port F = IRQ 30
    NVIC_EN0_R |= (1 << 1) | (1 << 3) | (1 << 4) | (1 << 30);
}

static inline uint32_t Btn_PF4(void) { return (GPIO_PORTF_DATA_R & BTN_PF4) == 0; }
static inline uint32_t Btn_PE0(void) { return (GPIO_PORTE_DATA_R & BTN_PE0) != 0; }
static inline uint32_t Btn_PE1(void) { return (GPIO_PORTE_DATA_R & BTN_PE1) != 0; }
static inline uint32_t Btn_PB0(void) { return (GPIO_PORTB_DATA_R & BTN_PB0) != 0; }
static inline uint32_t Btn_PB1(void) { return (GPIO_PORTB_DATA_R & BTN_PB1) != 0; }
static inline uint32_t Btn_PD0(void) { return (GPIO_PORTD_DATA_R & BTN_PD0) != 0; }
static inline uint32_t Btn_PD1(void) { return (GPIO_PORTD_DATA_R & BTN_PD1) != 0; }


#define OBSTACLE_MASK        (1U << 0)
#define LIMIT_OPEN_MASK      (1U << 1)
#define LIMIT_CLOSED_MASK    (1U << 2)
#define SEC_OPEN_MASK        (1U << 3)
#define SEC_CLOSE_MASK       (1U << 4)
#define DRIVER_OPEN_MASK     (1U << 5)
#define DRIVER_CLOSE_MASK    (1U << 6)

void LED_Set(uint32_t color_mask)
{
    GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R & ~LED_MASK) | (color_mask & LED_MASK);
}


void GPIOF_Handler(void) {
    // Initialized to pdFALSE. If giving the semaphore unblocks a 
    // higher priority task, this will be set to pdTRUE by the API.
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t status = GPIO_PORTF_RIS_R;

    if (status & (BTN_PF4 | BTN_PF0)) {
        GPIO_PORTF_ICR_R = (BTN_PF4 | BTN_PF0);
        xSemaphoreGiveFromISR(xInputReadySemaphore, &xHigherPriorityTaskWoken);
    }

    // If xHigherPriorityTaskWoken is now pdTRUE, force a context switch
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void GPIOB_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t status = GPIO_PORTB_RIS_R;
    
    // Acknowledge/Clear interrupt flags for PB0 and PB1
    GPIO_PORTB_ICR_R = status; 

    // Wake up the Input Task to debounce and process the press
    xSemaphoreGiveFromISR(xInputReadySemaphore, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void GPIOD_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t status = GPIO_PORTD_RIS_R;
    
    // Acknowledge/Clear interrupt flags for PD0 and PD1
    GPIO_PORTD_ICR_R = status; 

    // Wake up the Input Task
    xSemaphoreGiveFromISR(xInputReadySemaphore, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void GPIOE_Handler(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t status = GPIO_PORTE_RIS_R;
    
    // Acknowledge/Clear interrupt flags for PE0 and PE1
    GPIO_PORTE_ICR_R = status; 

    // Wake up the Input Task
    xSemaphoreGiveFromISR(xInputReadySemaphore, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void inputTask(void *pvParameters)
{
    uint32_t physical_last_states = 0; // Tracks the actual physical buttons
    TickType_t press_times[TOTAL_BUTTONS] = {0};
    ButtonMsg_t msg;

    for (;;) {
        if (xSemaphoreTake(xInputReadySemaphore, portMAX_DELAY) == pdPASS) {
            vTaskDelay(pdMS_TO_TICKS(50)); 
            
            uint32_t physical_current_states = ReadAllButtons();
            
            // Create a temporary copy to apply "Discard" logic
            uint32_t filtered_states = physical_current_states;
						bool sec_conflict = ((filtered_states & SEC_OPEN_MASK) && (filtered_states & SEC_CLOSE_MASK));
            bool drv_conflict = ((filtered_states & DRIVER_OPEN_MASK) && (filtered_states & DRIVER_CLOSE_MASK));
            static bool conflict_active = false;

						if (filtered_states & OBSTACLE_MASK) {
                filtered_states &= OBSTACLE_MASK; 
            }
						
            else if (sec_conflict || drv_conflict) {
                // 1. Send an explicit STOP message to the queue
                // We use a check to ensure we only send this once per conflict event
                if (!conflict_active) {
                    msg.button = (sec_conflict) ? BTN_SECURITY_OPEN : BTN_OPEN;
                    msg.event = EVENT_CONFLICT_STOP; // You must define this in your enum
                    xQueueSend(xButtonEventQueue, &msg, 0);
                    conflict_active = true;
                }
                
                // 2. Clear these buttons from filtered_states so the edge detector 
                // doesn't process them as normal "Press" events.
                filtered_states &= 0;
            } 
						
						else if(!sec_conflict && !drv_conflict){
                conflict_active = false;
            }
						
            else if (filtered_states & (LIMIT_OPEN_MASK | LIMIT_CLOSED_MASK)) {
                filtered_states &= (LIMIT_OPEN_MASK | LIMIT_CLOSED_MASK);
            }
						
            else if (filtered_states & (SEC_OPEN_MASK | SEC_CLOSE_MASK)) {
                filtered_states &= (SEC_OPEN_MASK | SEC_CLOSE_MASK);
            }

            // --- 2. EDGE DETECTION (Using physical states for tracking) ---
            for (int i = 0; i < TOTAL_BUTTONS; i++) {
                uint32_t mask = (1U << i);
                
                // Compare the PHYSICAL change
                if ((physical_current_states & mask) != (physical_last_states & mask)) {
									
									// --- NEW: OBSTACLE INTERCEPT ---
                    if (i == BTN_OBSTACLE) {
                        if (physical_current_states & mask) {
                            // Wake up the Highest Priority Safety Task immediately
                            xSemaphoreGive(xObstacleSemaphore);
                        }
                        continue; // Skip the standard queue logic for the obstacle button
                    }
                    
                    // ONLY send to queue if this button survived the "Filter/Discard" check
                    if (filtered_states & mask || !(physical_current_states & mask)) {
                        msg.button = (ButtonID_t)i;

                        if (physical_current_states & mask) {
                            msg.event = EVENT_PRESSED;
                            press_times[i] = xTaskGetTickCount();
                            xQueueSend(xButtonEventQueue, &msg, 0);
                        } else {
                            TickType_t duration = xTaskGetTickCount() - press_times[i];
                            msg.event = (duration < pdMS_TO_TICKS(1000)) ? 
                                         EVENT_RELEASE_SHORT_AUTO : EVENT_RELEASE_LONG_STOP;
                            xQueueSend(xButtonEventQueue, &msg, 0);
                        }
                    }
                }
            }

            // --- 3. THE KEY: Save the physical state, NOT the filtered state ---
            physical_last_states = physical_current_states;
        }
    }
}

uint32_t ReadAllButtons(void) {
    uint32_t buttons = 0;

//    // Port F: PF4 is Driver OPEN (Active Low, so we use ! operator)
//    if (!(GPIO_PORTF_DATA_R & (1U << 4))) buttons |= (1U << BTN_OBSTACLE);

//    // Port E: PE0 is Driver CLOSE, PE1 is Security OPEN (Active High)
//    if (GPIO_PORTE_DATA_R & (1U << 0)) buttons |= (1U << BTN_OPEN);
//    if (GPIO_PORTE_DATA_R & (1U << 1)) buttons |= (1U << BTN_CLOSE);

//    // Port B: PB0 is Security CLOSE, PB1 is Open Limit (Active High)
//    if (GPIO_PORTB_DATA_R & (1U << 0)) buttons |= (1U << BTN_SECURITY_OPEN);
//    if (GPIO_PORTB_DATA_R & (1U << 1)) buttons |= (1U << BTN_SECURITY_CLOSE);

//    // Port D: PD0 is Closed Limit, PD1 is Obstacle (Active High)
//    if (GPIO_PORTD_DATA_R & (1U << 0)) buttons |= (1U << BTN_LIMIT_OPEN);
//    if (GPIO_PORTD_DATA_R & (1U << 1)) buttons |= (1U << BTN_LIMIT_CLOSED);
	
			if (GPIO_PORTE_DATA_R & (1U << 0)) buttons |= (1U << BTN_OBSTACLE);

			if (GPIO_PORTD_DATA_R & (1U << 0)) buttons |= (1U << BTN_LIMIT_OPEN);
			if (GPIO_PORTD_DATA_R & (1U << 1)) buttons |= (1U << BTN_LIMIT_CLOSED);

			if (!(GPIO_PORTF_DATA_R & (1U << 4))) buttons |= (1U << BTN_SECURITY_OPEN);
			if (!(GPIO_PORTF_DATA_R & (1U << 0))) buttons |= (1U << BTN_SECURITY_CLOSE);

			if (GPIO_PORTB_DATA_R & (1U << 0)) buttons |= (1U << BTN_OPEN);
			if (GPIO_PORTB_DATA_R & (1U << 1)) buttons |= (1U << BTN_CLOSE);

    return buttons;
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

    GPIO_Init();
		
		// 2. Create RTOS Objects
    xButtonEventQueue = xQueueCreate(10, sizeof(ButtonMsg_t));
    xGateStateMutex = xSemaphoreCreateMutex();
    xInputReadySemaphore = xSemaphoreCreateBinary();
    xObstacleSemaphore = xSemaphoreCreateBinary(); 
		xLedSemaphore = xSemaphoreCreateBinary();
		
		if (xButtonEventQueue != NULL && xGateStateMutex != NULL && 
        xInputReadySemaphore != NULL && xObstacleSemaphore != NULL) 
    {
        // 3. Create Tasks with strict priority assignments
        
        // Safety Task: Highest Priority
        xTaskCreate(safetyTask,  "Safety", 256, NULL, 4, NULL);

        // Input Task: High Priority 
        xTaskCreate(inputTask, "Input", 256, NULL, 3, NULL);

        // Gate Control Task: Medium Priority 
        xTaskCreate(gateControlTask, "GateCtrl",  256, NULL, 2, NULL);

        // LED Control Task: Medium Priority 
        xTaskCreate(ledControlTask, "LEDTask", 128,  NULL, 2, &xLedTaskHandle);
			
		// Start the Scheduler
    vTaskStartScheduler();
		}

		while (1);
}