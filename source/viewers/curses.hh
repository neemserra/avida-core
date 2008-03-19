//////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1993 - 1996 California Institute of Technology             //
//                                                                          // 
// Read the LICENSE and README files, or contact 'charles@krl.caltech.edu', //
// before continuing.  SOME RESTRICTIONS MAY APPLY TO USE OF THIS FILE.     //
//////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_CURSES_HH
#define VIEW_CURSES_HH

#ifdef VIEW_CURSES
#define VIEW_TEXT

#undef TRUE
#undef FALSE

extern "C" {
#include <curses.h>
#include <signal.h>
}

// Define all colors as 0, since colors are not handled.
#ifndef COLOR_WHITE
#define COLOR_BLACK   0
#define COLOR_RED     0
#define COLOR_GREEN   0
#define COLOR_BLUE    0
#define COLOR_CYAN    0
#define COLOR_MAGENTA 0
#define COLOR_YELLOW  0
#define COLOR_WHITE   0
#endif

#define COLOR_OFF     0

// Define special characters

#define CHAR_TTEE     '+'
#define CHAR_BTEE     '+'
#define CHAR_LTEE     '+'
#define CHAR_RTEE     '+'
#define CHAR_PLUS     '+'
#define CHAR_HLINE    '-'
#define CHAR_VLINE    '|'
#define CHAR_ULCORNER '+'
#define CHAR_URCORNER '+'
#define CHAR_LLCORNER '+'
#define CHAR_LRCORNER '+'
#define CHAR_BULLET   '*'

// We need three special functions: One to initialize the viewer, one to
// close it up, and one to interactively get inputs from the buffer without
// needing <return> to be pressed.

void StartProg();
void EndProg(int ignore);
inline int GetInput() { return getch(); }
//inline int GetBlockingInput() { 
//  nodelay(stdscr, FALSE);
//  char c = getch(); 
//  nodelay(stdscr, TRUE);
//  return c; }


// Finally, we have the cTextWindow class.

class cTextWindow {
protected:
  WINDOW * win_id;
public:
  cTextWindow();
  cTextWindow(int y_size, int x_size, int y_start=0, int x_start=0);
  ~cTextWindow();

  void Construct(int y_size, int x_size, int y_start=0, int x_start=0);

  // These function return the number of characters wide or high
  // (respectively) that the screen is.
  inline int Width() { return win_id->_maxx; }
  inline int Height() { return win_id->_maxy; }

  // Clear the screen and redraw all text on it.
  inline void RedrawMain() { touchwin(win_id); wrefresh(win_id); }

  // Move the active position of the cursor.
  inline void Move(int new_y, int new_x) { wmove(win_id, new_y, new_x); }

  // Print all of the changes made onto the screen.
  inline void RefreshMain() { wrefresh(win_id); }

  // These functions clear sections of the screen.
  inline void ClearMain() { wclear(win_id); }
  inline void ClearToBot() { wclrtobot(win_id); }
  inline void ClearToEOL() { wclrtoeol(win_id); }
  
  // Various 'graphic' functions.  Box() draws a box-frame around the window.
  // With arguments, it draws a box starting at x, y, with size w by h.
  // VLine and HLine draw lines across the screen ending in the appropriate
  // facing T's (hence, they are used to cut boxes in half.  With two
  // coords and a length, they only draw the line from the specified start,
  // to the specified distance.
  void Box();
  void Box(int x, int y, int w, int h);
  void VLine(int in_x);
  void HLine(int in_y);
  void HLine(int in_y, int start_x, int length);

  // The following functions print characters onto the screen.  If they
  // begin with an y, x then they first move to those coords.
  
  inline void Print(chtype ch) { waddch(win_id, ch); }
  inline void Print(int in_y, int in_x, chtype ch)
    { mvwaddch(win_id, in_y, in_x, ch); }
  inline void Print(char * fmt, ...) {
    va_list argp;
    char buf[BUFSIZ];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    va_end(argp);
    waddstr(win_id, buf);
  }
  inline void Print(int in_y, int in_x, char * fmt, ...) {
    va_list argp;
    char buf[BUFSIZ];

    va_start(argp, fmt);
    vsprintf(buf, fmt, argp);
    va_end(argp);
    wmove(win_id, in_y, in_x);
    waddstr(win_id, buf);
  }

  // These functions check or set the screen colors (BG is always black)
  inline int HasColors() { return FALSE; }
  inline void SetColor(int color) {
    (void) color;
    wattrset(win_id, A_NORMAL);
  }
  inline void SetBoldColor(int color) {
    (void) color;
    wattrset(win_id, A_BOLD);
  }
};

#endif

#endif
