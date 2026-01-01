#ifndef OTA_UPDATER_H
#define OTA_UPDATER_H

#include <Arduino.h>
#include <Update.h>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <TFT_eSPI.h>

// Forward declaration
extern TFT_eSPI tft;

// OTA Task Configuration
#define OTA_TASK_STACK_SIZE 8192
#define OTA_TASK_PRIORITY 5
#define OTA_QUEUE_SIZE 8
#define OTA_BUFFER_SIZE 1024

enum OTAState {
    OTA_IDLE,
    OTA_ACTIVE,
    OTA_SUCCESS,
    OTA_FAILED,
    OTA_ABORTED
};

enum OTACommand {
    OTA_CMD_START,
    OTA_CMD_WRITE,
    OTA_CMD_FINISH,
    OTA_CMD_ABORT
};

struct OTAMessage {
    OTACommand command;
    uint8_t* data;
    size_t size;
    size_t totalSize;  // Only used for OTA_CMD_START
};

class OTAUpdater {
public:
    OTAUpdater();
    ~OTAUpdater();
    
    bool initialize();
    void shutdown();
    
    // Called from HTTP upload callbacks
    bool startOTA(size_t totalSize);
    bool writeData(uint8_t* data, size_t len);
    bool finishOTA();
    void abortOTA();
    
    OTAState getState() const { return currentState; }
    String getError() const { return lastError; }
    uint32_t getProgress() const { return bytesWritten; }
    
private:
    // OTA Task - runs all Update library operations
    static void otaTaskFunction(void* parameter);
    
    // System cleanup before OTA
    bool prepareForOTA();
    
    // Task communication
    TaskHandle_t otaTaskHandle;
    QueueHandle_t otaQueue;
    
    // State management
    volatile OTAState currentState;
    String lastError;
    uint32_t bytesWritten;
    uint32_t totalBytes;
    
    // Initialization flag
    bool initialized;
};

#endif // OTA_UPDATER_H