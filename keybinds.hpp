#ifndef KEYBINDS_HPP_
#define KEYBINDS_HPP_

#include <curses.h>

#define MODELINE_SIZE 2

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
}
#endif // KEYBINDS_HPP_
