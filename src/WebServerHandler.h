#ifndef WEB_SERVER_HANDLER_H
#define WEB_SERVER_HANDLER_H

#include <WebServer.h>

// External server object
extern WebServer server;

// Function declarations
void setupWebServer();
void handleRoot();
void handleUpdate();
void handleToggle();
void handleWebServerClients();

#endif // WEB_SERVER_HANDLER_H
