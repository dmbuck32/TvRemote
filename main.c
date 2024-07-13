// #include <state_machine/ButtonSm.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <ncurses.h>
#include <state_machine/TvRemoteSm.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static const char *const evval[3] = {
    "RELEASED",
    "PRESSED ",
    "REPEATED"
};

// const unsigned int B1 = 119; // w
// const unsigned int B2 = 115; // b
#define B1_CODE 17 // w
#define B2_CODE 31 // b
#define RELEASED_EVENT 0
#define PRESSED_EVENT 1
#define REPEATED_EVENT 2

// // https://stackoverflow.com/questions/4025891/create-a-function-to-check-for-key-press-in-unix-using-ncurses
// int kbhit(void)
// {
//     const int ch = getch();
//     if (ch != ERR) {
//         ungetch(ch);
//         return 1;
//     }
//     return 0;
// }

// int button_hit(int button) 
// {
//     const int ch = getch();
//     if (ch == ERR)
//     {
//         return 0;
//     }
//     if (ch != button) 
//     {
//         ungetch(ch);
//         return 0;
//     }
//     return 1;
// }

// int b1_hit(void) { return button_hit(B1); }
// int b2_hit(void) { return button_hit(B2); }

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

// Function to get the time in ms.
long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}



struct KeyInfo {
    unsigned long long initial_press;
};

unsigned long long b1_press_start = 0;
unsigned int b1_press_count = 0;
bool b1_pressed = false;
bool b1_long_press = false;

unsigned long long b2_press_start = 0;
unsigned int b2_press_count = 0;
bool b2_pressed = false;
bool b2_long_press = false;

int main(int argc, char ** argv)
{
    // ButtonSm B1;
    // ButtonSm B2;

    TvRemoteSm TvRemote;
    TvRemoteSm_ctor(&TvRemote);
    TvRemoteSm_start(&TvRemote);


    const char *dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
    struct input_event ev;
    ssize_t n;
    int fd;

    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        return EXIT_FAILURE;
    }

    // // init screen and sets up screen
    // initscr();
    // // print to screen
    // // printw("TV OFF\n");

    // // Setup non-blocking mode.
    // cbreak();
    // noecho();
    // nodelay(stdscr, true);

    // scrollok(stdscr, true);

    while(true)
    {
        n = read(fd, &ev, sizeof ev);
        if (n == (ssize_t)-1) {
            if (errno == EINTR)
                continue;
            else
                break;
        } else
        if (n != sizeof ev) {
            errno = EIO;
            break;
        }
        if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2)
        {
            switch (ev.code)
            {
            case B1_CODE:
            {
                switch (ev.value)
                {
                case RELEASED_EVENT:
                    /* code */
                    break;
                case PRESSED_EVENT:
                {
                    printf("B1 PRESS\n");
                    TvRemoteSm_dispatch_event(&TvRemote, TvRemoteSm_EventId_B1_PRESS);
                    /* code */
                    break;
                }
                case REPEATED_EVENT:
                {
                    printf("B2 LONG PRESS\n");
                    TvRemoteSm_dispatch_event(&TvRemote, TvRemoteSm_EventId_B1_LONG_PRESS);
                    /* code */
                    break;
                }
                default:
                    break;
                }
                break;
            }
            case B2_CODE:
            {
                switch (ev.value)
                {
                case RELEASED_EVENT:
                    /* code */
                    break;
                case PRESSED_EVENT:
                {
                    printf("B2 PRESS\n");
                    TvRemoteSm_dispatch_event(&TvRemote, TvRemoteSm_EventId_B2_PRESS);
                    /* code */
                    break;
                }
                case REPEATED_EVENT:
                {
                    printf("B2 LONG PRESS\n");
                    TvRemoteSm_dispatch_event(&TvRemote, TvRemoteSm_EventId_B2_LONG_PRESS);
                    /* code */
                    break;
                }
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }
        }
            printf("%s 0x%04x (%d)\n", evval[ev.value], (int)ev.code, (int)ev.code);

    }
    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
    return EXIT_FAILURE;

        // if (kbhit()) // If the keyboard is hit.
        // {
        //     if (b1_hit())
        //     {
        //         printw("B1 pressed\n");
        //         b1_press_count++;
        //         if (!b1_pressed)
        //         {
        //             b1_pressed = true;
        //             b1_press_start = timeInMilliseconds();
        //         }
        //     }
        //     if (b2_hit())
        //     {
        //         printw("B2 pressed\n");
        //         b2_press_count++;
        //         if (!b2_pressed)
        //         {
        //             b2_pressed = true;
        //             b2_press_start = timeInMilliseconds();
        //         }
        //     }

        //     // int ch = getch();
        //     // // printw("Key pressed: %d\n", ch);
        //     // switch(ch)
        //     // {
        //     //     case 119: // w
        //     //     {
        //     //         printw("w key pressed\n");
        //     //         if (!b1_pressed)
        //     //         {
        //     //             b1_press_start = timeInMilliseconds();
        //     //         }
        //     //         // B1.vars.input_is_pressed = true;
        //     //         break;
        //     //     }
        //     //     case 115: // s
        //     //     {
        //     //         printw("s key pressed\n");
        //     //         if (!b2_pressed)
        //     //         {
        //     //             b2_press_start = timeInMilliseconds();
        //     //         }
        //     //         // B2.vars.input_is_pressed = true;
        //     //         break;
        //     //     }
                
        //     // }
        // }
        // else {
        //     b1_pressed = false;
        //     b1_press_count = 0;
        //     b2_pressed = false;
        //     b2_press_count = 0;
        //     msleep(1);
        // }

    // }

    // // Deallocates memory and ends ncurses.
    // endwin();

    // return 0;
}