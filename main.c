#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ncurses.h>
#include <menu.h>

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

typedef enum {
    SCHEME_GREEN_RED,
    SCHEME_BLUE_YELLOW,
    SCHEME_GREEN_BLACK,
    SCHEME_WHITE_GREEN,
    SCHEME_COUNT
} ColorScheme;

typedef struct {
    ColorScheme scheme;
    bool invert;
} Settings;

// each row is {foreground, background}
static const int schemeColors[SCHEME_COUNT][2] = {
    { COLOR_GREEN, COLOR_RED },
    { COLOR_BLUE,  COLOR_YELLOW },
    { COLOR_GREEN, COLOR_BLACK },
    { COLOR_WHITE, COLOR_GREEN },
};

static const char *schemeNames[SCHEME_COUNT] = {
    "Green & Red",
    "Blue & Yellow",
    "Green & Black",
    "White & Green",
};

#define UI_PAIR 1

// Audio chime
typedef struct {
    bool available;
    char player[64];
    char soundFile[256];
} AudioConfig;

static AudioConfig audioConfig = { 0 };

static bool commandExists(const char *cmd) {
    const char *pathEnv = getenv("PATH");
    if (!pathEnv) return false;

    char pathCopy[4096];
    strncpy(pathCopy, pathEnv, sizeof(pathCopy) - 1);
    pathCopy[sizeof(pathCopy) - 1] = '\0';

    char *saveptr = NULL;
    char *dir = strtok_r(pathCopy, ":", &saveptr);
    while (dir) {
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, cmd);
        if (access(fullPath, X_OK) == 0) return true;
        dir = strtok_r(NULL, ":", &saveptr);
    }
    return false;
}

static void detectAudioConfig(void) {
    static const struct { const char *player; const char *file; } candidates[] = {
        { "paplay",  "/usr/share/sounds/freedesktop/stereo/complete.oga" },
        { "pw-play", "/usr/share/sounds/freedesktop/stereo/complete.oga" },
        { "paplay",  "/usr/share/sounds/freedesktop/stereo/bell.oga" },
        { "pw-play", "/usr/share/sounds/freedesktop/stereo/bell.oga" },
        { "aplay",   "/usr/share/sounds/alsa/Front_Center.wav" },
    };

    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        if (access(candidates[i].file, R_OK) == 0 && commandExists(candidates[i].player)) {
            strncpy(audioConfig.player, candidates[i].player, sizeof(audioConfig.player) - 1);
            strncpy(audioConfig.soundFile, candidates[i].file, sizeof(audioConfig.soundFile) - 1);
            audioConfig.available = true;
            return;
        }
    }

    audioConfig.available = false;
}

static void playChime(void) {
    if (!audioConfig.available) {
        beep(); // ncurses terminal bell: always safe, no audio subsystem touched
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull != -1) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
        }
        execlp(audioConfig.player, audioConfig.player, audioConfig.soundFile, (char *)NULL);
        _exit(127); // exec failed; parent doesn't wait so this just vanishes
    }
    // parent: don't wait, let it play in the background
}


static bool colorSupported = false;

static void applyColorScheme(Settings *settings) {
    if (!colorSupported) return;

    int fg = schemeColors[settings->scheme][0];
    int bg = schemeColors[settings->scheme][1];
    if (settings->invert) {
        int tmp = fg;
        fg = bg;
        bg = tmp;
    }

    init_pair(UI_PAIR, fg, bg);
    bkgd(COLOR_PAIR(UI_PAIR));
}


static void strobeScreen(int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        attron(A_REVERSE);
        bkgdset(A_REVERSE | (colorSupported ? COLOR_PAIR(UI_PAIR) : 0));
        erase();
        refresh();
        napms(delayMs);

        attroff(A_REVERSE);
        bkgdset(colorSupported ? COLOR_PAIR(UI_PAIR) : A_NORMAL);
        erase();
        refresh();
        napms(delayMs);
    }
}

// Timer phase (accurate countdown)

static void runPhase(WINDOW *win, int totalSeconds, const char *label, bool *cancelled) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    wtimeout(win, 200); // poll for cancel key without blocking display updates
    int lastRemaining = -1;

    while (true) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        double elapsed = (double)(now.tv_sec - start.tv_sec) +
                          (double)(now.tv_nsec - start.tv_nsec) / 1e9;
        int remaining = totalSeconds - (int)elapsed;
        if (remaining <= 0) break;

        if (remaining != lastRemaining) {
            lastRemaining = remaining;
            int minutes = remaining / 60;
            int seconds = remaining % 60;

            werase(win);
            box(win, 0, 0);
            mvwprintw(win, 1, 1, "Pomodoro Timer");
            mvwprintw(win, 2, 1, "%s", label);
            mvwprintw(win, 4, 3, "Time Remaining: %02d:%02d", minutes, seconds);
            mvwprintw(win, 5, 1, "[q] Cancel to menu");
            wrefresh(win);
        }

        int ch = wgetch(win);
        if (ch == 'q' || ch == 'Q' || ch == 27) {
            *cancelled = true;
            return;
        }
    }
}

// Session/break cycle

static void runPomodoroCycle(int sessionSeconds, int breakSeconds, Settings *settings) {
    int winWidth = 40, winHeight = 7;
    WINDOW *win = newwin(winHeight, winWidth, (LINES - winHeight) / 2, (COLS - winWidth) / 2);
    keypad(win, TRUE);
    if (colorSupported) wbkgd(win, COLOR_PAIR(UI_PAIR));

    bool cancelled = false;

    runPhase(win, sessionSeconds, "WORK SESSION", &cancelled);
    if (!cancelled) {
        playChime();
        strobeScreen(4, 120);
        if (colorSupported) wbkgd(win, COLOR_PAIR(UI_PAIR));
        touchwin(win);
        box(win, 0, 0);
        wrefresh(win);

        runPhase(win, breakSeconds, "BREAK", &cancelled);
        if (!cancelled) {
            playChime();
            strobeScreen(4, 120);
        }
    }

    delwin(win);
    (void)settings;
    touchwin(stdscr);
    refresh();
}

static void startCustomPomodoro(Settings *settings) {
    echo();
    curs_set(1);

    int row1 = LINES - 4, row2 = LINES - 3, col = 2;

    mvprintw(row1, col, "Enter session time (minutes): ");
    refresh();
    int sessionTime = 0;
    int r1 = scanw("%d", &sessionTime);

    mvprintw(row2, col, "Enter break time (minutes):   ");
    refresh();
    int breakTime = 0;
    int r2 = scanw("%d", &breakTime);

    noecho();
    curs_set(0);

    move(row1, 0); clrtoeol();
    move(row2, 0); clrtoeol();
    refresh();

    if (r1 != 1 || sessionTime <= 0) sessionTime = 25;
    if (r2 != 1 || breakTime < 0) breakTime = 5;

    runPomodoroCycle(sessionTime * 60, breakTime * 60, settings);
}

// Settings menu

static void settingsScreen(Settings *settings) {
    int winWidth = 44, winHeight = 10;
    WINDOW *win = newwin(winHeight, winWidth, (LINES - winHeight) / 2, (COLS - winWidth) / 2);
    keypad(win, TRUE);
    wtimeout(win, -1); // blocking: no live timer to update here

    int selected = 0;
    const int numRows = 3;
    bool inSettings = true;

    while (inSettings) {
        applyColorScheme(settings);
        if (colorSupported) wbkgd(win, COLOR_PAIR(UI_PAIR));

        werase(win);
        box(win, 0, 0);
        mvwprintw(win, 1, 2, "Settings");
        mvwprintw(win, 3, 2, "%c Color Scheme:  %-14s", selected == 0 ? '>' : ' ', schemeNames[settings->scheme]);
        mvwprintw(win, 4, 2, "%c Invert Colors: %s", selected == 1 ? '>' : ' ', settings->invert ? "ON " : "OFF");
        mvwprintw(win, 6, 2, "%c Back", selected == 2 ? '>' : ' ');
        mvwprintw(win, 8, 2, "UP/DOWN select, LEFT/RIGHT/ENTER change");
        wrefresh(win);

        int ch = wgetch(win);
        switch (ch) {
            case KEY_UP:
                selected = (selected - 1 + numRows) % numRows;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % numRows;
                break;
            case KEY_LEFT:
                if (selected == 0) settings->scheme = (settings->scheme - 1 + SCHEME_COUNT) % SCHEME_COUNT;
                else if (selected == 1) settings->invert = !settings->invert;
                break;
            case KEY_RIGHT:
            case 10: // enter
                if (selected == 0) settings->scheme = (settings->scheme + 1) % SCHEME_COUNT;
                else if (selected == 1) settings->invert = !settings->invert;
                else if (selected == 2) inSettings = false;
                break;
            case 'q':
            case 27: // esc
                inSettings = false;
                break;
            default:
                break;
        }
    }

    delwin(win);
    touchwin(stdscr);
    refresh();
}

// Main menu

int main(void) {
    signal(SIGCHLD, SIG_IGN); // reap chime child processes automatically
    detectAudioConfig();

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    Settings settings = { .scheme = SCHEME_GREEN_BLACK, .invert = false };

    colorSupported = has_colors();
    if (colorSupported) {
        start_color();
        applyColorScheme(&settings);
    }
    refresh();

    char *menuOptions[] = {"25/5 POMODORO", "50/10 POMODORO", "CUSTOM", "SETTINGS", "QUIT"};
    int numOptions = sizeof(menuOptions) / sizeof(menuOptions[0]);

    ITEM **menuItems = (ITEM **)calloc(numOptions + 1, sizeof(ITEM *));
    for (int i = 0; i < numOptions; i++) {
        menuItems[i] = new_item(menuOptions[i], "");
    }
    menuItems[numOptions] = NULL;
    MENU *menu = new_menu(menuItems);

    int menuWidth = 30, menuHeight = 10;
    WINDOW *titleWin = newwin(3, menuWidth, LINES / 2 - 8, (COLS - menuWidth) / 2);
    WINDOW *menuWin = newwin(menuHeight, menuWidth, LINES / 2 - 5, (COLS - menuWidth) / 2);
    keypad(menuWin, TRUE);

    set_menu_win(menu, menuWin);
    set_menu_sub(menu, derwin(menuWin, 6, menuWidth - 2, 2, 1));
    set_menu_format(menu, 5, 1);
    set_menu_mark(menu, "> ");

    if (colorSupported) {
        wbkgd(titleWin, COLOR_PAIR(UI_PAIR));
        wbkgd(menuWin, COLOR_PAIR(UI_PAIR));
    }

    box(titleWin, 0, 0);
    mvwprintw(titleWin, 1, 1, "PomoTask");
    wrefresh(titleWin);

    post_menu(menu);
    wrefresh(menuWin);

    bool running = true;
    while (running) {
        int choice = wgetch(menuWin);
        switch (choice) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
            case 10: { // enter key
                int idx = item_index(current_item(menu));
                if (idx == 0) {
                    runPomodoroCycle(25 * 60, 5 * 60, &settings);
                } else if (idx == 1) {
                    runPomodoroCycle(50 * 60, 10 * 60, &settings);
                } else if (idx == 2) {
                    startCustomPomodoro(&settings);
                } else if (idx == 3) {
                    settingsScreen(&settings);
                } else if (idx == 4) {
                    running = false;
                }

                // returning here from any sub-view redraws the main window
                // instead of the app exiting or recursing into itself
                if (running) {
                    applyColorScheme(&settings);
                    if (colorSupported) {
                        wbkgd(titleWin, COLOR_PAIR(UI_PAIR));
                        wbkgd(menuWin, COLOR_PAIR(UI_PAIR));
                    }
                    touchwin(titleWin);
                    touchwin(menuWin);
                    box(titleWin, 0, 0);
                    mvwprintw(titleWin, 1, 1, "PomoTask");
                    wrefresh(titleWin);
                    pos_menu_cursor(menu);
                    wrefresh(menuWin);
                }
                break;
            }
            default:
                break;
        }
    }

    // clean up everything
    unpost_menu(menu);
    free_menu(menu);
    for (int i = 0; i < numOptions; i++) {
        free_item(menuItems[i]);
    }
    free(menuItems);
    delwin(menuWin);
    delwin(titleWin);

    endwin();

    return 0;
}
