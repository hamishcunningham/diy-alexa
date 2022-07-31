// EtchASketchUIElement.cpp

#include "AllUIElement.h"

// x and y coords of the etching pen
int penx = 160; // put the pen in the ...
int peny = 240; // ... middle of the screen

/**
 * Show initial screen.
 */
void EtchASketchUIElement::draw(){
  m_tft->fillScreen(BLACK);
  m_tft->drawLine(0, 0, 319, 0, BLUE);
  m_tft->drawLine(319, 0, 319, 479, BLUE);
  m_tft->drawLine(319, 479, 0, 479, BLUE);
  m_tft->drawLine(0, 479, 0, 0, BLUE);
  drawSwitcher();
}

/**
 * Check the accelerometer, adjust the pen coords and draw a point.
 */
void EtchASketchUIElement::runEachTurn(){
  // get a new sensor event
  sensors_event_t event;
  unPhone::getAccelEvent(&event);

#if UNPHONE_SPIN >= 7
  if(event.acceleration.x > 2 & penx < 318)
    penx = penx +1;
  if(event.acceleration.x < -2 & penx > 1)
    penx = penx -1;
  if(event.acceleration.y > 2 & peny < 478)
    peny = peny +1;
  if(event.acceleration.y < -2 & peny > 1)
    peny = peny -1;
#else
  if(event.acceleration.y > 2 & penx < 318)
    penx = penx +1;
  if(event.acceleration.y < -2 & penx > 1)
    penx = penx -1;
  if(event.acceleration.x > 2 & peny < 478)
    peny = peny +1;
  if(event.acceleration.x < -2 & peny > 1)
    peny = peny -1;
#endif

  // draw
  m_tft->drawPixel(penx, peny, HX8357_GREEN);

  // display the results (acceleration is measured in m/s^2)
  /*
  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");
  Serial.println("m/s^2 ");
  */

  // delay before the next sample
  delay(20);
}

/**
 * Switch to menu if required.
 */
bool EtchASketchUIElement::handleTouch(long x, long y) {
  if(y < BOXSIZE && x > (BOXSIZE * SWITCHER))
    return true;
  return false;
}
