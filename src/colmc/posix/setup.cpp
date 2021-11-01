// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#if defined(__unix__) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))

#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <cstring>
#include <cassert>
#include <mutex>
#include <iostream>
#include <colmc/setup.h>
#include <colmc/raw_input.h>
#include <colmc/term_size.h>

using namespace colmc;

namespace {

bool raw_input_mode = false;
bool stdout_redirected = false;
bool is_setup = false;
termios old_terminal_settings;
termios new_terminal_settings;
int push_back_ch = -1;
std::mutex push_back_lock;

std::size_t bytes_available() {
	std::unique_lock<std::mutex> lock{push_back_lock};
	int n = 0;
	if (ioctl(STDIN_FILENO, FIONREAD, &n) < 0) {
		n = 0;
	}
	if (push_back_ch != -1) {
		++n;
	}
	return static_cast<std::size_t>(n);
}

int read_ch() {
	std::unique_lock<std::mutex> lock{push_back_lock};
	if (push_back_ch != -1) {
		const auto result = push_back_ch;
		push_back_ch = -1;
		return result;
	}
	unsigned char ch;
	if (read(STDIN_FILENO, &ch, 1) == 1) {
		const auto result = static_cast<int>(ch);
		return result;
	}
	return -1;
}

void push_back(char c) {
	std::unique_lock<std::mutex> lock{push_back_lock};
	assert(push_back_ch == -1);
	push_back_ch = static_cast<unsigned int>(c);
}

char itoc(int x) {
	return static_cast<char>(static_cast<unsigned char>(x));
}

}

namespace colmc {

void teardown();

void setup(config cfg) {
	if (is_setup) {
		return;
	}
	raw_input_mode = cfg.raw_input_mode;
	stdout_redirected = (::isatty(STDIN_FILENO) == 0);
	if (stdout_redirected) {
		raw_input_mode = false;
	}
	if (raw_input_mode) {
		tcgetattr(STDIN_FILENO, &old_terminal_settings);
		new_terminal_settings = old_terminal_settings;
		new_terminal_settings.c_lflag &= ~ICANON;
		new_terminal_settings.c_lflag &= ~ECHO;
		new_terminal_settings.c_lflag &= ~ISIG;
		new_terminal_settings.c_cc[VMIN] = 1;
		new_terminal_settings.c_cc[VTIME] = 0;
		tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal_settings);
		push_back_ch = -1;
	}
	std::atexit(teardown);
	is_setup = true;
}

void teardown() {
	if (raw_input_mode) {
		tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal_settings);
		std::memset(&old_terminal_settings, 0, sizeof(old_terminal_settings));
		push_back_ch = -1;
		tcflush(STDIN_FILENO, TCIFLUSH);
	}
	raw_input_mode = false;
	stdout_redirected = false;
	is_setup = false;
}

bool key_pressed() {
	if (!raw_input_mode) {
		return false;
	}
	return (bytes_available() > 0);
}

key get_key(bool block_until_pressed) {
	key result;
	if (!raw_input_mode) {
		return result; // default constructor is "no key pressed"
	}
	auto avail = bytes_available();
	if ((avail == 0) && (!block_until_pressed)) {
		return result; // default constructor is "no key pressed"
	}
	std::cout.flush();
	const int first_ch = read_ch();
	if (first_ch < 0) {
		return result; // error: there are bytes available but read() could't fetch them
	}
	if (first_ch < 128) {
		if (first_ch != '\x1B') { // regular non-esc ASCII char (UTF-8 sequence of length 1)
			result.special = key_enum::regular;
			result.regular.bytes[0] = itoc(first_ch);
			result.regular.bytes[1] = '\0';
			return result;
		}
		// esc handling. 2 cases: ESC plus '[' or single escape followed by sth. else
		avail = bytes_available();
		if (avail == 0) { // single escape without further sequence
			result.special = key_enum::regular;
			result.regular.bytes[0] = '\x1B';
			result.regular.bytes[1] = '\0';
			return result;
		}
		const int second_ch = read_ch();
		if (second_ch < 0) {
			return result; // error: there are bytes available but read() could't fetch them
		}
		if (second_ch != '[') { // no escape sequence: So we return esc and push_back the other char for next read
			push_back(itoc(second_ch));
			result.special = key_enum::regular;
			result.regular.bytes[0] = '\x1B';
			result.regular.bytes[1] = '\0';
			return result;
		}
		avail = bytes_available();
		if (avail < 1) { // escape '[' without anything behind?
			result.special = key_enum::unknown;
			return result;
		}
		const int third_ch = read_ch();
		if (third_ch < 0) {
			return result; // error: there are bytes available but read() could't fetch them
		}
		switch(third_ch) {
			case /* ESC [ */ 'A': result.special = key_enum::up; break;
			case /* ESC [ */ 'B': result.special = key_enum::down; break;
			case /* ESC [ */ 'C': result.special = key_enum::right; break;
			case /* ESC [ */ 'D': result.special = key_enum::left; break;
			case /* ESC [ */ 'H': result.special = key_enum::home; break;
			case /* ESC [ */ 'F': result.special = key_enum::end; break;
			default: {
				avail = bytes_available();
				if (avail < 1) { // escape sequence without anything behind?
					result.special = key_enum::unknown;
					return result;
				}
				const int fourth_ch = read_ch();
				if (fourth_ch < 0) {
					return result; // error: there are bytes available but read() could't fetch them
				}
				if (fourth_ch != '~') {
					result.special = key_enum::unknown;
					return result;
				}
				switch(third_ch) {
					case /* ESC [ */ '2' /* ~ */: result.special = key_enum::insert; break;
					case /* ESC [ */ '3' /* ~ */: result.special = key_enum::del; break;
					case /* ESC [ */ '5' /* ~ */: result.special = key_enum::page_up; break;
					case /* ESC [ */ '6' /* ~ */: result.special = key_enum::page_down; break;
					default:  result.special = key_enum::unknown; break;
				}
			}
		}
		return result;
	}
	// first_ch >= 128: multi-byte UTF-8 sequence
	avail = bytes_available() + 1u; // +1: first char already read
	std::size_t needed_avail;
	if ((static_cast<unsigned>(first_ch) & 0xE0) == 0xC0) { // 2-byte UTF-8 sequence
		needed_avail = 2;
	}
	else if ((static_cast<unsigned>(first_ch) & 0xF0) == 0xE0) { // 3-byte UTF-8 sequence
		needed_avail = 3;
	}
	else {
		needed_avail = 4;
	}
	if (avail < needed_avail) { // not a full UTF-8 in buffer? Don't know how to deal with that...
		result.special = key_enum::unknown;
		return result;
	}
	result.special = key_enum::regular;
	int bytes[4] = {'\0', '\0', '\0', '\0'};
	bytes[0] = first_ch;
	bytes[1] = read_ch();
	if (needed_avail > 2) {
		bytes[2] = read_ch();
	}
	if (needed_avail > 3) {
		bytes[3] = read_ch();
	}
	for (std::size_t i=0; i<4; ++i) {
		const auto c = bytes[i];
		if (c == -1) {
			result.special = key_enum::unknown;
			return result;
		}
		result.regular.bytes[i] = itoc(c);
	}
	return result;
}

terminal_size estimate_terminal_size(const terminal_size& default_if_not_gettable) {
	terminal_size result = default_if_not_gettable;
	if (!stdout_redirected) {
		struct winsize w;
		int ioctl_result = ::ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
		std::cout << "ioctl returned " << ioctl_result << std::endl;
		result.columns = w.ws_col;
		result.rows = w.ws_row;
	}
	return result;
}

}

#endif
