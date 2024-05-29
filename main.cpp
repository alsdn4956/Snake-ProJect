#include <ncurses.h>
#include <chrono>
#include <string>
#include <ctime>
#include <fstream>
#include <deque>
#include <cstring>
#include <unistd.h> 
#include <stdlib.h>
#include "Game.cpp"
using namespace std;

int getms() {
    return chrono::duration_cast<chrono::milliseconds> (
            chrono::system_clock::now().time_since_epoch()
        ).count();
}

int main() {
    // Ncurses setting
    WINDOW* game_win;
    WINDOW* score_win;
    WINDOW* mission_win;

    initscr();
    start_color();
    
    curs_set(0);
    noecho();
    const char * text = "Drink Thief";

    init_pair(TITLE1, COLOR_MAGENTA, COLOR_WHITE);
    int xMax, yMax;

    bkgd(COLOR_PAIR(TITLE1));

    getmaxyx(stdscr, yMax, xMax);

    // Start animation part

    for (int i = yMax; i > (yMax/2)-5; i--) {
        clear();
        refresh();

        attron(COLOR_PAIR(TITLE1));
        attron(A_BOLD);
        mvprintw(i, (xMax/2)-25, "  _________ _______      _____   ____  __.___________");
        mvprintw(i+1, (xMax/2)-25, " /   _____/ \\      \\    /  _  \\ |    |/ _|\\_   _____/");
        mvprintw(i+2, (xMax/2)-25, " \\_____  \\  /   |   \\  /  /_\\  \\|      <   |    __)_ ");
        mvprintw(i+3, (xMax/2)-25, " /        \\/    |    \\/    |    \\    |  \\  |        \\");
        mvprintw(i+4, (xMax/2)-25, "/_______  /\\____|__  /\\____|__  /____|__ \\/_______  /");
        mvprintw(i+5, (xMax/2)-25, "        \\/         \\/         \\/        \\/        \\/ ");

        mvprintw(i+7, (xMax/2)-25, "                    <Climbing a ladder>                        ");

        attroff(A_BOLD);
        refresh();

        usleep(108013);
    }

    attron(COLOR_PAIR(TITLE1));
    mvprintw(yMax-1, xMax-strlen(text), text);
    attroff(COLOR_PAIR(TITLE1));
    refresh();

    attron(COLOR_PAIR(TITLE1));
    attron(A_BOLD);
    attron(A_UNDERLINE);
    mvprintw((yMax/2) + 7, (xMax/2) - 15, "Press Space Bar to start game");

    while (true) {
        int ch = getch();
        if (ch == 32) {
            clear();
            break;
        }
    }
    
    init_pair(BKGRD, COLOR_MAGENTA, COLOR_WHITE);
    border('*', '*', '*', '*', '*', '*', '*', '*');
    refresh();

    game_win = newwin(21, 42, 1, 1);
    wrefresh(game_win);

    init_pair(INFO, COLOR_WHITE, COLOR_MAGENTA);
    score_win = newwin(11, 30, 1, 47);
    wbkgd(score_win, COLOR_PAIR(INFO));
    wattron(score_win, COLOR_PAIR(INFO));
    wborder(score_win, ' ',' ',' ',' ',' ',' ',' ',' ');
    box(score_win, 0, 0);
    wrefresh(score_win);

    init_pair(MISSION, COLOR_WHITE, COLOR_CYAN);
    init_pair(MISSION_NOT_CLEARED, COLOR_WHITE, COLOR_RED);
    init_pair(MISSION_CLEARED, COLOR_WHITE, COLOR_GREEN);

    mission_win = newwin(9, 30, 13, 47);
    wbkgd(mission_win, COLOR_PAIR(MISSION));
    wborder(mission_win, ' ',' ',' ',' ',' ',' ',' ',' ');
    box(mission_win, 0, 0);
    wrefresh(mission_win);

    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    for(int i = 1; i<=5; i++){
        // Game setting
        Game game(game_win, score_win, mission_win, i);
        int lastinput = NONE;
        int lasttime = getms();
        game.init("maps/" + to_string(i));

        // Main loop
        while (true) {
            int inp = getch();
            if (inp == 110 || inp == 78) {
                break;
            }
            if (258<=inp && inp<=261) lastinput = inp - 258;
            int now = getms();
            int dt = now - lasttime;
            if (dt>=game.speed) {
                if (!game.tick(lastinput)) {
                    // Game over
                    break;
                }
                game.draw();

                wrefresh(game_win);
                lasttime = now;
                lastinput = NONE;
            }
        }

        // game_over || game_clear
        
        if(game.game_over){
            wclear(mission_win);
            mvwprintw(mission_win, 1, 8, "[ Game Over! ]");
            mvwprintw(mission_win, 3, 11, "ReStart?");
            mvwprintw(mission_win, 4, 13, "Y/N");
            wborder(mission_win, ' ',' ',' ',' ',' ',' ',' ',' ');
            box(mission_win, 0, 0);
            wrefresh(mission_win);
            int b;
            while(true){
                b = getch();
                if(b == 121 || b == 89){
                    i--;
                    break;
                }
                else if(b == 110 || b == 78) break;
            }
            if(b == 110 || b == 78) break; 
        }
        else if(i == 4){
            wclear(mission_win);
            mvwprintw(mission_win, 1, 7, "[ Game Clear! ]");
            mvwprintw(mission_win, 3, 11, "ReStart?");
            mvwprintw(mission_win, 4, 13, "YES / NO");
            wborder(mission_win, ' ',' ',' ',' ',' ',' ',' ',' ');
            box(mission_win, 0, 0);
            wrefresh(mission_win);
            int b;
            while(true){
                b = getch();
                if(b == 121 || b == 89){
                    i=0;
                    break;
                }
                else if(b == 110 || b == 78) break;
            }
            if (b == 110 || b == 78) break; 
        }

        getch();
    }
    delwin(game_win);
    endwin();
    return 0;
} 