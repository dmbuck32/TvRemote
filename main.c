#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <state_machine/ButtonSm.h>
#include <state_machine/TvRemoteSm.h>

// https://stackoverflow.com/questions/4025891/create-a-function-to-check-for-key-press-in-unix-using-ncurses
int kbhit(void)
{
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

// https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int main(int argc, char ** argv)
{
    // init screen and sets up screen
    initscr();
    // print to screen
    printw("TV OFF\n");

    // Setup non-blocking mode.
    cbreak();
    noecho();
    nodelay(stdscr, true);

    scrollok(stdscr, true);

    while(true)
    {
        if (kbhit()) // If the keyboard is hit.
        {
            int ch = getch();
            // printw("Key pressed: %d\n", ch);
            switch(ch)
            {
                case 119: // w
                {
                    printw("w key pressed\n");
                    break;
                }
                case 115: // s
                {
                    printw("s key pressed\n");
                    break;
                }
                
            }
        }
        else {
            msleep(1);
        }

    }

    // Deallocates memory and ends ncurses.
    endwin();

    return 0;
}