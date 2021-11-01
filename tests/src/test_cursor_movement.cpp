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
	std::cout << "Use the arrow keys to move the cursor around. Press 'Z' to terminate." << std::endl;
	for (;;) {
		const auto pressed = get_key();
		if (pressed == 'Z') {
			break;
		}
		if (pressed == key_enum::up) {
			std::cout << up(1);
		}
		else if (pressed == key_enum::down) {
			std::cout << down(1);
		}
		else if (pressed == key_enum::right) {
			std::cout << forward(1);
		}
		else if (pressed == key_enum::left) {
			std::cout << backward(1);
		}
	}
}
