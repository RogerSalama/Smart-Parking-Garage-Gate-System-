#include "gate_control.h"

void safetyTask(void *pvParameters) {
    for (;;) {
        // Wait for the obstacle button to be pressed
        if (xSemaphoreTake(xObstacleSemaphore, portMAX_DELAY) == pdPASS) {
            
            // Take the state mutex to safely check and alter the state
            xSemaphoreTake(xGateStateMutex, portMAX_DELAY);
            
            // Obstacle logic only triggers if the gate is actively closing
            if (currentGateState == CLOSING) { 
                currentGateState = REVERSING;  
                
                // CRITICAL: Release the mutex before the delay so the LED task can run!
                xSemaphoreGive(xGateStateMutex);
                
                // Reverse for 0.5 seconds
                vTaskDelay(pdMS_TO_TICKS(500)); 
                
                // Take mutex again to perform the final stop
                xSemaphoreTake(xGateStateMutex, portMAX_DELAY);
                currentGateState = STOPPED_MIDWAY;
                xSemaphoreGive(xGateStateMutex);
                
            } else {
                // If the gate is OPENING or stopped, ignore the obstacle and release the mutex
                xSemaphoreGive(xGateStateMutex); 
            }
        }
    }
}

