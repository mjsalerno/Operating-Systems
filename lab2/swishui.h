#ifndef SWISHUI_H
#define SWISHUI_H
#include <curses.h>

/**
 * Moves the cursor left or right relative to the position that it is currently at.
 * @param screen Window associated with ncurses.
 * @param offset int representing the offset in characters to move. This value can be negative or positive.
 */
void moveCursorX(WINDOW *screen, int offset);

/**
 * Moves the cursor back to the current start of the line.
 * @param screen Window associated with ncurses.
 */
void resetCursorX(WINDOW *screen);

#endif
