#include <ncurses.h>

#include "session.h"
#include "vscreen.h"

/*
 * The control character used to escape program commands,
 * so that they are not simply transferred as input to the
 * foreground session.
 */
#define COMMAND_ESCAPE 0x1   // CTRL-A
int sigA;
int sigc;
int mainloop(void);
void set_status(char * status);
void vscreen_status(VSCREEN * vscreen);
void do_command(void);
void do_other_processing(void);
