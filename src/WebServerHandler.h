#ifndef WEB_SERVER_HANDLER_H
#define WEB_SERVER_HANDLER_H

#include <WebServer.h>

// External server object
extern WebServer server;

// Function declarations
void setupWebServer();
void startWebServer();
void stopWebServer();
void restartWebServer(); // New function for restarting WiFi
void handleRoot();
void handleUpdate();
void handleToggle();
void handleWebServerClients();
#ifdef __cplusplus
extern "C" {
#endif

void handleCanSpeed();

#ifdef __cplusplus
}
#endif

#endif // WEB_SERVER_HANDLER_H
