// unphone.cpp
// core library

#include "unphone.h"

bool UNPHONE_DBG = true; // debug switch

// the LCD and touch screen //////////////////////////////////////////////////
Adafruit_HX8357 *unPhone::tftp;
#if UNPHONE_SPIN == 6
Adafruit_STMPE610 *unPhone::tsp;
#elif UNPHONE_SPIN >= 7
XPT2046_Touchscreen *unPhone::tsp;
#endif
void unPhone::backlight(bool on) { // turn the backlight on or off
  if(on) digitalWrite(IOExpander::BACKLIGHT, HIGH);
  else   digitalWrite(IOExpander::BACKLIGHT, LOW);
}
void unPhone::expanderPower(bool on) { // expander board power on or off
#if UNPHONE_SPIN == 6
  return;
#elif UNPHONE_SPIN >= 7
  if(on) digitalWrite(EXPANDER_POWER, HIGH);
  else   digitalWrite(EXPANDER_POWER, LOW);
#endif
}

// SD card filesystem
void *unPhone::sdp;

// the accelerometer //////////////////////////////////////////////////////
#if UNPHONE_SPIN == 6
Adafruit_LSM303_Accel_Unified *unPhone::accelp;
#elif UNPHONE_SPIN >= 7
Adafruit_LSM9DS1 *unPhone::accelp;
#endif

// initialise unPhone hardware
void unPhone::begin() {
  Serial.begin(115200); // init the serial line
  D("UNPHONE_SPIN: %d\n", UNPHONE_SPIN);

  // fire up I²C, and the unPhone's IOExpander library
  recoverI2C();
  Wire.begin();
  Wire.setClock(400000); // rates > 100k used to trigger an IOExpander bug...?
  IOExpander::begin();

  // instantiate the display...
  tftp = new Adafruit_HX8357( // spin5 moves LCD_DC to 21
    IOExpander::LCD_CS, LCD_DC, IOExpander::LCD_RESET
  );
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);
  tftp->begin(HX8357D);
  IOExpander::digitalWrite(IOExpander::BACKLIGHT, HIGH);
  tftp->setTextWrap(false);
  // TODO clear screen to start with: tftp->fillScreen(HX8357_BLACK);

  // ...and the touch screen
#if UNPHONE_SPIN == 6
  tsp = new Adafruit_STMPE610(IOExpander::TOUCH_CS);
#elif UNPHONE_SPIN >= 7
  tsp = new XPT2046_Touchscreen(IOExpander::TOUCH_CS); // no IRQ
#endif
  bool status = tsp->begin();
  if(!status) {
    E("failed to start touchscreen controller\n");
  } else {
    D("worked\n");
  }

  // init the SD card
  // see Adafruit_ImageReader/examples/FeatherWingHX8357/FeatherWingHX8357.ino
/*
  sdp = new SdFat();
  if(!sdp->begin(IOExpander::SD_CS, SD_SCK_MHZ(25))) { // ESP32 25 MHz limit
    E("sdp->begin failed\n");
  } else {
    D("sdp->begin OK\n");
  }
*/

  // init touch screen GPIOs (used for vibe motor)
#if UNPHONE_SPIN == 6
  tsp->writeRegister8(0x17, 12); // use GPIO 2 & 3 (bcd)
  tsp->writeRegister8(0x13, 12); // all gpios as out, minimise power on unused
  tsp->writeRegister8(0x11, 8);  // set GPIO3 LOW to stop vibe
#elif UNPHONE_SPIN >= 7
  IOExpander::digitalWrite(IOExpander::VIBE, LOW);
#endif
  // initialise the buttons
#if UNPHONE_SPIN == 6
  Serial.printf("setting b2 %d to INPUT\n", BUTTON2);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT_PULLUP);
#elif UNPHONE_SPIN >= 7
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
// #elif UNPHONE_SPIN == 8
//   D("setting b3 %d to INPUT\n", BUTTON3);
//   pinMode(BUTTON1, INPUT_PULLUP);
//   pinMode(BUTTON2, INPUT_PULLUP);
//   pinMode(BUTTON3, INPUT);
#endif
  D("pinmodes done\n")
  D("board spin = %u\n", IOExpander::getVersionNumber())

  // expander power control, available from spin 7
#if UNPHONE_SPIN >= 7
  pinMode(EXPANDER_POWER, OUTPUT);
  // TODO this will default LOW, i.e. off, but let's make this explicit:
  // expanderPower(false);
#endif

  // the accelerometer
#if UNPHONE_SPIN == 6
  accelp = new Adafruit_LSM303_Accel_Unified(54321); // with a "unique" ID
  if(!accelp->begin())                               // problem detecting the sensor?
    E("oops, no LSM303 detected ... check your wiring?!\n")
  else
    D("accelp->begin OK\n")
#elif UNPHONE_SPIN >= 7
  accelp = new Adafruit_LSM9DS1(); // on i2c
  if (!accelp->begin()) // problem detecting the sensor?
    E("oops, no LSM9DS1 detected ... check your wiring?!\n")
  else
    D("accelp->begin OK\n")
#endif

  // set up IR_LED pin
  pinMode(IR_LEDS, OUTPUT);
} // begin()

void unPhone::vibe(bool on) {
#if UNPHONE_SPIN == 6
  if(on)
    tsp->writeRegister8(0x10, 8);   // set GPIO3 HIGH to start vibe
  else
    tsp->writeRegister8(0x11, 8); // set GPIO3 LOW to stop vibe
#elif UNPHONE_SPIN >= 7
  if (on)
    IOExpander::digitalWrite(IOExpander::VIBE, HIGH);
  else
    IOExpander::digitalWrite(IOExpander::VIBE, LOW);
#endif
}

void unPhone::rgb(uint8_t red, uint8_t green, uint8_t blue) {
  digitalWrite(IOExpander::LED_RED, red);
  digitalWrite(IOExpander::LED_GREEN, green);
  digitalWrite(IOExpander::LED_BLUE, blue);
}

bool unPhone::button1() { return digitalRead(unPhone::BUTTON1) == LOW; }
bool unPhone::button2() { return digitalRead(unPhone::BUTTON2) == LOW; }
bool unPhone::button3() { return digitalRead(unPhone::BUTTON3) == LOW; }

// get a (spin-agnostic) accelerometer reading
void unPhone::getAccelEvent(sensors_event_t *eventp)
{
/*
for UNPHONE_SPIN >= 9 on LSM6DS3TRC
https://github.com/adafruit/Adafruit_LSM6DS/blob/master/Adafruit_LSM6DS3TRC.h
something like:
  sensors_event_t gyro, temp;
  accelp->getEvent(eventp, &gyro, &temp);
(but check how to access the gyro/temp data, do we need to make that global?)
*/
#if UNPHONE_SPIN >= 7
  sensors_event_t m, g, temp;
  accelp->getEvent(eventp, &m, &g, &temp);
#else
  accelp->getEvent(eventp);
#endif
}

// try to recover I2C bus in case it's locked up...
// NOTE: only do this in setup **BEFORE** Wire.begin!
void unPhone::recoverI2C() {
  pinMode(SCL, OUTPUT);
  pinMode(SDA, OUTPUT);
  digitalWrite(SDA, HIGH);

  for(int i = 0; i < 10; i++) { // 9th cycle acts as NACK
    digitalWrite(SCL, HIGH);
    delayMicroseconds(5);
    digitalWrite(SCL, LOW);
    delayMicroseconds(5);
  }

  // a STOP signal (SDA from low to high while SCL is high)
  digitalWrite(SDA, LOW);
  delayMicroseconds(5);
  digitalWrite(SCL, HIGH);
  delayMicroseconds(2);
  digitalWrite(SDA, HIGH);
  delayMicroseconds(2);

  // I2C bus should be free now... a short delay to help things settle
  delay(200);
}

// power management chip API /////////////////////////////////////////////////
const byte unPhone::BM_I2Cadd   = 0x6b; // the chip lives here on I²C
const byte unPhone::BM_Watchdog = 0x05; // charge termination/timer cntrl reg
const byte unPhone::BM_OpCon    = 0x07; // misc operation control register
const byte unPhone::BM_Status   = 0x08; // system status register
const byte unPhone::BM_Version  = 0x0a; // vender / part / revision status reg

float unPhone::batteryVoltage() { // get the battery voltage
  float voltage = -1;
  // in spin7 VBAT_SENSE isn't connected so we use XPT2046
#if UNPHONE_SPIN == 6
  // TODO most of this should be in begin()?
  // set up the ADC and do the read
  pinMode(VBAT_SENSE, INPUT);
  analogReadResolution(12); // 10 bit = 0-1023, 11 = 0-2047, 12 = 0-4095
  analogSetPinAttenuation(
    VBAT_SENSE,
    ADC_6db // 0db is 0-1V, 2.5db is 0-1.5V, 6db is 0-2.2v, 11db is 0-3.3v
  );
  voltage = analogRead(VBAT_SENSE);
  voltage = (voltage / 4095) * 4.4;
#elif UNPHONE_SPIN >= 7
  voltage = tsp->getVBat();
#endif
  return voltage;
}

RTC_DATA_ATTR int bootCount = 0; // how many times we booted (since reset)

void unPhone::printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by external signal using RTC_CNTL");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("Wakeup caused by ULP program");
      break;
    default:
      Serial.printf("Wakeup not caused by deep sleep: %d\n",wakeup_reason);
      break;
  }
}

// check for power off states and do BM shipping mode (when on bat) or ESP
// deep sleep (when on USB 5V)
bool unPhone::checkPowerSwitch() {
// TODO this would be better as an interrupt, driven off GPIO_NUM36 (the
// unphone power switch slider)

  // what is the state of the power switch? (non-zero = on, which is
  // physically slid away from the USB socket)
  uint8_t inputPwrSw = IOExpander::digitalRead(IOExpander::POWER_SWITCH);

  bool usbPowerOn = // bit 2 of status register indicates if USB connected
    bitRead(getRegister(BM_I2Cadd, BM_Status), 2);

  if(!inputPwrSw) {  // when power switch off
    if(!usbPowerOn) { // and usb unplugged we go into shipping mode
      // tell BM to stop supplying power (from the battery)
      D("settign shipping mode true");
      setShipping(true);
    } else { // power switch off and usb plugged in we sleep
      D("enabling wakeup on 36/4 and doing shippig and deep_sleep")
#if UNPHONE_SPIN == 6 || UNPHONE_SPIN == 7
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0); // 1 = High, 0 = Low
#elif UNPHONE_SPIN >= 8
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0); // 1 = High, 0 = Low
#endif

      // cludge: LCD (and other peripherals) will still be powered when we're
      // on USB; the next call turns the LCD backlight off, but would be
      // preferable if we could cut the 5V to all but the BM (which needs it
      // for charging)...?
      IOExpander::digitalWrite(IOExpander::BACKLIGHT, LOW);

      // TODO turn off expander power
      expanderPower(false);

      // TODO LMIC_shutdown

      // deep sleep, wait for wakeup on GPIO
      esp_deep_sleep_start();
    }
  }

  return usbPowerOn;
}

// ask BM chip to shutdown or start up
void unPhone::setShipping(bool value) {
  byte result;
  if(value) {
    result=getRegister(BM_I2Cadd, BM_Watchdog);  // state of timing register
    bitClear(result, 5);                         // clear bit 5...
    bitClear(result, 4);                         // and bit 4 to disable...
    setRegister(BM_I2Cadd, BM_Watchdog, result); // WDT (REG05[5:4] = 00)

    result=getRegister(BM_I2Cadd, BM_OpCon);     // operational register
    bitSet(result, 5);                           // set bit 5 to disable...
    setRegister(BM_I2Cadd, BM_OpCon, result);    // BATFET (REG07[5] = 1)
  } else {
    result=getRegister(BM_I2Cadd, BM_Watchdog);  // state of timing register
    bitClear(result, 5);                         // clear bit 5...
    bitSet(result, 4);                           // and set bit 4 to enable...
    setRegister(BM_I2Cadd, BM_Watchdog, result); // WDT (REG05[5:4] = 01)

    result=getRegister(BM_I2Cadd, BM_OpCon);     // operational register
    bitClear(result, 5);                         // clear bit 5 to enable...
    setRegister(BM_I2Cadd, BM_OpCon, result);    // BATFET (REG07[5] = 0)
  }
}

// I2C helpers to drive the power management chip
void unPhone::setRegister(byte address, byte reg, byte value) {
  write8(address, reg, value);
}
byte unPhone::getRegister(byte address, byte reg) {
  byte result;
  result=read8(address, reg);
  return result;
}
void unPhone::write8(byte address, byte reg, byte value) {
  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)value);
  Wire.endTransmission();
}
byte unPhone::read8(byte address, byte reg) {
  byte value;
  Wire.beginTransmission(address);
  Wire.write((uint8_t)reg);
  Wire.endTransmission();
  Wire.requestFrom(address, (byte)1);
  value = Wire.read();
  Wire.endTransmission();
  return value;
}


// the LoRa board and TTN LoRaWAN ///////////////////////////////////////////
/*
void unPhone::loraSetup() { lora_setup(); }     // init the LoRa board
void unPhone::loraLoop() { lora_loop(); }       // service LoRa transactions
void unPhone::loraSend(const char *payload) { lora_send(payload); } // send
*/


// get the ESP's MAC address ///////////////////////////////////////////////
char MAC_ADDRESS[13]; // MAC addresses are 12 chars, plus the NULL terminator
char *unPhone::getMAC(char *buf) { // the MAC is 6 bytes, so needs careful conversion...
  uint64_t mac = ESP.getEfuseMac(); // ...to string (high 2, low 4):
  char rev[13];
  sprintf(rev, "%04X%08X", (uint16_t) (mac >> 32), (uint32_t) mac);

  // the byte order in the ESP has to be reversed relative to normal Arduino
  for(int i=0, j=11; i<=10; i+=2, j-=2) {
    buf[i] = rev[j - 1];
    buf[i + 1] = rev[j];
  }
  buf[12] = '\0';
  return buf;
}
