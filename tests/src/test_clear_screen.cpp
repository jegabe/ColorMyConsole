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
	std::cout << "############################################################" << std::endl;
	std::cout << "############################################################" << std::endl;
	std::cout << "############################################################" << std::endl;
	std::cout << "############################################################" << std::endl;
	std::cout << "############################################################" << std::endl;
	std::cout << goto_xy(0, 0) << "Press any key and screen will be cleared from cursor to end of screen.";
	(void)get_key();
	std::cout << clear_screen(clear_screen_mode::from_cursor_to_end_of_screen) << std::flush;
	std::cout << "Press any key and screen will be cleared from begin of screen up to cursor";
	(void)get_key();
	std::cout << clear_screen(clear_screen_mode::from_begin_of_screen_to_cursor) << std::flush;
	std::cout << "Press any key and screen will be cleared entirely, resetting cursor to (0,0)";
	(void)get_key();
	std::cout << clear_screen(clear_screen_mode::entire_screen) << std::flush;
	std::cout << "Press any key to terminate.";
	(void)get_key();
	return 0;
}
