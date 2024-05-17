#include <ncurses.h>
#include <chrono>
#include <string>
#include <ctime>
#include <fstream>
#include <deque>
#include <cstring>
#include <unistd.h>
#include <stdlib.h> 
using namespace std;

const int TITLE1 = 14;

const int MISSION = 11;
const int INFO = 10;
const int MISSION_NOT_CLEARED = 12;
const int MISSION_CLEARED = 13;

const int BKGRD = 16;
const int HEAD = 4;
const int BODY = 1;
const int WALL = 2;
const int IM_WALL = 3;
const int POISON = 5;
const int GROWTH = 6;
const int GATE1 = 7;
const int GATE2 = 8;
const int EMPTY = 9;

const int NONE = -1;
const int DOWN = 0;
const int UP = 1;
const int LEFT = 2;
const int RIGHT = 3;

const int CLOCKWISE[4] = {LEFT, RIGHT, UP, DOWN};

struct pos{
    int y;
    int x;
};


class Game{
public:
    int map[21][21];
    int going = NONE;
    int item_pos[100][2];
    pos gate_pos[2];
    int sc_growth = 0; int sc_poison = 0; int sc_gate = 0; 
    int item_tick;
    int mission[4];                                            // B,Grwoth,Poison,Gate
    bool game_over = false;
    int gate_cooltime = 0;
    int current_length = 0; int max_length = 0;
    int stage;
    int time_limit;
    int speed;
    int item_speed;
    int item_quantity;
    int difficult;

    deque<pos> body;
    WINDOW* win;
    WINDOW* info;
    WINDOW* miss;

    Game(WINDOW* win, WINDOW* info, WINDOW* miss, int stage) {
        this->win = win;
        this->info = info;
        this->miss = miss;
        this->stage = stage;
        init_pair(EMPTY, COLOR_WHITE, COLOR_WHITE);
        init_pair(HEAD, COLOR_BLUE, COLOR_BLUE);
        init_pair(BODY, COLOR_YELLOW, COLOR_YELLOW);
        init_pair(WALL, COLOR_CYAN, COLOR_CYAN);
        init_pair(IM_WALL, COLOR_CYAN, COLOR_CYAN);
        init_pair(GROWTH, COLOR_GREEN, COLOR_GREEN);
        init_pair(POISON, COLOR_RED, COLOR_RED);
        init_pair(GATE1, COLOR_MAGENTA, COLOR_MAGENTA);
        init_pair(GATE2, COLOR_MAGENTA, COLOR_MAGENTA);
    }

    int load_map(string uri) {
        ifstream readfile;
        readfile.open(uri);
        if (readfile.is_open()) {
            char tmp[64];
            for (int y=0; y<21; y++) {
                readfile.getline(tmp, 64);
                for (int x=0; x<21; x++) {
                    map[y][x] = tmp[x]-48;
                    if (map[y][x]==0) map[y][x]=EMPTY;
                    else if (map[y][x]==BODY || map[y][x]==HEAD) body.push_back(pos{y, x});
                }
            }
            readfile.getline(tmp, 64);
            speed = stoi(string(tmp));
            readfile.getline(tmp, 64);
            item_speed = stoi(string(tmp));
            readfile.getline(tmp, 64);
            item_quantity = stoi(string(tmp));
            readfile.getline(tmp, 64);
            difficult = stoi(string(tmp));
        } else {
            return 1;
        }
        return 0;
    }
    char* to_char(string a, char* b){
        strcpy(b,a.c_str());
        return b;
    }
    void add_mission(){
        srand((unsigned int)time(0));
        mission[0] = rand()%5 + 3 + difficult*2;
        mission[1] = rand()%3 + 1 + difficult;
        mission[2] = rand()%3 + 1 + difficult;
        mission[3] = rand()%2 + 1 + difficult;
        char b[20];
        time_limit = 550 - difficult*50;
        wclear(info);
        wborder(info, ' ',' ',' ',' ',' ',' ',' ',' ');
        box(info, 0, 0);
        mvwprintw(info, 1, 2, "[ Score Board ]");
        mvwprintw(info, 3, 2, "B :  3/3");
        mvwprintw(info, 4, 2, "+ :  0");
        mvwprintw(info, 5, 2, "- :  0");
        mvwprintw(info, 6, 2, "G :  0");
        mvwprintw(info, 8, 2, "[ Item Cooltime ]");
        string tmp = "Item : "+ to_string(item_tick);
        mvwprintw(info, 9, 2, to_char(tmp,b));
        wclear(miss);
        wborder(miss, ' ',' ',' ',' ',' ',' ',' ',' ');
        box(miss, 0, 0);

        wattron(miss, COLOR_PAIR(MISSION)); 
        mvwprintw(miss, 1, 2, "[ MISSION ]");
        wattroff(miss, COLOR_PAIR(MISSION)); 
        
        wattron(miss, COLOR_PAIR(MISSION)); tmp = "Time Limit : "+ to_string(time_limit);
        mvwprintw(miss, 3, 2, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION)); 

        wattron(miss, COLOR_PAIR(MISSION)); tmp = "B : "+ to_string(mission[0]);
        mvwprintw(miss, 4, 2, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION)); 
        
        wattron(miss, COLOR_PAIR(MISSION_NOT_CLEARED)); tmp = "(X)";
        mvwprintw(miss, 4, 10, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION_NOT_CLEARED)); 

        wattron(miss, COLOR_PAIR(MISSION)); tmp = "+ : "+ to_string(mission[1]);
        mvwprintw(miss, 5, 2, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION)); 

        wattron(miss, COLOR_PAIR(MISSION_NOT_CLEARED)); tmp = "(X)";
        mvwprintw(miss, 5, 10, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION_NOT_CLEARED)); 

        wattron(miss, COLOR_PAIR(MISSION)); tmp = "- : "+ to_string(mission[2]);
        mvwprintw(miss, 6, 2, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION)); 

        wattron(miss, COLOR_PAIR(MISSION_NOT_CLEARED)); tmp = "(X)";
        mvwprintw(miss, 6, 10, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION_NOT_CLEARED)); 

        wattron(miss, COLOR_PAIR(MISSION)); tmp = "G : "+ to_string(mission[3]);
        mvwprintw(miss, 7, 2, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION)); 

        wattron(miss, COLOR_PAIR(MISSION_NOT_CLEARED)); tmp = "(X)";
        mvwprintw(miss, 7, 10, to_char(tmp, b));
        wattroff(miss, COLOR_PAIR(MISSION_NOT_CLEARED)); 

        wrefresh(info);
        wrefresh(miss);
    }
    bool mission_check(int item){
        string tmp;
        char b[20];
        tmp = "B :  " + to_string(current_length) + "/" + to_string(max_length);
        
        mvwprintw(info, 3, 2, to_char(tmp, b));
        wattron(miss, COLOR_PAIR(MISSION_CLEARED)); 
        if(max_length == mission[0]) mvwprintw(miss, 4, 10, "(V)");
        wattroff(miss, COLOR_PAIR(MISSION_CLEARED)); 
        switch(item){
            case GROWTH:
                wattron(miss, COLOR_PAIR(INFO)); 
                tmp = "+ :  "+ to_string(sc_growth);
                mvwprintw(info, 4, 2, to_char(tmp, b));
                wattroff(miss, COLOR_PAIR(INFO)); 
                wattron(miss, COLOR_PAIR(MISSION_CLEARED)); 
                if(sc_growth == mission[1]) mvwprintw(miss, 5, 10, "(V)");
                wattroff(miss, COLOR_PAIR(MISSION_CLEARED)); 
                break;
            case POISON:
                wattron(miss, COLOR_PAIR(INFO));
                tmp = "- :  "+ to_string(sc_poison);
                mvwprintw(info, 5, 2, to_char(tmp, b));
                wattroff(miss, COLOR_PAIR(INFO)); 
                wattron(miss, COLOR_PAIR(MISSION_CLEARED)); 
                if(sc_poison == mission[2]) mvwprintw(miss, 6, 10, "(V)");
                wattroff(miss, COLOR_PAIR(MISSION_CLEARED)); 
                break;
            case GATE1:
            case GATE2:
                wattron(miss, COLOR_PAIR(INFO));
                tmp = "G :  "+ to_string(sc_gate);
                mvwprintw(info, 6, 2, to_char(tmp, b));
                wattroff(miss, COLOR_PAIR(INFO)); 
                wattron(miss, COLOR_PAIR(MISSION_CLEARED)); 
                if(sc_gate == mission[3]) mvwprintw(miss, 7, 10, "(V)");
                wattroff(miss, COLOR_PAIR(MISSION_CLEARED)); 
                break;
        }
        wattroff(miss, COLOR_PAIR(MISSION_CLEARED)); 
        if(mission[0] <= max_length && mission[1] <= sc_growth && mission[2] <= sc_poison && mission[3] <= sc_gate) return true;
        wrefresh(info);
        wrefresh(miss);
        return false;
    }
    void clear_item(){
        for(int i = 0; i<item_quantity; i++){
            if ((map[item_pos[i][0]][item_pos[i][1]] != BODY) || (map[item_pos[i][0]][item_pos[i][1]] != HEAD)) map[item_pos[i][0]][item_pos[i][1]] = EMPTY;
        }
        wrefresh(win);
    }
    void clear_gate(){
        map[gate_pos[0].y][gate_pos[0].x] = WALL;
        map[gate_pos[1].y][gate_pos[1].x] = WALL;
        wrefresh(win);
    }
    void generate_item(){
        srand((unsigned int)time(0));
        int percent = rand()%(item_quantity+1);
        for(int i = 0; i<percent; i++){
            while(true){
                int item_y = rand()%17 + 2; //1~19 random
                int item_x = rand()%17 + 2;
                if(map[item_y][item_x] == EMPTY){
                    map[item_y][item_x] = GROWTH;
                    item_pos[i][0] = item_y;
                    item_pos[i][1] = item_x;
                    break;
                }
            }
        } //growth
        for(int i = percent; i<item_quantity; i++){
            while(true){
                int item_y = rand()%17 + 2; //1~19 random
                int item_x = rand()%17 + 2;
                if(map[item_y][item_x] == EMPTY){
                    map[item_y][item_x] = POISON;
                    item_pos[i][0] = item_y;
                    item_pos[i][1] = item_x;
                    break;
                }
            }
        } //poision
    }  
    void generate_gate() {
        srand((unsigned int)time(0));
        int gate_y, gate_x;
        while(true){
            gate_y = rand()%21;
            gate_x = rand()%21;
            if(map[gate_y][gate_x] == WALL){
                gate_pos[0] = pos {gate_y, gate_x};
                break;
            }
        }
        map[gate_y][gate_x] = GATE1;
        while(true){
            gate_y = rand()%21;
            gate_x = rand()%21;
            if(map[gate_y][gate_x] == WALL){
                gate_pos[1] = pos {gate_y, gate_x};
                break;
            }
        }
        map[gate_y][gate_x] = GATE2;
    }    
    bool init(string map_uri) {
        load_map(map_uri);
        going = LEFT;
        current_length = 3;
        max_length = 3;
        item_tick = item_speed;
        generate_item();
        generate_gate();
        add_mission();
        return true;
    }
    bool tick(int lastinput) {
        if (item_tick-- == 0) {
            clear_item();
            generate_item();
            item_tick = item_speed;
        }
        string tmp = "Item : "+ to_string(item_tick) + "     ";
        char b[25];
        mvwprintw(info, 9, 2, to_char(tmp,b));
        if (gate_cooltime-- == 0) {
            clear_gate();
            generate_gate();
        }
        if(time_limit-- == 0){
            game_over = true;
            return false;
        }
        tmp = "Time Limit : "+ to_string(time_limit) + "   ";
        mvwprintw(miss, 3, 2, to_char(tmp, b));
        wrefresh(miss);
        wrefresh(info);
        return move(lastinput);
    }
    bool is_center(pos gate) {
        return !(gate.x==0 || gate.y==0 || gate.x==20 || gate.y==20);
    }
    bool using_gate(pos now_gate, pos other_gate) {
        bool ic = is_center(other_gate);
        if (ic) {
            while (true) {
                pos gate_go = pos {other_gate.y + (going==UP?-1:0) + (going==DOWN?1:0), other_gate.x + (going==LEFT?-1:0) + (going==RIGHT?1:0)};
                int go_tile = map[gate_go.y][gate_go.x];
                if (!((go_tile == GATE1) || (go_tile == GATE2) || (go_tile == WALL) || (go_tile == IM_WALL))) break;
                going = CLOCKWISE[going];
            }
        } else {
            if (other_gate.y==20) going = UP;
            else if (other_gate.y==0) going = DOWN;
            else if (other_gate.x==20) going = LEFT;
            else if (other_gate.x==0) going = RIGHT;
        }
        gate_cooltime = current_length-1;
        pos go = pos {other_gate.y + (going==UP?-1:0) + (going==DOWN?1:0), other_gate.x + (going==LEFT?-1:0) + (going==RIGHT?1:0)};
        if (can_go(go))
            return move_and_get_item(go);
        game_over = true;
        return false;
    }
    bool move(int direction) {
        switch (direction) {
            case UP:
                if (going != DOWN){
                    going = UP;
                    break;
                }
                else{
                    game_over = true;
                    return false;
                } 
            case DOWN:
                if (going != UP){
                    going = DOWN; 
                    break;
                } 
                else{
                    game_over = true;
                    return false;
                } 
            case LEFT:
                if (going != RIGHT){
                    going = LEFT;
                    break;
                }
                else{
                    game_over = true;
                    return false;
                } 
            case RIGHT:
                if (going != LEFT){
                    going = RIGHT;
                    break;
                } 
                else{
                    game_over = true;
                    return false;
                } 
        }
        if (going != NONE) {
            pos go = body.front();
            switch (going) {
                case UP:
                    go.y -= 1;
                    break;
                case DOWN:
                    go.y += 1;
                    break;
                case LEFT:
                    go.x -= 1;
                    break;
                case RIGHT:
                    go.x += 1;
                    break;
            }
            if (can_go(go)) {
                return move_and_get_item(go);
            } else {
                game_over = true;
                return false;
            }
        }
        return true;
    }
    bool move_and_get_item(pos go) {
        int go_tile = map[go.y][go.x];
        if (go_tile == EMPTY) {
            // Basic move
            pos tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;

            body.push_front(go);
            map[go.y][go.x] = HEAD;
            for (auto it = body.begin()+1; it != body.end(); it++) {
                pos bodys = *it;
                map[bodys.y][bodys.x] = BODY;
            }
        } else if (go_tile == GROWTH) {
            sc_growth += 1;
            current_length += 1;
            max_length = current_length>max_length?current_length:max_length;
            if(mission_check(go_tile)) return false;
            body.push_front(go);
            map[go.y][go.x] = HEAD;
            if (gate_cooltime>=0) gate_cooltime++;
            for (auto it = body.begin()+1; it != body.end(); it++) {
                pos bodys = *it;
                map[bodys.y][bodys.x] = BODY;
            }
        } else if (go_tile == POISON) {
            sc_poison += 1;
            current_length -= 1;
            if(mission_check(go_tile)) return false;
            pos tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;

            tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = EMPTY;
            
            body.push_front(go);
            map[go.y][go.x] = HEAD;
            for (auto it = body.begin()+1; it != body.end(); it++) {
                pos bodys = *it;
                map[bodys.y][bodys.x] = BODY;
            }
            if (current_length<3){
                game_over = true;
                return false;
            }
            if (gate_cooltime>0) gate_cooltime--;
        } else if (go_tile == GATE1 || go_tile == GATE2) {
            sc_gate += 1;
            if(mission_check(go_tile)) return false;
            pos now_gate = gate_pos[go_tile-GATE1];
            pos other_gate = gate_pos[GATE2-go_tile];
            return using_gate(now_gate, other_gate);
        }
        return true;
    }
    bool can_go(pos go) {
        if ((map[go.y][go.x] == WALL) || (map[go.y][go.x] == IM_WALL) || (map[go.y][go.x] == BODY)){
            game_over = true;
            return false;
        }
        return true;
    }
    void draw() {
        for (int y=0; y<21; y++) {
            for (int x=0; x<21; x++) {
                int now = map[y][x];
                wattron(win, COLOR_PAIR(now));
                mvwprintw(win, y, x*2, "aa");
                wattroff(win, COLOR_PAIR(now));
            }
        }
        wrefresh(win);
    }
};
