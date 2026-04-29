#include "gate_control.h"

// Define the shared state variable here
GateState_t currentGateState = IDLE_CLOSED;

void gateControlTask(void *pvParameters) {
    ButtonMsg_t msg;
    GateState_t nextState;

    for (;;) {
        if (xQueueReceive(xButtonEventQueue, &msg, portMAX_DELAY) == pdPASS) {
            
            xSemaphoreTake(xGateStateMutex, portMAX_DELAY);
            nextState = currentGateState; 
            
            switch (currentGateState) {

							  case REVERSING:
                    // Handled exclusively by the safetyTask. We ignore normal queue inputs here.
                    break;
							  case CLOSING:
                    if (msg.event == EVENT_CONFLICT_STOP) {
                        nextState = STOPPED_MIDWAY;
                    } 
                    else if (msg.event == EVENT_RELEASE_LONG_STOP && (msg.button == BTN_CLOSE || msg.button == BTN_SECURITY_CLOSE)) {
                        nextState = STOPPED_MIDWAY; 
                    } 
                    else if (msg.event == EVENT_PRESSED && msg.button == BTN_LIMIT_CLOSED) {
                        nextState = IDLE_CLOSED; 
                    }
                    break;

                case IDLE_CLOSED:
                    if (msg.event == EVENT_PRESSED && (msg.button == BTN_OPEN || msg.button == BTN_SECURITY_OPEN)) {
                        nextState = OPENING;
                    }
                    break;

                case IDLE_OPEN:
                    if (msg.event == EVENT_PRESSED && (msg.button == BTN_CLOSE || msg.button == BTN_SECURITY_CLOSE)) {
                        nextState = CLOSING;
                    }
                    break;

                case STOPPED_MIDWAY:
                    if (msg.event == EVENT_PRESSED) {
                        if (msg.button == BTN_OPEN || msg.button == BTN_SECURITY_OPEN) nextState = OPENING;
                        else if (msg.button == BTN_CLOSE || msg.button == BTN_SECURITY_CLOSE) nextState = CLOSING;
                    }
                    break;

                case OPENING:
                    if (msg.event == EVENT_CONFLICT_STOP) {
                        nextState = STOPPED_MIDWAY;
                    } 
                    else if (msg.event == EVENT_RELEASE_LONG_STOP && (msg.button == BTN_OPEN || msg.button == BTN_SECURITY_OPEN)) {
                        nextState = STOPPED_MIDWAY; 
                    } 
                    else if (msg.event == EVENT_PRESSED && msg.button == BTN_LIMIT_OPEN) {
                        nextState = IDLE_OPEN; 
                    }
                    break;
                
            }

            currentGateState = nextState;
            xSemaphoreGive(xGateStateMutex);
						
        }
    }
}

void ledControlTask(void *pvParameters) {
    GateState_t localState;
    
    for (;;) {
        // Safely read the current state
        xSemaphoreTake(xGateStateMutex, portMAX_DELAY);
        localState = currentGateState;
        xSemaphoreGive(xGateStateMutex);

        // Update LEDs based on the state machine definitions
        if (localState == OPENING || localState == REVERSING) {
            LED_Set(LED_GREEN);
        } else if (localState == CLOSING) {
            LED_Set(LED_RED);
        } else {
            // Includes IDLE_OPEN, IDLE_CLOSED, and STOPPED_MIDWAY
            LED_Set(0); // All LEDs OFF
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Poll every 50ms
    }
}