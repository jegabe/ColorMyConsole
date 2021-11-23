// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#include <iostream>
#include <colmc/setup.h>
#include <colmc/sequences.h>

using namespace colmc;

int main() {
	colmc::config cfg;
	cfg.allow_styles = true;
	colmc::setup(cfg);
	colmc::add_style("red", colmc::fore::red);
	colmc::add_style("green_on_blue", std::string{colmc::back::blue} + colmc::fore::green);
	std::cout << "Normal <red> Red <green_on_blue> GreenOnBlue </> Red </> Normal" << std::endl;
	std::cout << "Normal '<' signs cannot harm." << std::endl;
	std::cin.get();
	return 0;
}
