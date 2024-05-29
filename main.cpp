#include "Game.cpp"
#include <chrono>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <ncurses.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace std;

int getms() {
    return chrono::duration_cast<chrono::milliseconds>(
               chrono::system_clock::now().time_since_epoch())
        .count();
}

int main() {
    // Ncurses setting
    WINDOW *game_window;
    WINDOW *score_window;
    WINDOW *mission_window;

    initscr();
    start_color();

    curs_set(0);
    noecho();

    const char *text = "Climbing a ladder";

    init_pair(TITLE1, COLOR_GREEN, COLOR_WHITE);

    int max_x, max_y;

    bkgd(COLOR_PAIR(TITLE1));

    getmaxyx(stdscr, max_y, max_x);

    // Start animation part

    for (int i = max_y; i > (max_y / 2) - 5; i--) {
        clear();
        refresh();

        attron(COLOR_PAIR(TITLE1));
        attron(A_BOLD);
        mvprintw(i, (max_x / 2) - 25,
                 "  _________ _______      _____   ____  __.___________");
        mvprintw(i + 1, (max_x / 2) - 25,
                 " /   _____/ \\      \\    /  _  \\ |    |/ _|\\_   _____/");
        mvprintw(i + 2, (max_x / 2) - 25,
                 " \\_____  \\  /   |   \\  /  /_\\  \\|      <   |    __)_ ");
        mvprintw(i + 3, (max_x / 2) - 25,
                 " /        \\/    |    \\/    |    \\    |  \\  |        \\");
        mvprintw(i + 4, (max_x / 2) - 25,
                 "/_______  /\\____|__  /\\____|__  /____|__ \\/_______  /");
        mvprintw(i + 5, (max_x / 2) - 25,
                 "        \\/         \\/         \\/        \\/        \\/ ");
        mvprintw(i + 7, (max_x / 2) - 25,
                 "                  <Climbing a ladder>                        ");

        attroff(A_BOLD);
        refresh();

        usleep(108013);
    }

    attron(COLOR_PAIR(TITLE1));
    mvprintw(max_y - 1, max_x - strlen(text), text);
    attroff(COLOR_PAIR(TITLE1));
    refresh();

    attron(COLOR_PAIR(TITLE1));
    attron(A_BOLD);
    attron(A_UNDERLINE);
    mvprintw((max_y / 2) + 7, (max_x / 2) - 15, "Press Space Bar to start game");

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

    game_window = newwin(21, 42, 1, 1);
    wrefresh(game_window);

    init_pair(INFO, COLOR_WHITE, COLOR_MAGENTA);
    score_window = newwin(11, 30, 1, 47);
    wbkgd(score_window, COLOR_PAIR(INFO));
    wattron(score_window, COLOR_PAIR(INFO));
    wborder(score_window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    box(score_window, 0, 0);
    wrefresh(score_window);

    init_pair(MISSION, COLOR_WHITE, COLOR_CYAN);
    init_pair(MISSION_NOT_CLEARED, COLOR_WHITE, COLOR_RED);
    init_pair(MISSION_CLEARED, COLOR_WHITE, COLOR_GREEN);

    mission_window = newwin(9, 30, 13, 47);
    wbkgd(mission_window, COLOR_PAIR(MISSION));
    wborder(mission_window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    box(mission_window, 0, 0);
    wrefresh(mission_window);

    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    for (int i = 1; i <= 5; i++) {
        // Game setting
        Game game(game_window, score_window, mission_window, i);
        int last_input = NONE;
        int last_time = getms();

        game.init("maps/" + to_string(i));

        // Main loop
        while (true) {
            int inp = getch();

            if (inp == 110 || inp == 78) {
                break;
            }

            if (258 <= inp && inp <= 261) {
                last_input = inp - 258;
            }

            int now = getms();
            int dt = now - last_time;
            
            if (dt >= game.speed) {
                if (!game.tick(last_input)) {
                    // Game over
                    break;
                }

                game.draw_board();

                wrefresh(game_window);
                last_time = now;
                last_input = NONE;
            }
        }

        // game_over || game_clear
        bool game_clear = (i == 4);

        if (game.game_over) {
            wclear(mission_window);
            mvwprintw(mission_window, 1, 6, "[ Game Over... ]");
            mvwprintw(mission_window, 3, 11, "ReStart?");
            mvwprintw(mission_window, 4, 10, "[Y] / [N]");
            wborder(mission_window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
            box(mission_window, 0, 0);
            wrefresh(mission_window);

            int b;
            
            while (true) {
                b = getch();

                if (b == 121 || b == 89) {
                    i--;
                    break;
                }
                else if (b == 110 || b == 78) {
                    break;
                }
            }

            if (b == 110 || b == 78) {
                break;
            }
        } else if (game_clear) {
            wclear(mission_window);
            mvwprintw(mission_window, 1, 7, "[ Game Clear! ]");
            mvwprintw(mission_window, 3, 11, "ReStart?");
            mvwprintw(mission_window, 4, 8, "[Y] / [N]");
            wborder(mission_window, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
            box(mission_window, 0, 0);
            wrefresh(mission_window);
 
            int b;
            
            while (true) {
                b = getch();

                if (b == 121 || b == 89) {
                    i = 0;
                    break;
                } else if (b == 110 || b == 78) {
                    break;
                }
            } if (b == 110 || b == 78) {
                break;
            }
        }

        getch();
    }

    delwin(game_window);
    endwin();

    return 0;
}