// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#include <iostream>
#include <colmc/setup.h>
#include <colmc/raw_input.h>

using namespace colmc;

int main() {
	config cfg;
	cfg.raw_input_mode = true;
	setup(cfg);
	for (;;) {
		std::cout << "Press a key (press Z to terminate)" << std::endl;
		const auto evt = get_key();
		if ((evt == 'Z')) {
			break;
		}
		std::cout << "  You pressed '" << evt << "'" << std::endl;
	}
}
