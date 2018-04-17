/*
 * Ecran: A program that (if properly completed) supports the multiplexing
 * of multiple virtual terminal sessions onto a single physical terminal.
 */

#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "ecran.h"

static void initialize();
static void curses_init(void);
static void curses_fini(void);
static void finalize(void);
void set_time(){
    int hour;
    int minutes;
    int seconds;
    time_t inittime;
   struct tm *timeObject;
   time(&inittime);
   timeObject = gmtime(&inittime );
   wmove(status_screen,0,COLS -11);
   wrefresh(status_screen);
   hour = timeObject->tm_hour;
   if (hour <=4)
   {
        if(hour == 4){
            hour =0;
        }
        else{
        hour +=24;
        hour -= 4;
    }

   }
   // trying to submit again
   else
   {
    hour = (hour%24)-4;
   }
   minutes = (timeObject->tm_min)%60;
   seconds = (timeObject->tm_sec)%60;
   waddch (status_screen, (hour/10) + 48);
   waddch(status_screen, (hour%10) + 48);
   waddch(status_screen, ':');
   waddch (status_screen, (minutes/10) + 48);
   waddch(status_screen, (minutes%10) + 48);
   waddch(status_screen, ':');
   waddch (status_screen, (seconds/10) + 48);
   waddch(status_screen, (seconds%10) + 48);
   waddch(status_screen, ' ');
   waddch(status_screen, '#');
   waddch(status_screen, (numSessions%10)+48);
   wrefresh(status_screen);
   vscreen_sync(fg_session-> vscreen);

}


void set_status( char* status){
    char *line = status;
    wmove(status_screen, 1, 0);


    wclrtoeol(status_screen);
    for(int c = 0; c < strlen(line) ; c++)
    {
        char ch = line[c];
        if(isprint(ch))
        {
            waddch(status_screen, line[c]);
        }
    }
        //wmove(main_screen, fg_session->vscreen->cur_line, fg_session->vscreen->cur_col);
        wrefresh(status_screen);
        vscreen_sync(fg_session-> vscreen);



}

int main(int argc, char *argv[]) {
    numSessions=1;
     execute = 0;
    char * cmd;

     // CHECK IF ARGV 2 IS NOT NULL
    if(argc >=2){
        if(argv[1][0]== '-'){
            if (argv[1][1]=='o')
            {
               global_options= global_options | 0x00000001;
            }
        }
    }
    if(argv[2] != NULL)
    {
        filename = argv[2];
    }
    else{

    }
    if((global_options != 1) && (argc >=2))
    {
        execute =1;
        cmd = argv[1];
        argv = argv +1;



    }
    if ((global_options == 1) && (argc >=4))
    {
        execute =1;
        cmd = argv[3];
        argv = argv +3;
    }
    initialize(execute, cmd, argv);
    mainloop();

    // NOT REACHED
}

/*
 * Initialize the program and launch a single session to run the
 * default shell.
 */
static void initialize(int execute, char* cmd, char *argv[]) {
    curses_init();
    char *path = getenv("SHELL");
    if(path == NULL)
	   path = "/bin/bash";
    if (execute ==1)
    {
        session_init(cmd, argv);
    }
    else{
            char *argv[3] = { " (ecran session)",  NULL };
            if(session_init(path, argv) == NULL)
            {
            exit(EXIT_FAILURE);
            }

    }
}

/*
 * Cleanly terminate the program.  All existing sessions should be killed,
 * all pty file descriptors should be closed, all memory should be deallocated,
 * and the original screen contents should be restored before terminating
 * normally.  Note that the current implementation only handles restoring
 * the original screen contents and terminating normally; the rest is left
 * to be done.
 */
static void finalize(void) {
    for (int i = 0; i < MAX_SESSIONS; i++)
        {
            if(sessions[i]!= NULL)
            {
                session_kill(sessions[i]);
                sigc =1;
            }
        }
    curses_fini();
    exit(EXIT_SUCCESS);
}

/*
 * Helper method to initialize the screen for use with curses processing.
 * You can find documentation of the "ncurses" package at:
 * https://invisible-island.net/ncurses/man/ncurses.3x.html
 */
static void curses_init(void) {
    initscr();
    raw();                       // Don't generate signals, and make typein
                                 // immediately available.
    noecho();                    // Don't echo -- let the pty handle it.
    main_screen = newwin(LINES -1 ,COLS,0,0);
    status_screen = newwin(1,COLS,LINES-1 ,0);
    nodelay(status_screen, TRUE);  // Set non-blocking I/O on input.
    wclear(status_screen);
    nodelay(main_screen, TRUE);  // Set non-blocking I/O on input.
    wclear(main_screen);         // Clear the screen.

    refresh();                   // Make changes visible.
}

/*
 * Helper method to finalize curses processing and restore the original
 * screen contents.
 */
void curses_fini(void) {
    endwin();
}

/*
 * Function to read and process a command from the terminal.
 * This function is called from mainloop(), which arranges for non-blocking
 * I/O to be disabled before calling and restored upon return.
 * So within this function terminal input can be collected one character
 * at a time by calling getch(), which will block until input is available.
 * Note that while blocked in getch(), no updates to virtual screens can
 * occur.
 */
void do_command() {
    // Quit command: terminates the program cleanly
    char character = wgetch(main_screen);
    if(character == 'q'){
	   finalize();
    }




    if(character== 'n'){
        int currentId = fg_session -> sid;
        char *path = getenv("SHELL");
        if(path == NULL)
        path = "/bin/bash";
        char *argv[2] = { " (ecran session)", NULL };
        if(session_init(path, argv) == NULL){
            set_status("too many sessions");



        }
        numSessions++;


    }

    if (character == 'k')

    {
        int isChar =0;
        char character = wgetch(main_screen);
        for (int i = 0; i < MAX_SESSIONS; i++)
        {

            if(character== i+48)
            {
                isChar =1;
                if(sessions[i]!= NULL)
                {
                   session_kill(sessions[i]);


                }
                else
                {

                flash();

                }
            }

        }
        if (isChar == 0)
        {
            flash();
        }
    }


    for (int i = 0; i < MAX_SESSIONS; i++)
    {


        if(character== i+48)
        {
            if(sessions[i]!= NULL)
            {
                vscreen_status(sessions[i]-> vscreen);
                session_setfg(sessions[i]);
            }
            else
            {

            flash();

            }
        }
    }



}

/*
 * Function called from mainloop(), whose purpose is do any other processing
 * that has to be taken care of.  An example of such processing would be
 * to deal with sessions that have terminated.
 */
void do_other_processing() {
int empty =0;
int pid = 0;
int sessionId = fg_session -> sid;
     if ( sigA==1 ) {
            set_time();
            sigA= 0;
            alarm( 1 );
        }
     else{

     }

    if(sigc == 1){





       while( (pid = waitpid(-1,0,WNOHANG)) > 0)
       {

            if(fg_session -> pid == pid)
            {


                for (int i = 0; i < MAX_SESSIONS; i++)
                {
                    if((sessions[i]!=NULL) &&(i!= sessionId))
                    {


                        empty =1;
                        vscreen_status(sessions[i]-> vscreen);
                        session_setfg(sessions[i]);
                    }

                }

                close(sessions[sessionId]->ptyfd);
                session_fini(sessions[sessionId]);
                sessions[sessionId] = NULL;
                numSessions--;



            }
            else
            {

                for (int i = 0; i< MAX_SESSIONS; i++)
                {
                    if (sessions[i]!= NULL)
                    {
                        empty = 1;
                        int s = sessions[i] -> pid;
                        if (s == pid)
                        {

                            close(sessions[i]->ptyfd);
                            session_fini(sessions[i]);
                            sessions[i] = NULL;
                            numSessions--;
                            break;
                        }
                    }

                }

            }

       }
       if(empty != 1)
       {
            finalize();
       }

    }
    sigc = 0;

}
