#ifndef KEYBINDS_HPP_
#define KEYBINDS_HPP_

#include <chrono>
#include <thread>
#include <curses.h>

#define MODELINE_SIZE 2

static inline std::size_t line_numbers_offset_x(std::size_t max_y)
{
    std::size_t i = 5;
    std::size_t y = max_y;
    while (y / 10 > 10) {
	y /= 10;
	++i;
    }
    return i;
}

static inline std::size_t get_eol_x()
{
    std::size_t y, x , m_y, m_x;
    getmaxyx(stdscr, m_y, m_x);
    (void) m_y;
    getyx(stdscr, y, x);
    const std::size_t r_y = y;
    const std::size_t r_x = x;
    move(y, --m_x);
    while ((inch() & A_CHARTEXT) == ' ') {
	move(y, --m_x);
    }
    ++m_x;
    move(r_y, r_x);
    return m_x;
}

static inline bool is_line_empty()
{
    std::size_t y, m_y, r_x, m_x, line_num_off_x;
    getmaxyx(stdscr, m_y, m_x);
    line_num_off_x = line_numbers_offset_x(m_y);
    getyx(stdscr, y, r_x);
    m_x = get_eol_x();
    for (std::size_t x {line_num_off_x}; x < m_x; ++x) {
	move(y, x);
	if ((inch() & A_CHARTEXT) != ' ') {
	    return false;
	}
    }
    move(y, r_x);
    return true;
}

static inline std::size_t get_last_line()
{
    std::size_t y, x , m_y, m_x;
    getmaxyx(stdscr, m_y, m_x);
    getyx(stdscr, y, x);
    while (m_y) {
	move(m_y, 0);
	if (!is_line_empty()) {
	    move(y, x);
	    return m_y;
	}
	--m_y;
    }
    move(y, x);
    return m_y;
}

static inline void move_cursor_to_eol()
{
    std::size_t y, x;
    getyx(stdscr, y, x);
    x = get_eol_x();
    move(y, x);
}

static inline void cap_max_y(std::size_t& y)
{
    std::size_t m_y, m_x;
    getmaxyx(stdscr, m_y, m_x);
    (void) m_x;
    if (m_y <= y)
	y = m_y - 1;
}
namespace key {
    static inline void newline(std::size_t y, std::size_t x,
			       std::size_t line_num_off_x)
    {
	(void) x;
	mvprintw(0, 0, "# pressed ENTER");
	cap_max_y(++y);
	move(y, line_num_off_x);
    }
    static inline void backspace(std::size_t y, std::size_t x,
				 std::size_t line_num_off_x)
    {
	if (--x < line_num_off_x) {
	    if (y <= MODELINE_SIZE) {
		y = MODELINE_SIZE;
		return;
	    }
	    move(--y, line_num_off_x);
	    move_cursor_to_eol();
	    return;
	}
	move(y, x);
	cchar_t c {};
	c.chars[0] = L' ';
	if (add_wch(&c) != OK) {
	    throw std::runtime_error {"could not delete char"};
	}
	move(y, x);
    }
    static inline void del(std::size_t y, std::size_t x,
			   std::size_t line_num_off_x)
    {
	(void) line_num_off_x;
	if (get_eol_x() <= x) {
	    return;
	}
	cchar_t c {};
	c.chars[0] = L' ';
	if (add_wch(&c) != OK) {
	    throw std::runtime_error {"could not delete char"};
	}
	move(y, ++x);
    }
    
    static inline void uparr(std::size_t y, std::size_t x,
			     std::size_t line_num_off_x)
    {
	(void) line_num_off_x;
	if (--y <= MODELINE_SIZE)
	    y = MODELINE_SIZE;
	mvprintw(0, 0, "# pressed UP");
	move(y, x);
	std::size_t eol = get_eol_x();
	if (x < eol)
	    eol = x;
	move(y, eol);
    }
    static inline void downarr(std::size_t y, std::size_t x,
			       std::size_t line_num_off_x)
    {
	(void) line_num_off_x;
	mvprintw(0, 0, "# pressed DOWN");
	cap_max_y(++y);
	move(y, x);
	std::size_t eol = get_eol_x();
	if (x < eol)
	    eol = x;
	move(y, eol);
    }
    static inline void leftarr(std::size_t y, std::size_t x,
			       std::size_t line_num_off_x)
    {
	mvprintw(0, 0, "# pressed LEFT");
	if (--x < line_num_off_x)
	    x = line_num_off_x;
	move(y, x);
    }
    static inline void rightarr(std::size_t y, std::size_t x,
				std::size_t line_num_off_x)
    {
	(void) line_num_off_x;
	mvprintw(0, 0, "# pressed RIGHT");
	move(y, ++x);
	std::size_t eol = get_eol_x();
	if (x > eol)
	    move(y, eol);
    }
    static inline void end(std::size_t y, std::size_t x,
			   std::size_t line_num_off_x)
    {
	(void) y; (void) x; (void) line_num_off_x;
	move_cursor_to_eol();
    }
    static inline void home(std::size_t y, std::size_t x,
			    std::size_t line_num_off_x)
    {
	(void) x;
	move(y, line_num_off_x);
    }
    static inline void shutdown(std::size_t y, std::size_t x, std::size_t z)
    {
	(void) y; (void) x; (void) z;
	throw std::system_error {EINTR, std::system_category(), strerror(EINTR)};
    }
    static inline std::string prompt_for_filename()
    {
	std::size_t m_y, m_x;
	getmaxyx(stdscr, m_y, m_x);
	std::size_t box_size_y {m_y / 2};
	std::size_t box_size_x {m_x / 2};
	std::size_t min_box_size_y {5};
	std::size_t min_box_size_x {30};
	if (box_size_y < min_box_size_y) {
	    if (m_y < min_box_size_y) {
		throw std::runtime_error {"window is too small!"};
	    }
	    box_size_y = min_box_size_y;
	}
	if (box_size_x < min_box_size_x) {
	    if (m_x < min_box_size_x) {
		throw std::runtime_error {"window is too small!"};
	    }
	    box_size_x = min_box_size_x;
	}
	std::size_t box_pos_y {(m_y - box_size_y) / 2};
	std::size_t box_pos_x {(m_x - box_size_x) / 2};
	auto* win = newwin(box_size_y, box_size_x, box_pos_y, box_pos_x);
	if (!win) {
	    throw std::runtime_error {"could not create window"};
	}
	if (keypad(win, true) != OK) {
	    throw std::runtime_error {"could not activate keypad mode"};
	}
	box(win, 0, 0);
	if (box_size_y > min_box_size_y) {
	    mvwprintw(win, 1, 2, "Save buffer as:");
	    mvwprintw(win, 3, 2, "_________________________");
	    mvwprintw(win, 5, 2, "Save <RET>   Cancel <ESC>");
	    wmove(win, 3, 2);
	}
	if (redrawwin(win) != OK) {
	    throw std::runtime_error {"could not refresh window"};
	}
	wint_t wi;
	cchar_t cc {};
	std::string str {};
	bool escaped {false};
	while (wget_wch(win, &wi) != ERR) {
	    if (wi == KEY_ENTER) {
		// ENTER was pressed
		break;
	    }
	    else if (wi == 0x1b && wget_wch(win, &wi) == ERR) {
		// ESC pressed
		escaped = true;
		break;
	    }
	    // ALT + char or just char pressed, ignore ALT keypresses
	    cc.chars[0] = static_cast<wchar_t>(wi);
	    str += cc.chars[0];
	    if (wadd_wch(win, &cc) != OK) {
		// printing char failed
		break;
	    }
	    if (wrefresh(win) != OK) {
		break;
	    }
	}
	if (delwin(win) != OK) {
	    throw std::runtime_error {"could not delete window"};
	}
	win = nullptr;
	if (redrawwin(stdscr) != OK) {
	    throw std::runtime_error {"could not refresh window"};
	}
	if (escaped) {
	    return "";
	}
	return str;
    }
    static inline void save(std::size_t y, std::size_t x,
			    std::size_t)
    {
	getyx(stdscr, y, x);
	std::string str {prompt_for_filename()};
	mvprintw(0, 0, "# filename: %s", str.c_str());
	move(y, x);
    }
}
#endif // KEYBINDS_HPP_
