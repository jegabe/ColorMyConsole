// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#include <iostream>
#include <colmc/setup.h>
#include <colmc/sequences.h>

using namespace colmc;

int main() {
	setup();
	for (;;) {
		std::cout << "Please enter a X coordinate (or '-1' to leave) >";
		int x;
		std::cin >> x;
		if (x < 0) {
			break;
		}
		std::cout << "Please enter a Y coordinate (or '-1' to leave) >";
		int y;
		std::cin >> y;
		if (y < 0) {
			break;
		}
		std::cout << goto_xy(x, x) << "Now at X=" << x << ", Y=" << y;
	}
}
