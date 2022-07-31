// unphone.h
// core definitions and includes

#ifndef UNPHONE_H
#define UNPHONE_H

// include this first so we can pick up pins_arduino.h for the current board
#include <Arduino.h>

// define the default hardware spin=6 if it is not set by the build context
// (spin6 uses the featheresp32 board definition, other spins are present in
// boards.txt and <device>.json)
#ifndef UNPHONE_SPIN
#define UNPHONE_SPIN 6
#endif

#include "joinme.h"               // OTA etc.
// #include "lora.h"                 // LoRaWAN and TTN API
#include <stdint.h>               // integer types
#include "IOExpander.h"           // the IO expander chip

// support for unPhone's TCA9555 IO expander chip (also injected into
// libraries that talk to components connected to the expander); behaviour is
// the same as the normally scoped methods except when using pins defined in
// IOExpander.h (which are OR'd with 0x40)
#define digitalWrite IOExpander::digitalWrite // call...
#define digitalRead  IOExpander::digitalRead  // ...ours...
#define pinMode      IOExpander::pinMode      // ...please

//#include <SdFat.h>                // SD card & FAT filesystem library
// #include <Arduino_LoRaWAN_ttn.h>  // MCCI LoraWAN library
// #include <mcciadk_env.h>
// #include <arduino_lmic_hal_configuration.h>
#include <SPI.h>                  // the SPI bus
#include <Adafruit_GFX.h>         // core graphics library
#include <Adafruit_HX8357.h>      // tft display

#if UNPHONE_SPIN == 6
#include <Adafruit_STMPE610.h>    // touch screen
#include <Adafruit_LSM303_U.h>    // the accelerometer sensor
#elif UNPHONE_SPIN >= 7
#include <XPT2046_Touchscreen.h>  // touch screen
#include <Adafruit_LSM9DS1.h>     // the accelerometer sensor
#endif

#include <Wire.h>                 // I²C comms on the Arduino
#include <IOExpander.h>           // unPhone's IOExpander (controlled via I²C)
#include <Adafruit_Sensor.h>      // base class etc. for sensor abstraction
#include <driver/i2s.h>           // ESP I²S bus

// code specific to unphone hardware
class unPhone {
public:
  static void begin(); // initialise hardware

  // buttons, IR LEDs, expander power switching
#if UNPHONE_SPIN == 6
  static const uint8_t BUTTON1 = 33;                  // left button
  static const uint8_t BUTTON2 = IOExpander::BUTTON2; // middle button
  static const uint8_t BUTTON3 = 34;                  // right button
  static const uint8_t IR_LEDS = 12;                  // the IR LED pins
#elif UNPHONE_SPIN == 7
  static const uint8_t BUTTON1 = 33;                  // left button
  static const uint8_t BUTTON2 = 35;                  // middle button
  static const uint8_t BUTTON3 = 34;                  // right button
  static const uint8_t IR_LEDS = 13;                  // the IR LED pins
  static const uint8_t EXPANDER_POWER = 2;            // enable exp when high
#elif UNPHONE_SPIN == 8
  static const uint8_t BUTTON1 = 16;                  // left button
  static const uint8_t BUTTON2 = 7;                   // middle button
  static const uint8_t BUTTON3 = 6;                   // right button
  static const uint8_t IR_LEDS = 9;                   // the IR LED pins
  static const uint8_t EXPANDER_POWER = 48;           // enable exp when high
#elif UNPHONE_SPIN == 9
  static const uint8_t BUTTON1 = 45;                  // left button
  static const uint8_t BUTTON2 = 0;                   // middle button
  static const uint8_t BUTTON3 = 21;                  // right button
  static const uint8_t IR_LEDS = 13;                  // the IR LED pins
  static const uint8_t EXPANDER_POWER = 48;           // TODO move to IOExp; enable exp when high
#endif
  static bool button1();                              // register...
  static bool button2();                              // ...button...
  static bool button3();                              // ...presses

  static void vibe(bool);             // vibe motor on or off
  static bool checkPowerSwitch();     // if power switch is off shutdown
  static void printWakeupReason();    // what woke us up?
  static void recoverI2C();           // deal with i2c hangs

  // the touch screen, display and accelerometer ////////////////////////////
  static Adafruit_HX8357 *tftp;
#if UNPHONE_SPIN == 6
  static Adafruit_STMPE610 *tsp;
  static Adafruit_LSM303_Accel_Unified *accelp;
#elif UNPHONE_SPIN >= 7
  static XPT2046_Touchscreen *tsp;
  static Adafruit_LSM9DS1 *accelp;
#endif
  static void getAccelEvent(sensors_event_t *); // spin-agnostic accelerometer
#if UNPHONE_SPIN == 8
  static const uint8_t LCD_DC =  42;
#elif UNPHONE_SPIN == 9
  static const uint8_t LCD_DC =  47;  // from spin 6, previously IOExp 7
#else
  static const uint8_t LCD_DC =  21;
#endif
  static void backlight(bool);        // turn the backlight on or off
  static void expanderPower(bool);    // turn expander board power on or off

  // SD card filesystem
//  static SdFat *sdp;
  static void *sdp;

  // calibration data for converting raw touch data to screen coordinates
#if UNPHONE_SPIN == 6
  static const uint16_t TS_MINX = 3800;
  static const uint16_t TS_MAXX =  100;
  static const uint16_t TS_MINY =  100;
  static const uint16_t TS_MAXY = 3750;
#elif UNPHONE_SPIN >= 7
  static const uint16_t TS_MINX =  300;
  static const uint16_t TS_MAXX = 3800;
  static const uint16_t TS_MINY =  500;
  static const uint16_t TS_MAXY = 3750;
#endif

  // the RGB LED
  static void rgb(uint8_t red, uint8_t green, uint8_t blue);

  // LoRa radio
  static void loraSetup();              // init the LoRa board
  static void loraLoop();               // service lora transactions
  static void loraSend(const char *);   // send (TTN) LoRaWAN message
#if UNPHONE_SPIN <= 7
  static const uint8_t LMIC_DIO0 = 39;
  static const uint8_t LMIC_DIO1 = 26;
#else
  static const uint8_t LMIC_DIO0 =  5;
  static const uint8_t LMIC_DIO1 = 18;
#endif

  // power management chip API
  // TODO simplify, add USB_VSENSE
#if UNPHONE_SPIN == 6
  static const uint8_t VBAT_SENSE = 35; // battery voltage pin
#else
  static const uint8_t VBAT_SENSE = 7;  // battery voltage pin
#endif
  static float batteryVoltage(); // get the battery voltage
  static const byte BM_I2Cadd;   // the chip lives here on I²C
  static const byte BM_Watchdog; // charge termination/timer control register
  static const byte BM_OpCon;    // misc operation control register
  static const byte BM_Status;   // system status register
  static const byte BM_Version;  // vender / part / revision status register
  static void setShipping(bool value); // tells BM chip to shut down
  static void setRegister(byte address, byte reg, byte value); //
  static byte getRegister(byte address, byte reg);             // I²C...
  static void write8(byte address, byte reg, byte value);      // ...helpers
  static byte read8(byte address, byte reg);                   //

  static char *getMAC(char *buf); // MAC address
}; // class unPhone

extern char MAC_ADDRESS[13]; // MACs are 12 chars, plus the NULL terminator

// macros for debug (and error) calls to Serial.printf, with (D) and without
// (DD) new line, and to Serial.println (DDD) TODO combine with next
#ifdef UNPHONE_PRODUCTION_BUILD
# define D(args...)
# define E(args...)
#else
extern bool UNPHONE_DBG;     // debug switch
# define D_ON  UNPHONE_DBG = 1;
# define DD(args...)  if(UNPHONE_DBG) Serial.printf(args);
# define D_OFF UNPHONE_DBG = 0;
# define D(args...) if(UNPHONE_DBG) printf(args);
# define E(args...) printf(args);
#endif

// debugging infrastructure; setting different DBGs true triggers prints ////
#define dbg(b, s)       if(b) Serial.print(s)
#define dbf(b, ...)     if(b) Serial.printf(__VA_ARGS__)
#define dln(b, s)       if(b) Serial.println(s)
#define startupDBG      true
#define loopDBG         true
#define monitorDBG      true
#define netDBG          true
#define miscDBG         true
#define analogDBG       true
#define otaDBG          true
#define touchDBG        false
static const char *TAG = "MAIN";        // ESP logger debug tag

// delay/yield/timing and time-slicing macros
#define WAIT_A_SEC   vTaskDelay(    1000/portTICK_PERIOD_MS); // 1 second
#define WAIT_SECS(n) vTaskDelay((n*1000)/portTICK_PERIOD_MS); // n seconds
#define WAIT_MS(n)   vTaskDelay(       n/portTICK_PERIOD_MS); // n millis
extern int loopIter; // main task loop iteration counter

#endif
