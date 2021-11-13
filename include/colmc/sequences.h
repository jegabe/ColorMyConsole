// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#ifndef colmc_sequences_h_INCLUDED
#define colmc_sequences_h_INCLUDED

#include <string>
#include <sstream>

#include <colmc/push_warnings.h>

// All of the following constants are just plain
// old ANSI escape sequences. These are here just for
// convenience. Of course, they can also be hard coded by the
// library user.

namespace colmc {

constexpr char reset_all[]  = "\x1B[0m";

namespace fore {

constexpr char bright[]     = "\x1B[1m";
constexpr char dim[]        = "\x1B[2m";
constexpr char normal[]     = "\x1B[22m";

constexpr char black[]      = "\x1B[30m";
constexpr char red[]        = "\x1B[31m";
constexpr char green[]      = "\x1B[32m";
constexpr char yellow[]     = "\x1B[33m";
constexpr char blue[]       = "\x1B[34m";
constexpr char magenta[]    = "\x1B[35m";
constexpr char cyan[]       = "\x1B[36m";
constexpr char white[]      = "\x1B[37m";
constexpr char reset[]      = "\x1B[39m";

}

namespace back {

constexpr char black[]      = "\x1B[40m";
constexpr char red[]        = "\x1B[41m";
constexpr char green[]      = "\x1B[42m";
constexpr char yellow[]     = "\x1B[43m";
constexpr char blue[]       = "\x1B[44m";
constexpr char magenta[]    = "\x1B[45m";
constexpr char cyan[]       = "\x1B[46m";
constexpr char white[]      = "\x1B[47m";
constexpr char reset[]      = "\x1B[49m";

}

//! \brief x and y are zero based! ANSI is one-based, so this
//! function adds one
inline std::string goto_xy(int x, int y) {
	std::ostringstream oss;
	if ((x >= 0) && (y >= 0)) {
		oss << "\x1B[" << (y + 1) << ';' << (x + 1) << 'H';
	}
	return oss.str();
}

inline std::string up(int n) {
	std::ostringstream oss;
	if (n > 0) {
		oss << "\x1B[" << n << 'A';
	}
	return oss.str();
}

inline std::string down(int n) {
	std::ostringstream oss;
	if (n > 0) {
		oss << "\x1B[" << n << 'B';
	}
	return oss.str();
}

inline std::string forward(int n) {
	std::ostringstream oss;
	if (n > 0) {
		oss << "\x1B[" << n << 'C';
	}
	return oss.str();
}

inline std::string backward(int n) {
	std::ostringstream oss;
	if (n > 0) {
		oss << "\x1B[" << n << 'D';
	}
	return oss.str();
}

enum class clear_screen_mode: int {
	from_cursor_to_end_of_screen   = 0,
	from_begin_of_screen_to_cursor = 1,
	entire_screen                  = 2
};

inline std::string clear_screen(clear_screen_mode mode = clear_screen_mode::entire_screen) {
	std::ostringstream oss;
	oss << "\x1B[" << static_cast<int>(mode) << 'J';
	return oss.str();
}

enum class clear_line_mode: int {
	from_cursor_to_end_of_line   = 0,
	from_begin_of_line_to_cursor = 1,
	entire_line                  = 2
};

inline std::string clear_line(clear_line_mode mode = clear_line_mode::entire_line) {
	std::ostringstream oss;
	oss << "\x1B[" << static_cast<int>(mode) << 'K';
	return oss.str();
}

}

#include <colmc/pop_warnings.h>

#endif
