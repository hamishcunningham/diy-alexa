// MenuUIElement.cpp

#include "AllUIElement.h"

// handle touch on this page
// @returns true if the touch is a menu item
bool MenuUIElement::handleTouch(long x, long y) {
  // D("text mode: responding to touch @ %d/%d/%d: ", x, y,-1)
  m_tft->setTextColor(WHITE, BLACK);
  uint8_t menuItem = mapTextTouch(x, y);
  D("menuItem=%d, ", menuItem)

  if(menuItem > 0 && menuItem <= NUM_UI_ELEMENTS) {
    menuItemSelected = menuItem;
    return true;
  }
  return false;
}

// returns menu item number //////////////////////////////////////////////
uint8_t MenuUIElement::mapTextTouch(long xInput, long yInput) {
  for(int y = 30, i = 1; y < 480; y += 48, i++)
    if(xInput > 270 && yInput > y && yInput < y + 48)
      return i;
  return -1;
}

// draw a textual menu ///////////////////////////////////////////////////
void MenuUIElement::draw(){
  m_tft->setTextSize(2);
  m_tft->setTextColor(BLUE);

  m_tft->setCursor(230, 0);
  m_tft->print("MENU");

  uint16_t yCursor = 30;
  m_tft->drawFastHLine(0, yCursor, 320, MAGENTA);
  yCursor += 16;

  for(int i = 1; i < NUM_UI_ELEMENTS; i++) {
    m_tft->setCursor(0, yCursor);
    m_tft->print(ui_mode_names[i]);
    drawSwitcher(288, yCursor - 12);
    yCursor += 32;
    m_tft->drawFastHLine(0, yCursor, 320, MAGENTA);
    yCursor += 16;
  }
}

//////////////////////////////////////////////////////////////////////////
void MenuUIElement::runEachTurn(){ // text page UI, run each turn
  // do nothing
}
