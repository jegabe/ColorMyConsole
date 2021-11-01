// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#include <iostream>
#include <colmc/setup.h>
#include <colmc/sequences.h>

using namespace colmc;

struct color {
	const char* esc_sequence;
	const char* name;
};

const struct color bg_colors[] = {
	{ back::black,   "black  " },
	{ back::red,     "red    " },
	{ back::green,   "green  " },
	{ back::yellow,  "yellow " },
	{ back::blue,    "blue   " },
	{ back::magenta, "magenta" },
	{ back::cyan,    "cyan   " },
	{ back::white,   "white  " }
};

const struct color fg_colors[] = {
	{ fore::black,   "black  " },
	{ fore::red,     "red    " },
	{ fore::green,   "green  " },
	{ fore::yellow,  "yellow " },
	{ fore::blue,    "blue   " },
	{ fore::magenta, "magenta" },
	{ fore::cyan,    "cyan   " },
	{ fore::white,   "white  " }
};


int main() {
	setup();
	std::cout << "        ";
	for (std::size_t fg_index = 0; fg_index < (sizeof(fg_colors)/sizeof(fg_colors[0])); ++fg_index) {
		std::cout << fg_colors[fg_index].esc_sequence << fg_colors[fg_index].name;
	}
	std::cout << std::endl;
	for (std::size_t bg_index = 0; bg_index < (sizeof(bg_colors)/sizeof(bg_colors[0])); ++bg_index) {
		std::cout << bg_colors[bg_index].esc_sequence << fore::reset << bg_colors[bg_index].name;
		for (std::size_t fg_index = 0; fg_index < (sizeof(fg_colors)/sizeof(fg_colors[0])); ++fg_index) {
			std::cout << back::reset << " ";
			std::cout << bg_colors[bg_index].esc_sequence;
			std::cout << fg_colors[fg_index].esc_sequence <<  fore::dim << "X " << fore::normal << "X " << fore::bright << "X " << reset_all;
		}
		std::cout << std::endl;
	}
	std::cout << reset_all << "Press return to terminate";
	std::cin.get();
	return 0;
}
