/*
 * Для компиляции необходимо добавить ключ -lncurses
 * g++ -o snake main.cpp -lncurses
 */

#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <ncurses.h>

enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME='q'};
enum {MAX_TAIL_SIZE=1000, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10, SPEED=20000, SEED_NUMBER=3};

struct Tail {
    int x, y;
};

struct Food {
    int x, y;
    time_t put_time;
    char point;
    bool enable;
};

class Snake {
public:
    int x, y, direction;
    size_t tsize;
    char head, body;
    std::vector<Tail> tail;

    Snake() : x(0), y(2), direction(RIGHT), tsize(START_TAIL_SIZE+1) {
        tail.resize(MAX_TAIL_SIZE);
    }

    void head_and_body(char c1, char c2) {
	head = c1;
	body = c2;
    }

    void move() {
        int max_x = 0, max_y = 0;
        getmaxyx(stdscr, max_y, max_x);
        mvprintw(y, x, " ");

        switch (direction) {
            case LEFT:
                if (x <= 0) x = max_x;
                mvprintw(y, --x, "%c", head);
                break;
            case RIGHT:
                if (x >= max_x) x = 0;
                mvprintw(y, ++x, "%c", head);
                break;
            case UP:
                if (y <= 0) y = max_y;
                mvprintw(--y, x, "%c", head);
                break;
            case DOWN:
                if (y >= max_y) y = 0;
                mvprintw(++y, x, "%c", head);
                break;
        }
        refresh();
    }

    void moveTail() {
        mvprintw(tail[tsize - 1].y, tail[tsize - 1].x, " ");
        for (size_t i = tsize - 1; i > 0; i--) {
            tail[i] = tail[i - 1];
            if (tail[i].y || tail[i].x) {
                mvprintw(tail[i].y, tail[i].x, "%c", body);
            }
        }
        tail[0].x = x;
        tail[0].y = y;
    }

    void addTail() {
        if (tsize < MAX_TAIL_SIZE) {
            tsize++;
        } else {
            mvprintw(0, 0, "Can't add tail");
        }
    }

    bool isCrash() {
        for (size_t i = 1; i < tsize; ++i) {
            if (x == tail[i].x && y == tail[i].y) return true;
        }
        return false;
    }
};

class Work_with_food {
    std::vector<Food> food;
public:
    
    Work_with_food() {
	food.resize(MAX_FOOD_SIZE);
	int max_y = 0, max_x = 0;
    	getmaxyx(stdscr, max_y, max_x);
    	for (auto& f : food) {
     	   f = {0, 0, 0, '$', false};
	}
        putFood();
    }

    void putFoodSeed(Food& fp) {
    	int max_x = 0, max_y = 0;
    	getmaxyx(stdscr, max_y, max_x);
    	mvprintw(fp.y, fp.x, " ");
    	fp.x = rand() % (max_x - 1);
    	fp.y = rand() % (max_y - 2) + 1;
    	fp.put_time = time(nullptr);
    	fp.enable = true;
    	mvprintw(fp.y, fp.x, "%c", fp.point);
    }

    void putFood() {
    	for (auto& f : food) {
    	    putFoodSeed(f);
    	}
    }

    void refreshFood() {
    	for (auto& f : food) {
            if (f.put_time && (!f.enable || (time(nullptr) - f.put_time) > FOOD_EXPIRE_SECONDS)) {
            	putFoodSeed(f);
            }
    	}
    }

    bool haveEat(Snake& snake) {
    	for (auto& f : food) {
            if (f.enable && snake.x == f.x && snake.y == f.y) {
            	f.enable = false;
                return true;
            }
    	}
    	return false;
    }

    void repairSeed(Snake& snake) {
    	for (size_t i = 0; i < snake.tsize; i++) {
            for (auto& f : food) {
            	if (f.x == snake.tail[i].x && f.y == snake.tail[i].y && f.enable) {
                    putFoodSeed(f);
            	}
            }
    	}
    }
};

class Game {
    Work_with_food meal;
    Snake snake;
    int key_pressed;
   
    void changeDirection(int& new_direction, int key) {
	switch (key) {
	    case KEY_DOWN: new_direction = DOWN; break;
	    case KEY_UP: new_direction = UP; break;
            case KEY_LEFT: new_direction = LEFT; break;
            case KEY_RIGHT: new_direction = RIGHT; break;
	}
    }
   
    void printLevel() {
        int max_x = 0, max_y = 0;
	getmaxyx(stdscr, max_y, max_x);
	mvprintw(0, max_x - 10, "LEVEL: %zu", snake.tsize);
    }
   
    void printExit() {
    	int max_x = 0, max_y = 0;
    	getmaxyx(stdscr, max_y, max_x);
    	mvprintw(max_y / 2, max_x / 2 - 5, "Your LEVEL is %zu", snake.tsize);
    }

    void end_game() {
	printExit();
	timeout(SPEED);
	getch();
	endwin();
    }

public:

    Game() {
        snake.head_and_body('@', '*');
	key_pressed = 0;
	srand(time(nullptr));
	
	initscr();
	keypad(stdscr, TRUE);
	raw();
	noecho();
	curs_set(FALSE);
	
	mvprintw(0, 0, " Use arrows for control. Press 'q' for EXIT");
	timeout(0);
    }

    void run() {
        while (key_pressed != STOP_GAME) {
            key_pressed = getch();
            changeDirection(snake.direction, key_pressed);
        
            if (snake.isCrash()) break;

            snake.move();
            snake.moveTail();

            if (meal.haveEat(snake)) {
            	snake.addTail();
            	printLevel();
            }

            meal.refreshFood();
            meal.repairSeed(snake);

            timeout(100);
        }
	end_game();
    }
};

int main() {
    Game game_in_snake;
    game_in_snake.run();

    return 0;
}
