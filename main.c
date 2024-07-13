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
#include <termios.h>
#include <time.h>
#include <unistd.h>

// Key codes for B1 & B2.
#define B1_CODE 17 // w
#define B2_CODE 31 // b

// Key event types.
#define RELEASED_EVENT 0
#define PRESSED_EVENT 1
#define REPEATED_EVENT 2

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

    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

// logic comes from https://github.com/MichaelDipperstein/keypress/blob/master/keypress.c
void console_echo(bool echo_off)
{
    unsigned char echo_bit;
    if(echo_off)
    {
        echo_bit = ECHO; // Set to disable echo.
    }
    else {
        echo_bit = 0; // Clear to enable echo.
    }

    struct termios state;
    tcgetattr(STDIN_FILENO, &state);
    state.c_lflag &= ~(echo_bit | ICANON);;
    tcsetattr(STDIN_FILENO, TCSANOW, &state);
}

const unsigned int LONG_PRESS_TIMEOUT = 800; // ms.

typedef struct KeyState {
    unsigned long long press_start_time;
    bool pressed;
    bool long_press;
    int press_event;
    int long_press_event;
} KeyState;

void handle_button_press(int value, KeyState* key, TvRemoteSm* tv_remote)
{
    switch (value)
    {
        case RELEASED_EVENT:
        {
            key->pressed = false;
            key->long_press = false;
            break;
        }
        case PRESSED_EVENT:
        {
            if (!key->pressed)
            {
                key->pressed = true;
                key->press_start_time = timeInMilliseconds();
                TvRemoteSm_dispatch_event(tv_remote, key->press_event);
            }
            break;
        }
        case REPEATED_EVENT:
        {
            if (!key->long_press)
            {
                if ((timeInMilliseconds() - key->press_start_time) > LONG_PRESS_TIMEOUT)
                {
                    key->long_press = true;
                    TvRemoteSm_dispatch_event(tv_remote, key->long_press_event);
                }
            }
            break;
        }
        default:
            break;
    }
    return;
}

int main(int argc, char ** argv)
{
    // Don't echo key inputs.
    const bool ECHO_OFF = true;
    console_echo(ECHO_OFF);

    printf("Initializing TV remote state machine...\n");
    TvRemoteSm TvRemote;
    TvRemoteSm_ctor(&TvRemote);
    TvRemoteSm_start(&TvRemote);

    printf("Initalizing keys...\n");
    KeyState b1 = {
        .pressed = false,
        .press_start_time = 0,
        .long_press = false,
        .press_event = TvRemoteSm_EventId_B1_PRESS,
        .long_press_event = TvRemoteSm_EventId_B1_LONG_PRESS
    };
    KeyState b2 = {
        .pressed = false,
        .press_start_time = 0,
        .long_press = false,
        .press_event = TvRemoteSm_EventId_B2_PRESS,
        .long_press_event = TvRemoteSm_EventId_B2_LONG_PRESS
    };

    printf("Initializing key reader...\n");

    // Get keyboard events.
    const char *dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
    struct input_event ev;
    ssize_t n;
    int fd;

    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Starting loop.\n");
    while(true)
    {
        n = read(fd, &ev, sizeof ev);
        if (n == (ssize_t)-1) {
            if (errno == EINTR)
                continue;
            else
                break;
            } else if (n != sizeof ev) {
            errno = EIO;
            break;
        }

        if (ev.type == EV_KEY && ev.value >= 0 && ev.value <= 2)
        {
            switch (ev.code)
            {
            case B1_CODE:
            {
                handle_button_press(ev.value, &b1, &TvRemote);
                break;
            }
            case B2_CODE:
            {
                handle_button_press(ev.value, &b2, &TvRemote);
                break;
            }
            default:

                break;
            }
        }
    }
    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));
    const bool ECHO_ON = false;
    console_echo(ECHO_ON);
    return EXIT_SUCCESS;
}