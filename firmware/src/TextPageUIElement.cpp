// TextPageUIElement.cpp

#include "AllUIElement.h"

/////////////////////////////////////////////////////////////////////////
// predictive text, and text input history //////////////////////////////
Predictor predictor;
class TextHistory {
public:
  static const uint8_t SIZE = 20; // set to 3 for testing
  char *buf[SIZE];    // the history buffer
  uint8_t cursor;     // next insertion point
  uint8_t start;      // first in the sequence
  int8_t iter;        // iterator index
  uint8_t members;    // number of stored values

  TextHistory() { clear(); }
  void store(const char *word);
  void remove();
  bool full() { return members == SIZE; }
  void clear();
  const char *first();
  const char *next();
  void debug();
  void test();
  uint8_t size() { return members; }
};
TextHistory textHistory;

//////////////////////////////////////////////////////////////////////////
/**
 * Function that handles the touch on this page
 * 
 * @param x - the x coordinate of the touch 
 * @param y - the y coordinate of the touch 
 * @returns bool - true if the touch is on the switcher
 */
bool TextPageUIElement::handleTouch(long x, long y) {
  m_tft->setTextColor(WHITE, BLACK);
    // D("text mode: responding to touch @ %d/%d/%d: ", x, y,-1)

    uint8_t symbol = mapTextTouch(x, y);
    D("sym=%d, ", symbol)

    if(symbol == 0) { // "ok"
      D("accepting\n")
      textHistory.store(predictor.first()); // textHistory.debug();
      predictor.reset();
      printHistory(0, 0);
    } else if(symbol >= 1 && symbol <= 8) { // next char
      D("suggesting for %c\n", ((symbol + 1) + '0'));
      if(predictor.suggest(symbol + 1) >= 0) {
        m_tft->setCursor(0, 80);
        int charsPrinted = 0;
        const char *cp = NULL;
        while( (cp = predictor.next()) != NULL ) {
          m_tft->print(cp);
          m_tft->print(" ");
          charsPrinted += strlen(cp) + 1;
        }
        for( ; charsPrinted < 100; charsPrinted++)
          m_tft->print(" ");
      }
    } else if(symbol ==  9) { // delete
      D("calling tH.remove(), (%d)\n", symbol);
      textHistory.remove(); // textHistory.debug();
      printHistory(0, 0);
    } else if(symbol == 10) { // "  _" / ?2
      // TODO
    } else if(symbol == 11) { // mode switcher arrow
      return true;
    }
  return false;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
void TextPageUIElement::printHistory(uint16_t x, uint16_t y) { 
  int charsPrinted = 0;
  const char *cp = NULL;
  m_tft->setCursor(x, y);
  for(cp = textHistory.first(); cp; cp = textHistory.next()) {
    m_tft->print(cp);
    m_tft->print(" ");
    charsPrinted += strlen(cp) + 1;
  }
  for( ; charsPrinted < 200; charsPrinted++)
    m_tft->print(" ");
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
uint16_t debugCursor = 0; // TODO
uint8_t TextPageUIElement::mapTextTouch(long xInput, long yInput) { //////////////////////////////////////
  int8_t sym = -1; // symbol
  int8_t row = 0;  // rows (0 is above the text entry area)
  int8_t col = 1;  // columns

  for(int y = 160, i = 1; y < 480; y += 80, i++)
    if(yInput > y) row = i;
  for(int x = 107, i = 2; x < 480; x += 107, i++)
    if(xInput > x) col = i;
  if(row > 0 && col >= 1)
    sym = (( (row - 1) * 3) + col) - 1;

  // D("row=%d, col=%d, sym=%d\n", row, col, sym)

  // TODO
  m_tft->setTextColor(WHITE, BLACK);
  char s[5];
  sprintf(s, "%2d,", sym);
  m_tft->setCursor(debugCursor,150);
  debugCursor += 3;
  if(debugCursor > 280) debugCursor = 0;
  // TODO uncomment for debug sym printing: m_tft->print(s);
  return sym;
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * 
 */
void TextPageUIElement::draw(){
  drawTextBoxes();
  drawSwitcher(255, 420);
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Divide up the screen into areas and creates the keyboard
 */
void TextPageUIElement::drawTextBoxes() {
  for(int y = 160; y < 480; y += 80)
    m_tft->drawFastHLine(0, y, 320, MAGENTA);
  m_tft->drawFastHLine(0, 479, 320, MAGENTA);
  for(int x = 0; x < 480; x += 107)
    m_tft->drawFastVLine(x, 160, 320, MAGENTA);
  m_tft->drawFastVLine(319, 160, 320, MAGENTA);

  m_tft->setTextSize(2);
  m_tft->setTextColor(BLUE);
  const uint8_t NUMLABELS = 12;
  const char *labels[NUMLABELS] = {
    " ok",
    " ABC", "DEF", " GHI", " JKL", "MNO", "PQRS", " TUV", "WXYZ",
    " del", "  _", ""
  };
  for(int i = 0, x = 30, y = 190; i < NUMLABELS; i++) {
    for(int j = 0; j < 3; j++, i++) {
      m_tft->setCursor(x, y);
      if(i == 0 || i == 9 || i == 10) m_tft->setTextColor(WHITE);
      m_tft->print(labels[i]);
      if(i == 0 || i == 9 || i == 10) m_tft->setTextColor(BLUE);
      x += 107;
      if(x > 344) x = 30;
    }
    i--;
    y += 80;
    if(y > 430) y = 190;
  }
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/**
 * Text page UI, run each turn
 */
void TextPageUIElement::runEachTurn(){
  // Do nothing
}
//////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
void TextHistory::clear() {
  members = cursor = start = 0;
  iter = -1;
  memset(buf, 0, SIZE);
}

void TextHistory::store(const char *word) {
  if(word == NULL)
    return;
  if(full() && ++start == SIZE)
    start = 0;
  if(members < SIZE)
    members++;

  buf[cursor++] = (char *) word;
  if(cursor == SIZE)
    cursor = 0;
}

// TODO validate this
void TextHistory::remove() { // remove last member
  uint8_t lastMemberIndex;
  if(members == 0) return;
  if(cursor == 0)
    lastMemberIndex = SIZE;
  else
    lastMemberIndex = cursor - 1;
  buf[lastMemberIndex] = NULL;
  cursor = lastMemberIndex;
  members--;
}

const char *TextHistory::first() {
  char *firstVal = NULL;
  if(members > 0) {
    firstVal = buf[start];
    iter = (start + 1) % SIZE;
    if(buf[iter] == 0)
      iter = -1;
  }
  return firstVal;
}

const char *TextHistory::next() {
  if(iter == -1) // first must be called before next
    return NULL;
  const char *nextVal = NULL;

  nextVal = buf[iter];
  iter = ( iter + 1 ) % SIZE;
  if(iter == cursor)
    iter = -1;

  return nextVal;
}

void TextHistory::debug() {
  D(
    "cursor=%d, iter=%d, start=%d, members=%d, ",
    cursor, iter, start, members
  )
  for(int i = 0; i<SIZE; i++)
    D("buf[%d]=%s ", i, buf[i] ? buf[i] : "NULL")
  D("\n")
}
void TextHistory::test() { // note this assumes SIZE of 3
  // assert(textHistory.size() == 0);
  // textHistory.store("1");
  // assert(textHistory.size() == 1);
  // textHistory.clear();
  // assert(textHistory.size() == 0);
  // textHistory.store("1");
  // assert(textHistory.first()[0] == '1');
  // textHistory.store("2");
  // assert(textHistory.next() == NULL);
  // assert(textHistory.first()[0] == '1');
  // assert(textHistory.next()[0] == '2');
  // assert(textHistory.next() == NULL);
  // textHistory.store("3");
  // assert(textHistory.first()[0] == '1');
  // assert(textHistory.next()[0] == '2');
  // assert(textHistory.next()[0] == '3');
  // assert(textHistory.next() == NULL);
  // textHistory.store("4");
  // assert(textHistory.next() == NULL);
  // assert(textHistory.first()[0] == '2');
  // assert(textHistory.next()[0] == '3');
  // assert(textHistory.next()[0] == '4');
  // assert(textHistory.next() == NULL);

  // D("\n")
  // textHistory.debug();
  // D("\n")

  // const char *w[] = { "a", "b", "c", "d" };
  // textHistory.debug();
  // for(int i = 0; i<4; i++) {
  //   textHistory.store(w[i]);
  //   textHistory.debug();
  // }

  // m_tft->fillScreen(BLACK);
  // m_tft->setTextSize(2);
  // m_tft->setTextColor(WHITE, BLACK);
  // m_tft->setCursor(0, 0);
  // //for(int i = 0; i<3; i++)
  // //  m_tft->print(textHistory.buf[i]);
  // for(const char *cp = textHistory.first(); cp; cp = textHistory.next())
  //   m_tft->print(cp ? cp : "NULL");
  // textHistory.debug();

  // D("textHistory.first()=%s\n", textHistory.first() ? "true" : "false")
  // if(textHistory.first())
  //   D("textHistory.first()=%s\n", textHistory.first())
  // const char *n = textHistory.next();
  // D("textHistory.next()=%s\n", n ? n : "NULL")
  // n = textHistory.next();
  // D("textHistory.next()=%s\n", n ? n : "NULL")
  // n = textHistory.next();
  // D("textHistory.next()=%s\n", n ? n : "NULL")
  // n = textHistory.next();
  // D("textHistory.next()=%s\n", n ? n : "NULL")

  // D("iterating: ");
  // for(const char *cp = textHistory.first(); cp; cp = textHistory.next())
  //   D(cp ? cp : "NULL");
  // D("\n");
  // textHistory.debug();

  // D("\niterating with e:\n");
  // textHistory.debug();
  // textHistory.store("e");
  // textHistory.debug();
  // for(const char *cp = textHistory.first(); cp; cp = textHistory.next()) {
  //   textHistory.debug();
  //   D(cp ? cp : "NULL");
  // }
  // D("\n");
  // textHistory.debug();

  /*
  m_tft->setCursor(0, 40);
  m_tft->print(textHistory.hasNext());
  m_tft->print(textHistory.next());
  m_tft->setCursor(0, 60);
  m_tft->print(textHistory.hasNext());
  m_tft->print(textHistory.next());
  m_tft->setCursor(0, 80);
  m_tft->print(textHistory.hasNext());
  // m_tft->print(textHistory.next());
  m_tft->print(textHistory.next());
  m_tft->print(textHistory.next());
  m_tft->print(textHistory.next());
  for(int y = 0; textHistory.hasNext(); y += 20 ) {
    m_tft->setCursor(0, y);
    m_tft->print(textHistory.next());
    m_tft->print(y);
  }
  */
}
