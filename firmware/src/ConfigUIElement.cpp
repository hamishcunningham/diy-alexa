// ConfigUIElement.cpp

#include "AllUIElement.h"
#include <WiFi.h>

extern int firmwareVersion;
extern String apSSID;

// handle touch on this page ////////////////////////////////////////////////
bool ConfigUIElement::handleTouch(long x, long y) {
  return y < BOXSIZE && x > (BOXSIZE * SWITCHER);
}

// writes various things including mac address and wifi ssid ///////////////
void ConfigUIElement::draw(){
  // say hello
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);
  uint16_t yCursor = 0;
  m_tft->setCursor(0, yCursor);
  m_tft->print("Welcome to unPhone!");
  m_tft->setTextColor(BLUE);

  // note about switcher
  yCursor += 20;
  if(UIController::provisioned) {
    showLine("(where you see the arrow,", &yCursor);
    showLine("  press for menu screen)", &yCursor);
    drawSwitcher();
  } else {
    yCursor += 40;
  }

  // are we connected?
  yCursor += 60;
  m_tft->setCursor(0, yCursor);
  if (WiFi.status() == WL_CONNECTED) {
    m_tft->print("Connected to: ");
    m_tft->setTextColor(GREEN);
    m_tft->print(WiFi.SSID());
    m_tft->setTextColor(BLUE);
  } else {
    m_tft->setTextColor(RED);
    m_tft->print("Not connected to WiFi:");
    yCursor += 20;
    m_tft->setCursor(0, yCursor);
    m_tft->print("  trying to connect...");
    m_tft->setTextColor(BLUE);
  }

/* TODO always reads same value :(
  // display the on-board temperature
  char buf[256];
  float onBoardTemp = temperatureRead();
  sprintf(buf, "MCU temp: %.2f C", onBoardTemp);
  yCursor += 40;
  m_tft->setCursor(0, yCursor);
  m_tft->print(buf);
*/

  // display the mac address
  char mac_buf[13];
  yCursor += 40;
  m_tft->setCursor(0, yCursor);
  m_tft->print("MAC addr: ");
  m_tft->print(unPhone::getMAC(mac_buf));

  // firmware version
  showLine("Firmware version: ", &yCursor);
  m_tft->print(firmwareVersion);

  // AP details
  showLine("AP SSID: ", &yCursor);
  showLine("  ", &yCursor);
  m_tft->print(apSSID);

  // IP address
  showLine("IP: ", &yCursor);
  m_tft->print(WiFi.localIP());

  // battery voltage
  showLine("VBAT: ", &yCursor);
  m_tft->print(unPhone::batteryVoltage());

  // battery voltage
  showLine("Hardware version: ", &yCursor);
  m_tft->print(UNPHONE_SPIN);

  // web link
  yCursor += 60;
  showLine("An ", &yCursor);
  m_tft->setTextColor(MAGENTA);
  m_tft->print("IoT platform");
  m_tft->setTextColor(BLUE);
  m_tft->print(" from the");
  m_tft->setTextColor(MAGENTA);
  showLine("  University of Sheffield", &yCursor);
  m_tft->setTextColor(BLUE);
  showLine("Find out more at", &yCursor);
  m_tft->setTextColor(GREEN);
  showLine("              unphone.net", &yCursor);
}

//////////////////////////////////////////////////////////////////////////
void ConfigUIElement::runEachTurn(){
  
}
