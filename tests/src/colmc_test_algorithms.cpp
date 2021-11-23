// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#include <iostream>
#include <colmc/algorithms.h>

using namespace colmc;

int main() {
	int result = 0;
	{ // replacement same size
		std::vector<char> v { 'a', 'b', 'c', 'd', 'e', 'f', '\0', '\0' };
		std::size_t num_of_used_bytes = 6;
		replace_content(v, num_of_used_bytes, 2, 2, "gh", 2, 0);
		std::vector<char> expected { 'a', 'b', 'g', 'h', 'e', 'f', '\0', '\0' };
		if (v != expected) {
			std::cout << "line " << __LINE__  << ": v is not equal to expected";
			result = 1;
		}
		if (num_of_used_bytes != 6) {
			std::cout << "line " << __LINE__  << ": num_of_used_bytes is wrong";
			result = 1;
		}
	}
	{ // replacement bigger
		std::vector<char> v { 'a', 'b', 'c', 'd', 'e', 'f' };
		std::size_t num_of_used_bytes = 6;
		replace_content(v, num_of_used_bytes, 2, 2, "ghij", 4, 6);
		std::vector<char> expected { 'a', 'b', 'g', 'h', 'i', 'j', 'e', 'f', '\0', '\0', '\0', '\0' };
		if (v != expected) {
			std::cout << "line " << __LINE__  << ": v is not equal to expected";
			result = 1;
		}
		if (num_of_used_bytes != 8) {
			std::cout << "line " << __LINE__  << ": num_of_used_bytes is wrong";
			result = 1;
		}
	}
	{ // replacement smaller
		std::vector<char> v { 'a', 'b', 'c', 'd', 'e', 'f' };
		std::size_t num_of_used_bytes = 6;
		replace_content(v, num_of_used_bytes, 2, 4, "gh", 2, 0);
		std::vector<char> expected { 'a', 'b', 'g', 'h', 'e', 'f' };
		if (v != expected) {
			std::cout << "line " << __LINE__  << ": v is not equal to expected";
			result = 1;
		}
		if (num_of_used_bytes != 4) {
			std::cout << "line " << __LINE__  << ": num_of_used_bytes is wrong";
			result = 1;
		}
	}
	if (result == 0) {
		std::cout << "All tests passed." << std::endl;
	}
	else {
		std::cout << "Some tests failed." << std::endl;
	}
	std::cout << "Press return to terminate." << std::endl;
	std::cin.get();
	return result;
}
