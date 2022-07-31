// TouchpaintUIElement.cpp

#include "AllUIElement.h"

/**
 * 
 */
void TouchpaintUIElement::draw(){
  m_tft->fillScreen(BLACK);
  drawSelector();
  drawSwitcher();
}

/**
 * 
 */
void TouchpaintUIElement::runEachTurn(){
  // Do nothing 
}

/**
 * selection for point inside menu
 */
void TouchpaintUIElement::colourSelector(long x, long y) {
  oldcolour = currentcolour;

  uint16_t boxXOrigin;
  for(uint8_t i = 0; i < NUM_BOXES; i++) { // white border around selection
    boxXOrigin = i * BOXSIZE;
    if(x < ((i + 1) * BOXSIZE) ) {
      currentcolour = colour2box[i];
      if(i == 6) // white box: red border
        m_tft->drawRect(boxXOrigin, 0, BOXSIZE, BOXSIZE, RED);
      else       // white border
        m_tft->drawRect(boxXOrigin, 0, BOXSIZE, BOXSIZE, WHITE);
      break;
    }
  }

  if(oldcolour != currentcolour) { // refill previous selection (etc.)
    drawSelector();
    D("changed colour to %d\n", currentcolour)
  }
}

/**
 * 
 */
void TouchpaintUIElement::drawSelector() { 
  for(uint8_t i = 0; i < NUM_BOXES; i++) {
    m_tft->fillRect(i * BOXSIZE, 0, BOXSIZE, BOXSIZE, colour2box[i]);
  }
}

/**
 * 
 */
bool TouchpaintUIElement::handleTouch(long x, long y) {
  if(y < BOXSIZE && x > (BOXSIZE * SWITCHER)){
    return true;
  }

  if(y < BOXSIZE) { // we're in the control area
    D("in control area, calling selectColour\n")
    colourSelector(x, y);
  } else if(((y-PENRADIUS) > 0) && ((y+PENRADIUS) < m_tft->height())) {
    D("in drawing area, calling fillCircle\n")
    m_tft->fillCircle(x, y, PENRADIUS, currentcolour);
  }
  
  return false;
}
