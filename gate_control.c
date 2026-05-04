#include "gate_control.h"

// Define the shared state variable here
GateState_t currentGateState = IDLE_CLOSED;

void gateControlTask(void *pvParameters) {
    ButtonMsg_t msg;
    GateState_t nextState;
    
    // 1. Initialize the flag to ensure predictable startup behavior
    bool is_Security = false; 

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
                        // If Security is moving the gate, ignore Driver conflicts.
                        // If Driver is moving the gate, ANY conflict stops it.
                        if ((is_Security == true && msg.button == BTN_SECURITY_OPEN) || (is_Security == false)) {
                            nextState = STOPPED_MIDWAY;
                        }
                    } 
                    else if (msg.event == EVENT_RELEASE_LONG_STOP) {
                        // Prevent driver releases from stopping a security-driven movement
                        if (is_Security == true && msg.button == BTN_SECURITY_CLOSE) {
                            nextState = STOPPED_MIDWAY; 
                        }
                        else if (is_Security == false && (msg.button == BTN_CLOSE || msg.button == BTN_SECURITY_CLOSE)) {
                            nextState = STOPPED_MIDWAY;
                        }
                    } 
                    else if (msg.event == EVENT_PRESSED && msg.button == BTN_LIMIT_CLOSED) {
                        nextState = IDLE_CLOSED; 
                    }
                    else if (msg.event == EVENT_PRESSED && msg.button == BTN_SECURITY_OPEN){
                        // Immediate Security Override
                        nextState = OPENING;
                        is_Security = true;
                    }
                    break;

                case IDLE_CLOSED:
                    if (msg.event == EVENT_PRESSED) {
                        if(msg.button == BTN_OPEN){
                            nextState = OPENING;
                            is_Security = false;
                        }
                        else if(msg.button == BTN_SECURITY_OPEN){
                            nextState = OPENING;
                            is_Security = true;
                        }
                    }
                    break;

                case IDLE_OPEN:
                    if (msg.event == EVENT_PRESSED && (msg.button == BTN_CLOSE || msg.button == BTN_SECURITY_CLOSE)) {
                        if(msg.button == BTN_CLOSE){
                            nextState = CLOSING;
                            is_Security = false;
                        }
                        else if(msg.button == BTN_SECURITY_CLOSE){
                            nextState = CLOSING;
                            is_Security = true;
                        }
                    }
                    break;

                case STOPPED_MIDWAY:
                    if (msg.event == EVENT_PRESSED) {
                        if (msg.button == BTN_OPEN){
                            nextState = OPENING;
                            is_Security = false;
                        }
                        else if(msg.button == BTN_SECURITY_OPEN){
                            nextState = OPENING;
                            is_Security = true;
                        }
                        else if (msg.button == BTN_CLOSE) {
                            nextState = CLOSING;
                            is_Security = false;
                        }
                        else if(msg.button == BTN_SECURITY_CLOSE){
                            nextState = CLOSING;
                            is_Security = true;
                        }
                    }
                    break;

                case OPENING:
                    if (msg.event == EVENT_CONFLICT_STOP) {
                        // If Security is moving the gate, ignore Driver conflicts.
                        // If Driver is moving the gate, ANY conflict stops it.
                        if ((is_Security == true && msg.button == BTN_SECURITY_OPEN) || (is_Security == false)) {
                            nextState = STOPPED_MIDWAY;
                        }
                    }
                    else if (msg.event == EVENT_RELEASE_LONG_STOP) {
                        // Prevent driver releases from stopping a security-driven movement
                        if (is_Security == true && msg.button == BTN_SECURITY_OPEN) {
                            nextState = STOPPED_MIDWAY; 
                        }
                        else if (is_Security == false && (msg.button == BTN_OPEN || msg.button == BTN_SECURITY_OPEN)) {
                            nextState = STOPPED_MIDWAY;
                        }
                    } 
                    else if (msg.event == EVENT_PRESSED && msg.button == BTN_LIMIT_OPEN) {
                        nextState = IDLE_OPEN; 
                    }
                    else if (msg.event == EVENT_PRESSED && msg.button == BTN_SECURITY_CLOSE){
                        // Immediate Security Override
                        nextState = CLOSING;
                        is_Security = true;
                    }
                    break;
            }

            currentGateState = nextState;
            xSemaphoreGive(xGateStateMutex);
            xSemaphoreGive(xLedSemaphore);
            taskYIELD();
        }
    }
}

void ledControlTask(void *pvParameters) {
    
    for (;;) {
        // Safely read the current state
				xSemaphoreTake(xLedSemaphore, portMAX_DELAY);
        xSemaphoreTake(xGateStateMutex, portMAX_DELAY);

        // Update LEDs based on the state machine definitions
        if (currentGateState == OPENING || currentGateState == REVERSING) {
            LED_Set(LED_GREEN);
        } else if (currentGateState == CLOSING) {
            LED_Set(LED_RED);
        } else {
            // Includes IDLE_OPEN, IDLE_CLOSED, and STOPPED_MIDWAY
            LED_Set(0); // All LEDs OFF
        }
				xSemaphoreGive(xGateStateMutex);
    }
}