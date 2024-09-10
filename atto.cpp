// ncursesw expects that
#define _XOPEN_SOURCE_EXTENDED 1
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <functional>
#include <signal.h>
#include <errno.h>
#include <cstring>

#include <curses.h>

#include "keybinds.hpp"

void line_numbers_draw(std::size_t offset_y, std::size_t max_y)
{
    for (std::size_t k {0}; k < max_y; ++k) {
	// print line numbers starting with number 1
	std::size_t n = k + 1;
	if (max_y < 1000 && n < 10) {
	    mvprintw(k + offset_y, 0, "  %ld |", n);
	    continue;
	}
	if ((max_y < 1000 && n < 100) || (max_y < 100 && n < 10)) {
	    mvprintw(k + offset_y, 0, " %ld |", n);
	    continue;
	}
	mvprintw(k + offset_y, 0, "%ld |", n);
    }
}

typedef std::function<void(std::size_t, std::size_t, std::size_t)> cmd_t;

std::unordered_map<wint_t, cmd_t> keymap {
    {L'\n', key::newline},
    {KEY_ENTER, key::newline},
    {KEY_BACKSPACE, key::backspace},
    {KEY_DC, key::del},
    {KEY_END, key::end},
    {KEY_HOME, key::home},

    {KEY_UP, key::uparr},
    {KEY_DOWN, key::downarr},
    {KEY_LEFT, key::leftarr},
    {KEY_RIGHT, key::rightarr},

    {WEOF, [](std::size_t y, std::size_t x, std::size_t z) {
	(void) z;
	getyx(stdscr, y, x);
	mvprintw(0, 0, "# how did you even press WEOF?");
	move(y, x);
    }},

    {0x03 /* C-c */, key::shutdown},
    {0x13 /* C-s */, key::save}
};

void work()
{
    std::size_t max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    std::size_t line_num_off_x = line_numbers_offset_x(max_y);
    mvprintw(0, 0, "# welcome to atto, a cmdline editor. "
	     "press C-s to save or C-c to quit");
    mvprintw(1, 0, "# stats: max_y = %ld, max_x = %ld, line_num_off_x = %ld",
	     max_y, max_x, line_num_off_x);

    line_numbers_draw(MODELINE_SIZE, max_y);
    
    wint_t wi;
    if (get_wch(&wi) == ERR) {
	throw std::runtime_error {"could not get keypress"};
    }

    if (erase() != OK) {
	throw std::runtime_error {"could not erase the screen"};
    }

    std::size_t y = MODELINE_SIZE, x;
    line_numbers_draw(y, max_y);
    x = line_num_off_x = line_numbers_offset_x(max_y);
    move(y, line_num_off_x);
    do {
	getyx(stdscr, y, x);
	// clear modeline
	move(0, 0);
	if (clrtoeol() != OK) {
	    throw std::runtime_error {"could not clear modeline"};
	}
	move(y, x);
	if (keymap.find(wi) == keymap.end()) {
	    // just insert it
	    cchar_t cc {};
	    cc.chars[0] = (wchar_t)wi;
	    if (add_wch(&cc) != OK) {
		throw std::runtime_error {"could not print character"};
	    }
	    getyx(stdscr, y, x);
	    mvprintw(0, 0, "# pressed %d", wi);
	    move(y, x);
	    continue;
	}
	keymap[wi](y, x, line_num_off_x);
    } while (get_wch(&wi) != ERR); // cancel with C-c
}

int main()
{
    setlocale(LC_ALL, "");
    initscr();
    raw();
    noecho();

    try {
	if (keypad(stdscr, true) != OK) {
	    throw std::runtime_error {"could not activate keypad mode"};
	}
	work();
    }
    catch (const std::exception& e) {
	std::clog << "ERROR: " << e.what() << std::endl;
    }

    endwin();
    return 0;
}
