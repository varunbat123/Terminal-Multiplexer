#ifndef VSCREEN_H
#define VSCREEN_H

/*
 * Data structure maintaining information about a virtual screen.
 */

#include <ncurses.h>

typedef struct vscreen VSCREEN;

extern WINDOW *main_screen;
extern WINDOW * status_screen;

VSCREEN *vscreen_init(void);
void vscreen_show(VSCREEN *vscreen);
void vscreen_sync(VSCREEN *vscreen);
void vscreen_putc(VSCREEN *vscreen, char c);
void vscreen_fini(VSCREEN *vscreen);

#endif
