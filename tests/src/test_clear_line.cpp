// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#include <iostream>
#include <colmc/setup.h>
#include <colmc/sequences.h>
#include <colmc/raw_input.h>

using namespace colmc;

int main() {
	config cfg;
	cfg.raw_input_mode = true;
	setup(cfg);
	std::cout << goto_xy(0, 0) << "Press any key and line will be cleared from cursor to end of line" << backward(11);
	(void)get_key();
	std::cout << clear_line(clear_line_mode::from_cursor_to_end_of_line) << std::flush;
	std::cout << "\nPress any key and line will be cleared from begin of line to cursor" << backward(10);
	(void)get_key();
	std::cout << clear_line(clear_line_mode::from_begin_of_line_to_cursor) << std::flush;
	std::cout << "\nPress any key and line will be cleared entirely";
	(void)get_key();
	std::cout << clear_line(clear_line_mode::entire_line) << std::flush;
	std::cout << "\nPress any key to terminate.";
	(void)get_key();
	return 0;
}
