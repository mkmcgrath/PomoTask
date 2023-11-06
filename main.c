#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <menu.h>

// function to start a Pomodoro session with custom times
void startPomodoro(int sessionTime, int breakTime) {
    WINDOW *countdownWindow = newwin(6, 30, (LINES - 6) / 2, (COLS - 30) / 2);
    box(countdownWindow, 0, 0);
    wrefresh(countdownWindow);

    int totalTime = sessionTime + breakTime;
    int remainingTime = sessionTime;

    while (totalTime > 0) {
        int minutes = remainingTime / 60;
        int seconds = remainingTime % 60;

        werase(countdownWindow);
        box(countdownWindow, 0, 0);
        mvwprintw(countdownWindow, 1, 1, "Pomodoro Timer");
        mvwprintw(countdownWindow, 3, 3, "Time Remaining: %02d:%02d", minutes, seconds);
        wrefresh(countdownWindow);

        sleep(1);
        totalTime--;
        remainingTime--;

        if (remainingTime < 0) {
            remainingTime = breakTime;
            // notifies the user that the session is over
            mvwprintw(countdownWindow, 5, 1, "STUDY SESSION OVER. STARTING BREAK.");
            wrefresh(countdownWindow);
            sleep(3); // display the message for 3 seconds
        }
    }

    delwin(countdownWindow);

    // starts the break immediately after the session
    startPomodoro(breakTime, 0);
}

// function to start a custom Pomodoro session
void startCustomPomodoro() {
    int sessionTime, breakTime;
    
    // prompt for session time
    mvprintw(10, 1, "Enter session time (minutes): ");
    echo();
    scanw("%d", &sessionTime);
    noecho();
    
    // prompt for break time
    mvprintw(11, 1, "Enter break time (minutes): ");
    echo();
    scanw("%d", &breakTime);
    noecho();
    
    // start the custom Pomodoro session
    startPomodoro(sessionTime * 60, breakTime * 60);
}

int main() {
    initscr();      // initializes Ncurses
    cbreak();       // disables line buffering
    noecho();       // don't display typed characters
    keypad(stdscr, TRUE); // enable special keys

    int windowHeight = 10;
    int windowWidth = 30;
    int windowY = (LINES - windowHeight) / 2;
    int windowX = (COLS - windowWidth) / 2;

    // create our countdown window
    WINDOW *countdownWindow = newwin(windowHeight, windowWidth, windowY, windowX);
    box(countdownWindow, 0, 0); // Draw a border around the countdown window
    mvwprintw(countdownWindow, 1, 1, "Pomodoro Timer");
    wrefresh(countdownWindow); // Refresh the countdown window

    // define the menu options
    char *menuOptions[] = {"25/5 POMODORO", "50/10 POMODORO", "CUSTOM", "QUIT"};
    int numOptions = sizeof(menuOptions) / sizeof(menuOptions[0]);

    // create a menu using the menu library
    ITEM **menuItems = (ITEM **)calloc(numOptions + 1, sizeof(ITEM *));
    MENU *menu;
    for (int i = 0; i < numOptions; i++) {
        menuItems[i] = new_item(menuOptions[i], "");
    }
    menuItems[numOptions] = NULL;
    menu = new_menu(menuItems);

    // create the menu window
    WINDOW *menuWin = newwin(10, 30, LINES / 2 - 5, COLS / 2 - 15);
    keypad(menuWin, TRUE);

    set_menu_win(menu, menuWin);
    set_menu_sub(menu, derwin(menuWin, 6, 28, 2, 1));
    set_menu_format(menu, 5, 1);
    set_menu_mark(menu, "> ");

    post_menu(menu);
    wrefresh(menuWin);

    int choice;
    bool running = true;
    
    // logic for the choices
    while (running && (choice = wgetch(menuWin))) {
        switch (choice) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
            case 10:  // enter key
                if (item_index(current_item(menu)) == 0) {
                    // 25/5 POMODORO selected
                    startPomodoro(25 * 60, 5 * 60);
                } else if (item_index(current_item(menu)) == 1) {
                    // 50/10 POMODORO selected
                    startPomodoro(50 * 60, 10 * 60);
                } else if (item_index(current_item(menu)) == 2) {
                    // CUSTOM selected, call custom pomodoro function
                    startCustomPomodoro();
                } else if (item_index(current_item(menu)) == 3) {
                    // QUIT selected
                    running = false;
                }
                break;
        }
        wrefresh(menuWin);
    }

    // clean up everything
    unpost_menu(menu);
    free_menu(menu);
    for (int i = 0; i < numOptions; i++) {
        free_item(menuItems[i]);
    }
    delwin(menuWin);

    endwin();

    return 0;
}
