#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>


int x = 60;
int i;


int countdown(int countTime) {
	for (i = 0; ++i; --countTime){
		//printf("%i\n", countTime);
		//char countTimeStr[10];
		//snprintf(countTimeStr, sizeof(countTimeStr), "%d", countTime);
		//addstr(countTimeStr);
		//addch('a');
		clear();
		printw("%i\n", countTime);
		refresh();
		sleep(1);
		
		
	}
	return 0;
}










int moving_and_sleeping()
{
    int row = 50;
    int col = 5;

    curs_set(0);

    for(char c = 65; c <= 90; c++)
    {
        move(row++, col++);
        addch(c);
        refresh();
        napms(100);
    }

    row = 5;
    col = 3;

    for(char c = 97; c <= 122; c++)
    {
        mvaddch(row++, col++, c);
        refresh();
        napms(100);
    }

    curs_set(1);

    addch('\n');
	return 0;
}
















int main() {
	initscr();
	addstr("Welcome to Pomodoro!!\n");
	//addstr("%i\n", countTime);

	//printf("Welcome to Pomodoro!!\n");
	//countdown(x);
	moving_and_sleeping();
	return 0;
}



    //int row = 5;
    //int col = 0;
//
    //curs_set(0);
//
    //for(char c = 65; c <= 90; c++)
    //{
    //    move(row++, col++);
    //    addch(c);
    //    refresh();
    //    napms(100);
    //}