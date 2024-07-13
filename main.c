#include <errno.h> // for errno
#include <fcntl.h> // for open
#include <linux/input.h> // for input_event
#include <stdbool.h> // for bool
#include <stdio.h> // for fprint
#include <stdlib.h> // for EXIT_FAILURE & EXIT_SUCCESS
#include <string.h> // for strerror
#include <termios.h> // for termios
#include <time.h> // for nanosleep
#include <unistd.h> // for read & STDIN_FILENO

// The state machine for the TV remote.
#include "state_machine/TvRemoteSm.h"

// Key codes for B1 & B2.
#define B1_CODE 17 // w
#define B2_CODE 31 // b

// Key event types.
#define RELEASED_EVENT 0
#define PRESSED_EVENT 1
#define REPEATED_EVENT 2

#define SECONDS_TO_MS 1000
#define MS_TO_MICROSEC 1000
#define SECONDS_TO_NANOSEC 1000000

// https://stackoverflow.com/questions/1157209/is-there-an-alternative-sleep-function-in-c-to-milliseconds
/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(const unsigned long msec)
{
    struct timespec ts;
    int res;

    // Get the non-fractional seconds.
    ts.tv_sec = msec / SECONDS_TO_MS;
    // Get the fractional value at nanosecond precision.
    ts.tv_nsec = (msec % SECONDS_TO_MS) * SECONDS_TO_NANOSEC;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

// Function to get the time in ms.
long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long)tv.tv_sec)*SECONDS_TO_MS)+(tv.tv_usec/MS_TO_MICROSEC);
}

// logic comes from https://github.com/MichaelDipperstein/keypress/blob/master/keypress.c
void console_echo(const bool echo_off)
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

// Handle the state transitions between key press & long-press.
void handle_button_press(const int value, KeyState* key, TvRemoteSm* tv_remote)
{
    switch (value)
    {
        case RELEASED_EVENT:
        {
            // Clear the flags.
            key->pressed = false;
            key->long_press = false;
            key->press_start_time = 0;
            break;
        }
        case PRESSED_EVENT:
        {
            // Check if the key has already been pressed.
            // If not, then register the press.
            // This will be cleared on the key release event.
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
            // Check if the key has already been pressed and a long-press event hasn't been recorded.
            // If the long-press event hasn't been recorded, then register the long-press.
            // This is necessary because this event will continue to be triggered while the key is depressed.
            if (key->pressed && !key->long_press)
            {
                // Check if it has been long enough to be considered a "long-press";
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

    // Configure the State Machine for the TV remote.
    TvRemoteSm TvRemote;
    TvRemoteSm_ctor(&TvRemote);
    TvRemoteSm_start(&TvRemote);

    // Store the state of the buttons.
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

    // Open the keyboard input device.
    // https://stackoverflow.com/questions/20943322/accessing-keys-from-linux-input-device/20946151#20946151
    const char *dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
    struct input_event event;
    ssize_t n;
    const int fd = open(dev, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        return EXIT_FAILURE;
    }

    printf("Starting loop.\n");
    while(true)
    {
        // Read an event from the keyboard.
        n = read(fd, &event, sizeof event);
        if (n == (ssize_t)-1) {
            if (errno == EINTR) {
                // Continue processing in the case of an interrupted system call.
                continue;
            } else {
                // Error.
                break;
            }
        } else if (n != sizeof event) {
            // Failed to read enough data to constitute an event.
            errno = EIO;
            break;
        }

        // Check if this is a key event with an event that we care about:
        // - RELEASED_EVENT: 0
        // - PRESSED_EVENT: 1
        // - REPEATED_EVENT: 2
        if (event.type == EV_KEY && event.value >= 0 && event.value <= 2)
        {
            switch (event.code)
            {
            case B1_CODE:
            {
                handle_button_press(event.value, &b1, &TvRemote);
                break;
            }
            case B2_CODE:
            {
                handle_button_press(event.value, &b2, &TvRemote);
                break;
            }
            default:
                // Ignore other keys.
                break;
            }
        }
    }
    // Flush any remaining output.
    fflush(stdout);
    fprintf(stderr, "%s.\n", strerror(errno));

    // Reset the console.
    const bool ECHO_ON = false;
    console_echo(ECHO_ON);

    // Exit the application.
    return EXIT_SUCCESS;
}