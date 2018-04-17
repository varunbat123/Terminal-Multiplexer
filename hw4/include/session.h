#ifndef SESSION_H
#define SESSION_H



/*
 * Data structure maintaining information about a session.
 */

#include "vscreen.h"

struct session {
    int sid;           // Index in session table.
    int pid;           // Process ID of session leader.
    int ptyfd;         // FD for master side of pty.
    int error;         // Whether a read error has occurred.
    VSCREEN *vscreen;  // Associated virtual screen.
};
typedef struct session SESSION;

#define MAX_SESSIONS 10
int numSessions;
int execute;
extern SESSION *sessions[];
extern SESSION *fg_session;
unsigned int global_options;
char * filename;
SESSION *session_init(char *path, char *argv[]);
void session_setfg(SESSION *session);
int session_read(SESSION *session, char *buf, int bufsize);
int session_putc(SESSION *session, char c);
void session_kill(SESSION *session);
void session_fini(SESSION *session);

#endif
