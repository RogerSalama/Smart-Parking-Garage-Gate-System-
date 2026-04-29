#ifndef GATE_CONTROL_H
#define GATE_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define TOTAL_BUTTONS 7

typedef enum {
    IDLE_CLOSED,
    OPENING,
    IDLE_OPEN,
    CLOSING,
    STOPPED_MIDWAY,
    REVERSING
} GateState_t;

typedef enum {
    BTN_OBSTACLE = 0,
		BTN_LIMIT_OPEN = 1,
    BTN_LIMIT_CLOSED = 2,
		BTN_SECURITY_OPEN = 3,
    BTN_SECURITY_CLOSE = 4,
		BTN_OPEN = 5,
    BTN_CLOSE = 6
} ButtonID_t;

typedef enum {
    EVENT_PRESSED,
    EVENT_RELEASE_SHORT_AUTO,  // Trigger One-Touch Auto
    EVENT_RELEASE_LONG_STOP,   // Stop Manual Movement
    EVENT_CONFLICT_STOP        // Specific signal when OPEN & CLOSE are pressed together
} EventType_t;

typedef struct {
    ButtonID_t button;
    EventType_t event;
} ButtonMsg_t;

// Shared RTOS handles (created in main.c)
extern QueueHandle_t xButtonEventQueue;
extern SemaphoreHandle_t xGateStateMutex;
extern GateState_t currentGateState;
extern SemaphoreHandle_t xObstacleSemaphore;

// External hardware functions (defined in main.c)
extern void LED_Set(uint32_t color_mask);

// LED Masks needed for the LED Task
#define LED_RED      (1U << 1)
#define LED_GREEN    (1U << 3)

// Task Prototypes
void gateControlTask(void *pvParameters);
void ledControlTask(void *pvParameters);
void safetyTask(void *pvParameters) ;

#endif /* GATE_CONTROL_H */