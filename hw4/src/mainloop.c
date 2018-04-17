/*
 * === DO NOT MODIFY THIS FILE ===
 * If you know what is good for you, you won't make any changes to this file.
 * This is because this file performs magic that you aren't supposed to have
 * to know about to do this assignment, and if you change it, it isn't going
 * to work any more.
 * === DO NOT MODIFY THIS FILE ===
 */

#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/signal.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

#include "ecran.h"

static int setfds(fd_set *fds);



void alrmhandle(){
	sigA =1;
}
void handler(){

sigc = 1;



}

/*
 * This function encapsulates the technicalities of non-blocking I/O,
 * which is required in order to ensure that data is transferred back
 * and forth between the physical terminal and the virtual sessions
 * without the possibility of "hanging" due to trying to read from a
 * file descriptor that does not currently have available data.
 */
int mainloop(void) {
	alarm(1);
    struct timeval tv;
    fd_set fds;
    while(1) {
	int c = wgetch(main_screen);
	if(c != ERR) {

	    // If command escape -- process command
	    if(c == COMMAND_ESCAPE) {
		// Temporarily disable non-blocking I/O to make it
		// easier to collect the rest of the command.
		nodelay(main_screen, FALSE);
		do_command();
		// Restore non-blocking I/O before returing.
		nodelay(main_screen, TRUE);
	    } else {
		// Write char to pty of foreground session -- as if typed.
		session_putc(fg_session, c);
	    }
	}


	signal(SIGCHLD, handler);
	signal(SIGALRM, alrmhandle);




	// Hook called to do any other processing (such as dealing with
	// terminated sessions) that must be taken care of.
	do_other_processing();

	// Check each session to see if there is output to read.
	// Maybe we only really have to do this for foreground session.
	// But we still need to use select to be responsive to command escapes.
	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	FD_ZERO(&fds);
	int nfds = setfds(&fds);
	int s;
	if((s = select(nfds, &fds, NULL, NULL, &tv)) > 0) {
	    for(int i = 0; i < MAX_SESSIONS; i++) {
		SESSION *session = sessions[i];
		if(session != NULL && FD_ISSET(session->ptyfd, &fds)) {
		    char buf[100];
		    int n = session_read(session, buf, sizeof(buf));
		    if(n == EOF) {
			// This can occur if the session leader terminates,
			// leaving no process on the slave side of the pty.
			// To avoid spinning until the session has been
			// properly cleaned up, we set an error flag so that
			// this session will be ignored by select().
			session->error = 1;
		    } else {
			for(char *bp = buf; n > 0; n--) {
			    vscreen_putc(session->vscreen, *bp++);
			    vscreen_sync(session->vscreen);
			}
		    }
		}
	    }
	}

    }

    // NOT REACHED
}

/*
 * Helper function to initialize a set of file descriptors for select().
 */
int setfds(fd_set *fds) {
    int max = -1;
    for(int i = 0; i < MAX_SESSIONS; i++) {
	SESSION *session = sessions[i];
	if(session != NULL && !session->error) {
	    FD_SET(session->ptyfd, fds);
	    max = session->ptyfd;
	}
    }
    return max+1;
}
