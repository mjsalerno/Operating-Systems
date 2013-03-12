#include "swishui.h"


void moveCursorX(WINDOW *screen, int offset){
  move(getcury(screen), getcurx(screen) + offset);
}

void resetCursorX(WINDOW *screen){
  move(getcury(screen), 0);
}
