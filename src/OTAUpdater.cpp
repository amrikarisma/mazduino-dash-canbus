#include "OTAUpdater.h"
#include <esp_task_wdt.h>
#include <SPIFFS.h>

OTAUpdater::OTAUpdater() :
    otaTaskHandle(nullptr),
    otaQueue(nullptr),
    currentState(OTA_IDLE),
    bytesWritten(0),
    totalBytes(0),
    initialized(false)
{
}

OTAUpdater::~OTAUpdater() {
    shutdown();
}

bool OTAUpdater::initialize() {
    if (initialized) {
        Serial.println("OTA already initialized");
        return true;
    }
    
    // Create queue for OTA messages
    otaQueue = xQueueCreate(OTA_QUEUE_SIZE, sizeof(OTAMessage*));
    if (otaQueue == nullptr) {
        Serial.println("Failed to create OTA queue");
        return false;
    }
    
    // DON'T create task yet - create it only when OTA starts
    initialized = true;
    currentState = OTA_IDLE;
    Serial.println("OTA system initialized (task will be created on-demand)");
    return true;
}

void OTAUpdater::shutdown() {
    if (!initialized) return;
    
    // Send abort command if OTA is active
    if (currentState == OTA_ACTIVE) {
        abortOTA();
        vTaskDelay(pdMS_TO_TICKS(2000)); // Wait for cleanup
    }
    
    // Delete task
    if (otaTaskHandle != nullptr) {
        vTaskDelete(otaTaskHandle);
        otaTaskHandle = nullptr;
    }
    
    // Delete queue
    if (otaQueue != nullptr) {
        vQueueDelete(otaQueue);
        otaQueue = nullptr;
    }
    
    initialized = false;
    currentState = OTA_IDLE;
    Serial.println("OTA system shutdown");
}

bool OTAUpdater::startOTA(size_t totalSize) {
    if (!initialized) {
        lastError = "OTA not initialized";
        return false;
    }
    
    if (currentState != OTA_IDLE) {
        lastError = "OTA already in progress";
        return false;
    }
    
    // Create OTA task now if it doesn't exist
    if (otaTaskHandle == nullptr) {
        BaseType_t result = xTaskCreatePinnedToCore(
            otaTaskFunction,
            "OTA_Task",
            OTA_TASK_STACK_SIZE,
            this,
            OTA_TASK_PRIORITY,
            &otaTaskHandle,
            1  // Pin to core 1
        );
        
        if (result != pdPASS) {
            lastError = "Failed to create OTA task";
            Serial.println("ERROR: " + lastError);
            return false;
        }
        
        Serial.println("OTA Task created on-demand");
        // Give task time to initialize
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    // Create start message
    OTAMessage* msg = (OTAMessage*)malloc(sizeof(OTAMessage));
    if (!msg) {
        lastError = "Memory allocation failed";
        return false;
    }
    
    msg->command = OTA_CMD_START;
    msg->data = nullptr;
    msg->size = 0;
    msg->totalSize = totalSize;
    
    if (xQueueSend(otaQueue, &msg, pdMS_TO_TICKS(1000)) != pdPASS) {
        free(msg);
        lastError = "Failed to queue start command";
        return false;
    }
    
    currentState = OTA_ACTIVE;
    bytesWritten = 0;
    totalBytes = totalSize;
    lastError = "";
    
    Serial.printf("OTA start queued: %u bytes\n", totalSize);
    return true;
}

bool OTAUpdater::writeData(uint8_t* data, size_t len) {
    if (currentState != OTA_ACTIVE) {
        lastError = "OTA not active";
        return false;
    }
    
    // Allocate buffer for data
    uint8_t* buffer = (uint8_t*)malloc(len);
    if (!buffer) {
        lastError = "Memory allocation failed";
        return false;
    }
    
    memcpy(buffer, data, len);
    
    // Create write message
    OTAMessage* msg = (OTAMessage*)malloc(sizeof(OTAMessage));
    if (!msg) {
        free(buffer);
        lastError = "Memory allocation failed";
        return false;
    }
    
    msg->command = OTA_CMD_WRITE;
    msg->data = buffer;
    msg->size = len;
    msg->totalSize = 0;
    
    if (xQueueSend(otaQueue, &msg, pdMS_TO_TICKS(5000)) != pdPASS) {
        free(buffer);
        free(msg);
        lastError = "Failed to queue write command";
        return false;
    }
    
    return true;
}

bool OTAUpdater::finishOTA() {
    if (currentState != OTA_ACTIVE) {
        lastError = "OTA not active";
        return false;
    }
    
    // Create finish message
    OTAMessage* msg = (OTAMessage*)malloc(sizeof(OTAMessage));
    if (!msg) {
        lastError = "Memory allocation failed";
        return false;
    }
    
    msg->command = OTA_CMD_FINISH;
    msg->data = nullptr;
    msg->size = 0;
    msg->totalSize = 0;
    
    if (xQueueSend(otaQueue, &msg, pdMS_TO_TICKS(1000)) != pdPASS) {
        free(msg);
        lastError = "Failed to queue finish command";
        return false;
    }
    
    Serial.println("OTA finish queued");
    return true;
}

void OTAUpdater::abortOTA() {
    // Create abort message
    OTAMessage* msg = (OTAMessage*)malloc(sizeof(OTAMessage));
    if (!msg) return;
    
    msg->command = OTA_CMD_ABORT;
    msg->data = nullptr;
    msg->size = 0;
    msg->totalSize = 0;
    
    xQueueSend(otaQueue, &msg, 0); // Don't block
    
    Serial.println("OTA abort queued");
}

void OTAUpdater::otaTaskFunction(void* parameter) {
    OTAUpdater* updater = static_cast<OTAUpdater*>(parameter);
    
    // CRITICAL: Register task with watchdog FIRST
    esp_task_wdt_add(NULL);
    
    Serial.println("OTA Task started with watchdog registration");
    
    bool otaInitialized = false;
    
    while (true) {
        // CRITICAL: Feed watchdog at start of loop
        esp_task_wdt_reset();
        
        OTAMessage* msg = nullptr;
        
        // Wait for messages with timeout
        if (xQueueReceive(updater->otaQueue, &msg, pdMS_TO_TICKS(5000)) != pdPASS) {
            // Timeout - continue to feed watchdog
            continue;
        }
        
        if (!msg) continue;
        
        // Process OTA command
        switch (msg->command) {
            case OTA_CMD_START:
                {
                    Serial.printf("OTA: Starting update (%u bytes)\n", msg->totalSize);
                    
                    // Prepare system for OTA
                    if (!updater->prepareForOTA()) {
                        updater->lastError = "Failed to prepare system for OTA";
                        updater->currentState = OTA_FAILED;
                        break;
                    }
                    
                    // Initialize Update library
                    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                        updater->lastError = "Update.begin failed";
                        Serial.printf("OTA: Update.begin failed: %d\n", Update.getError());
                        Update.printError(Serial);
                        updater->currentState = OTA_FAILED;
                    } else {
                        otaInitialized = true;
                        updater->bytesWritten = 0;
                        Serial.println("OTA: Update started successfully");
                    }
                }
                break;
                
            case OTA_CMD_WRITE:
                {
                    if (!otaInitialized) {
                        updater->lastError = "OTA not initialized";
                        updater->currentState = OTA_FAILED;
                        break;
                    }
                    
                    // Write data to flash - ONLY from this task!
                    size_t written = Update.write(msg->data, msg->size);
                    if (written != msg->size) {
                        updater->lastError = "Write failed";
                        Serial.printf("OTA: Write failed %u/%u, error: %d\n", 
                                     written, msg->size, Update.getError());
                        Update.printError(Serial);
                        Update.abort();
                        updater->currentState = OTA_FAILED;
                        otaInitialized = false;
                    } else {
                        updater->bytesWritten += written;
                        // Progress feedback every 50KB
                        if (updater->bytesWritten % 51200 == 0) {
                            Serial.printf("OTA: Progress %u bytes\n", updater->bytesWritten);
                        }
                    }
                }
                break;
                
            case OTA_CMD_FINISH:
                {
                    if (!otaInitialized) {
                        updater->lastError = "OTA not initialized";
                        updater->currentState = OTA_FAILED;
                        break;
                    }
                    
                    // Finish OTA update
                    if (!Update.end(true)) {
                        updater->lastError = "Update.end failed";
                        Serial.printf("OTA: Update.end failed: %d\n", Update.getError());
                        Update.printError(Serial);
                        updater->currentState = OTA_FAILED;
                    } else {
                        updater->currentState = OTA_SUCCESS;
                        Serial.println("OTA: Update completed successfully!");
                        Serial.printf("OTA: Total bytes: %u\n", updater->bytesWritten);
                        
                        // Clean exit from watchdog before reboot
                        esp_task_wdt_delete(NULL);
                        
                        // Brief delay then reboot
                        vTaskDelay(pdMS_TO_TICKS(2000));
                        ESP.restart();
                    }
                    otaInitialized = false;
                }
                break;
                
            case OTA_CMD_ABORT:
                {
                    Serial.println("OTA: Aborting update");
                    if (otaInitialized && Update.isRunning()) {
                        Update.abort();
                        otaInitialized = false;
                    }
                    updater->currentState = OTA_IDLE;
                }
                break;
        }
        
        // Clean up message
        if (msg->data) {
            free(msg->data);
        }
        free(msg);
        
        // Yield to other tasks
        vTaskDelay(1);
    }
    
    // Clean exit (should never reach here)
    esp_task_wdt_delete(NULL);
    vTaskDelete(NULL);
}

bool OTAUpdater::prepareForOTA() {
    Serial.println("OTA: Preparing system for update");
    
    try {
        // Stop TFT operations
        Serial.println("OTA: Stopping display");
        // Note: Some TFT libraries don't have end(), so we avoid operations instead
        
        // Stop SPIFFS if mounted
        if (SPIFFS.begin()) {
            SPIFFS.end();
            Serial.println("OTA: SPIFFS stopped");
        }
        
        // Additional system preparations can be added here:
        // - Stop other tasks that access flash
        // - Disable interrupts that might interfere
        // - Free up maximum RAM
        
        Serial.println("OTA: System prepared for update");
        return true;
        
    } catch (...) {
        Serial.println("OTA: Exception during system preparation");
        return false;
    }
}
