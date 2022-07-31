// IOExpander.cpp

#include "Arduino.h"
#include "Wire.h"

#include "IOExpander.h"

uint16_t IOExpander::directions = 0x00;
uint16_t IOExpander::output_states = 0x00;

void IOExpander::begin() {
  // setup the IO expander intially as all inputs so we 
  // don't accidentally drive anything until it's asked for
  // this is done by writing all ones to the two config registers
  IOExpander::writeRegisterWord(0x06, 0xFFFF);

  // read the current port directions and output states
  IOExpander::directions = IOExpander::readRegisterWord(0x06);
  IOExpander::output_states = IOExpander::readRegisterWord(0x02);
}

void IOExpander::pinMode(uint8_t pin, uint8_t mode) {
  if(pin & 0x40) {
    pin &= 0b10111111;  // mask out the high bit
    
    uint16_t new_directions = IOExpander::directions;    
    if(mode==OUTPUT){
      new_directions &= ~(1UL << pin);
    } else {
      new_directions |= (1UL << pin);
    }
    if(new_directions != IOExpander::directions) {      
      IOExpander::writeRegisterWord(0x06, new_directions);
      IOExpander::directions = new_directions;
    }
  } else {    
    ::pinMode(pin, mode);
  }
}

void IOExpander::digitalWrite(uint8_t pin, uint8_t value) {
  if(pin & 0x40) {
    pin &= 0b10111111;  // mask out the high bit

    // set the output state
    uint16_t new_output_states = IOExpander::output_states;
    if(value == HIGH) {
      new_output_states |=  (1UL << pin);
    } else {
      new_output_states &= ~(1UL << pin);
    }
    if(new_output_states != IOExpander::output_states) {
      IOExpander::writeRegisterWord(0x02, new_output_states);
      IOExpander::output_states = new_output_states;
    }
  
    // set the pin direction to output
    uint16_t new_directions = IOExpander::directions;
    new_directions &= ~(1UL << pin);
    if(new_directions != IOExpander::directions) {
      IOExpander::writeRegisterWord(0x06, new_directions);
      IOExpander::directions = new_directions;
    }     
  } else {
    ::digitalWrite(pin, value);
  }
}  

uint8_t IOExpander::digitalRead(uint8_t pin) {
  if(pin & 0x40) {
    pin &= 0b10111111;

    // set the pin direction to input
    uint16_t new_directions = IOExpander::directions;
    new_directions |= (1UL << pin);
    if(new_directions != IOExpander::directions) {
      IOExpander::writeRegisterWord(0x06, new_directions);
      IOExpander::directions = new_directions;
    }          

    // read the input register inverted
    uint16_t inputs = IOExpander::readRegisterWord(0x00);
    return (inputs & (1UL << pin)) ? HIGH : LOW;
  } else {
    return ::digitalRead(pin);
  }
}

uint8_t IOExpander::getVersionNumber() {
#if UNPHONE_SPIN == 6
  uint8_t v1 = IOExpander::digitalRead(IOExpander::VERSION_1) ? 1 : 0;
  uint8_t v2 = IOExpander::digitalRead(IOExpander::VERSION_2) ? 1 : 0;
  uint8_t v3 = IOExpander::digitalRead(IOExpander::VERSION_3) ? 1 : 0;

  return 11 - ((v3 << 2) | (v2 << 1) | (v1 << 0));
#else
  return UNPHONE_SPIN;
#endif
}

uint16_t IOExpander::readRegisterWord(uint8_t reg) {
  Wire.beginTransmission(IOExpander::i2c_address);
  Wire.write(reg);
  Wire.endTransmission();  
  Wire.requestFrom(IOExpander::i2c_address, (uint8_t)(2));
  return Wire.read() | (Wire.read() << 8);
}
  
void IOExpander::writeRegisterWord(uint8_t reg, uint16_t value) {
  Wire.beginTransmission(IOExpander::i2c_address);
  Wire.write(reg);      
  Wire.write(value);
  Wire.write(value >> 8);
  Wire.endTransmission();  
}
