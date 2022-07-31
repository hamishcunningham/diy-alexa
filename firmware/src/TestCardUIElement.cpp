// TestCardUIElement.cpp

#include "AllUIElement.h"
#include <Adafruit_ImageReader.h> // image-reading functions

//////////////////////////////////////////////////////////////////////////
/**
 * Function that handles the touch on this page
 * 
 * @param x - the x coordinate of the touch 
 * @param y - the y coordinate of the touch 
 * @returns bool - true if the touch is on the switcher
 */
bool TestCardUIElement::handleTouch(long x, long y) {
  return y < BOXSIZE && x > (BOXSIZE * SWITCHER);
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Function that controls the drawing on the test page
 */
void TestCardUIElement::draw(){
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(2);

  drawBBC();
  drawTestcard();

  WAIT_A_SEC;

  drawSwitcher();
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Test page function that runs each turn 
 */
void TestCardUIElement::runEachTurn(){
  // do nothing 
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * draws the bbc test image
 */
void TestCardUIElement::drawBBC() { 
/*
  Adafruit_ImageReader reader(*m_sd); // Image-reader object, pass in SD filesys

  // draw bmp
  Adafruit_Image       img;        // an image loaded into RAM
  ImageReturnCode      stat;       // status from image-reading functions
  stat = reader.drawBMP("/testcard.bmp", *m_tft, 0, 0); // draw it
  reader.printStatus(stat);        // how'd we do?

  m_tft->setTextSize(2);
  m_tft->setTextColor(BLUE);
  m_tft->setCursor(10, 360); m_tft->print("please wait"); WAIT_MS(100)
  m_tft->setCursor(10, 340); m_tft->print("Winding up elastic band:");
  WAIT_MS(600)
  for(int i = 0; i<12; i++) {
    m_tft->setCursor(150 + (i * 5), 360); m_tft->print(".");
    WAIT_MS(300)
  }
  WAIT_MS(300) WAIT_MS(300)
*/
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * draw stuff to make screen dimensions obvious
 */
void TestCardUIElement::drawTestcard() {
  m_tft->fillScreen(BLACK);
  m_tft->setTextColor(GREEN);
  m_tft->setTextSize(1);
  m_tft->fillCircle(160, 240 + 100, 1, RED);
  m_tft->setCursor(160 + 8, 340);
  m_tft->setTextColor(RED);
  m_tft->print("X:160");

  m_tft->fillCircle(160 + 100, 240, 1, CYAN);
  m_tft->setCursor(260 + 8, 240);
  m_tft->setTextColor(CYAN);
  m_tft->print("Y:240");

  // (we're 320 wide and 480 tall in portrait mode)
  //            X    Y    W    H
  m_tft->drawRect(  0,   0,  60,  60, GREEN);
  m_tft->drawRect(130, 215,  60,  60, GREEN);
  m_tft->drawRect(260, 420,  60,  60, GREEN);
  m_tft->fillTriangle(5, 443, 10, 433, 15, 443, MAGENTA);
  m_tft->fillRoundRect(153, 435, 20, 10, 4, RED);
  m_tft->fillRect(295, 435, 10, 10, MAGENTA);

  // TODO the below causes a hang (bad radius?); use to set up task WDT
  // registration of this task with recovery?
  // m_tft->fillRoundRect(150, 430, 20, 10, 10, RED);

  // label the green boxes
  m_tft->setTextColor(BLUE);
  m_tft->setCursor(3, 50); m_tft->print("60x60:0,0");
  m_tft->setCursor(130, 277); m_tft->print("60x60:130,215");
  m_tft->setCursor(241, 410); m_tft->print("60x60:260,420");

  // significant positions, text
  m_tft->setTextSize(3);
  m_tft->setTextColor(YELLOW);
  m_tft->setCursor(135,  10); m_tft->print("the");
  m_tft->setCursor(135,  45); m_tft->print("test");
  m_tft->setCursor(135,  80); m_tft->print("card");
  m_tft->setTextSize(2);
  m_tft->setTextColor(WHITE);
  m_tft->setCursor(  0,   0);   m_tft->print("0,0");
  m_tft->setCursor(  0, 300);   m_tft->print("0,300");
  // m_tft->setCursor(260,   0); m_tft->print("260,0"); (obscured by switcher box)
  m_tft->setCursor(248,  60); m_tft->print("248,60");
  m_tft->setCursor(100, 300); m_tft->print("100,300");
  m_tft->setCursor(150, 240); m_tft->print("150,240");
  m_tft->setCursor(200, 300); m_tft->print("200,300");
  m_tft->setCursor(235, 465); m_tft->print("235,465");
  m_tft->setCursor(  0, 465);   m_tft->print("0,465");

  m_tft->drawFastHLine(150, 160, 120, MAGENTA);
  m_tft->drawFastVLine(270, 160,  50, MAGENTA);
  m_tft->drawLine(150, 160, 270, 210, MAGENTA);
  m_tft->fillCircle(230, 180, 5, CYAN);

  // horizontal and vertical counters
  m_tft->setTextColor(GREEN);
  for(int i=0, j=1; i<=460; i+=20, j++) {
    if(j == 7) continue;
    m_tft->setCursor(70, i);
    if(j < 10) m_tft->print(" ");
    if(i == 460) m_tft->setTextColor(GREEN);
    m_tft->print(j);
    WAIT_MS(50)
    if(i == 460) {
      m_tft->setCursor(98, 467);
      m_tft->setTextSize(1);
      m_tft->print("(X:70, Y:");
      m_tft->print(i);
      m_tft->print(")");
      m_tft->setTextSize(2);
    }
  }
  m_tft->setCursor(0, 120);
  m_tft->setTextColor(GREEN);
  m_tft->print("01234567890123456789012345");
}
/////////////////////////////////////////////////////////////////////////
