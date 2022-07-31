// joinme.h
// IoT device provisioning over wifi, captive portal and OTA for the ESP32

#ifndef JOINME_H
#define JOINME_H

// TODO move startAP, startWebServer, and basic handlers in here and verify
// with OTAThing and waterelf

#include <HTTPClient.h>

// WiFi provisioning ////////////////////////////////////////////////////////
bool joinmeManageWiFi(const char *apSSID, const char *apKey); // true if joins

// DNS stuff ////////////////////////////////////////////////////////////////
void joinmeDNSSetup(void* server, IPAddress apIP); // capture clients
void joinmeTurn(); // run once per main loop iteration
// note: you *must* regularly call joinmeTurn;
// server *must* live until after you last call joinmeTurn

// OTA stuff ////////////////////////////////////////////////////////////////
void joinmeOTAUpdate(int, String, String, String);         // main OTA logic
int  joinmeCloudGet(HTTPClient *, String, String, String); // download 'ware

#endif
