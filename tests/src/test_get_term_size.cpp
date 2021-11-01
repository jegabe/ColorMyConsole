// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#include <iostream>
#include <colmc/setup.h>
#include <colmc/term_size.h>

using namespace colmc;

int main() {
	setup();
	const auto term_size = estimate_terminal_size();
	std::cout << "The terminal size is " << term_size.rows << " rows and " << term_size.columns << " columns." << std::endl;
	return 0;
}
