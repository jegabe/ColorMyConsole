// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX

#include <streambuf>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <cassert>
#include <Windows.h>
#include <io.h> 
#include <fcntl.h>
#include <conio.h>
#include <colmc/setup.h>
#include <colmc/raw_input.h>
#include <colmc/term_size.h>
#include <colmc/algorithms.h>

using namespace colmc;

namespace {

constexpr std::size_t min_buf_size = 16u;
constexpr std::size_t parsed_params_capacity = 4u;
constexpr std::size_t default_buf_size = 256u;
constexpr std::size_t buf_growth = 256u;
constexpr char esc = '\x1B';
bool is_setup = false;
bool stdout_redirected = false;
bool win_utf8 = false;
bool allow_styles = false;
int old_cin_mode = -1;
int old_cout_mode = -1;
std::unique_ptr<std::basic_streambuf<char>> cout_buf;
std::unique_ptr<std::basic_streambuf<char>> cin_buf;
std::basic_streambuf<char>* old_cout_buf = nullptr;
std::basic_streambuf<char>* old_cin_buf = nullptr;
HANDLE h_console = nullptr;
CONSOLE_SCREEN_BUFFER_INFO initial_console_settings;
DWORD old_console_mode = 0;
bool raw_input_mode = false;
std::unordered_map<std::string, std::string> styles;
std::vector<std::string> style_stack;
std::mutex style_mutex;

bool is_stdout_redirected() {
	DWORD temp;
	const BOOL success = ::GetConsoleMode(h_console, &temp);
	return (success == 0); // failure indicates redirected stream
}

class ostreambuf : public std::basic_streambuf<char>
{
public:
	using base = std::basic_streambuf<char>;
	virtual ~ostreambuf() {}
	
	explicit ostreambuf(std::size_t buf_size)
		:m_buf(buf_size, '\0')
	{
		m_previous_text_attributes = initial_console_settings.wAttributes;
		if (buf_size < min_buf_size) {
			m_buf.resize(min_buf_size);
		}
		m_parsed_params.reserve(parsed_params_capacity);
		reset_region();
	}

protected:

	// derived classes implement it for std::wcout and std::cout forwarding
	virtual void output(const char* p, std::size_t n) = 0;

	void output(char c) {
		output(&c, 1u);
	}

	// called in case the buffer is too small before a flush()/std::endl call.
	// Calls should be very rare because a terminal line length is restricted
	// and the default buf size should be sufficient for most cases.
	int_type overflow(int_type ch = std::char_traits<char>::eof()) override {
		if (ch == std::char_traits<char>::eof()) {
			return ch;
		}
		const auto next_char = static_cast<char>(ch);
		m_buf.back() = next_char;
		const std::size_t old_size = m_buf.size();
		m_buf.resize(old_size + buf_growth); // make buffer bigger
		// give client new write area in the new space, leaving one byte for the next overflow
		base::setp(m_buf.data() + old_size, m_buf.data() + m_buf.size() - 1u);
		return 0;
	}

	// called when data should appear on screen. This is where escape sequences are filtered
	// and handled specially
	int sync() override {
		handle(static_cast<std::size_t>(pptr() - m_buf.data()));
		reset_region();
		return 0;
	}

	void reset_region() {
		base::setp(m_buf.data(), m_buf.data() + m_buf.size() - 1u); // -1u so that overflow() can put the next char before handle()
	}

	void handleStyleTags(std::size_t& num_of_chars) {
		std::unique_lock<std::mutex> lock{style_mutex};
		std::size_t n = num_of_chars;
		std::size_t i = 0;
		while(n > 0) {
			const char* p = m_buf.data() + i;
			const std::size_t pos = index_of(p, '<', n);
			if (pos == no_pos) {
				break; // all style tags handled, job finished
			}
			const std::size_t remaining = (n - pos);
			if (remaining < 3) { // 3: minimum a '<', then a single char or '/', then '>
				i += (pos + 1);
				n -= (pos + 1);
				continue;
			}
			std::size_t end = find_end_of_style_sequence(p + pos, remaining);
			if (end == invalid_end_of_sequence) { // it wasn't a sequence but a regular '>' or '<' inside text...
				i += (pos + 1);
				n -= (pos + 1);
				continue;
			}
			end += pos; // convert relative end to absolute end
			assert(p[pos] == '<');
			assert(p[end-1] == '>');
			// [pos, end) is the range of the style tag
			const bool is_end_style = ((end-pos) > 2) && (p[pos+1] == '/');
			std::string sequence = "\x1B[0m"; // reset style sequence
			if (!is_end_style) {
				const auto key = std::string{p + pos + 1, end - pos - 2 };
				const auto style = styles.find(key);
				if (style == styles.end()) {
					++p;
					--n;
					continue;
				}
				sequence += style->second;
				style_stack.push_back(key);
			}
			else {
				if (!style_stack.empty()) {
					style_stack.resize(style_stack.size() - 1u);
				}
				if (!style_stack.empty()) {
					sequence += styles[style_stack.back()];
				}
			}
			const auto num_of_chars_before = num_of_chars;
			colmc::replace_content(m_buf, num_of_chars, i + pos, end-pos, sequence.c_str(), sequence.size(), buf_growth);
			if (num_of_chars > num_of_chars_before) {
				n += (num_of_chars - num_of_chars_before);
			}
			else {
				n -= (num_of_chars_before - num_of_chars);
			}
			i += (pos + sequence.size()); // continue searching after the pasted sequence in m_buf
			n -= (pos + sequence.size());
		}
	}

	void handle(std::size_t num_of_chars) {
		if (allow_styles) {
			handleStyleTags(num_of_chars);
		}
		const char* begin = m_buf.data();
		while (num_of_chars > 0) {
			std::size_t n = count_until_esc(begin, num_of_chars);
			if (n > 0) {
				output(begin, n);
				begin += n;
				num_of_chars -= n;
			}
			if (num_of_chars == 0) {
				break;
			}
			if (begin[0] == esc) {
				if ((num_of_chars > 2u) && (begin[1] == '[')) {
					n = find_end_of_esc_sequence(begin, num_of_chars);
					if (n == invalid_end_of_sequence) {
						// output the invalid sequence so at least it is obvious that it's wrong
						output(begin, n);
						begin += n;
						num_of_chars -= n;
						continue;
					}
					if (!handle_esc_sequence(begin, n)) {
						// something is wrong with the sequence
						output(begin, n);
					}
					begin += n;
					num_of_chars -= n;
					continue;
				}
				// else:
				output(esc);
				++begin;
				--num_of_chars;
				continue;
			}
			assert(false); // line shouldn't be reached
			output(begin[0]);
			++begin;
			--num_of_chars;
			continue;
		}
	}

	std::size_t count_until_esc(const char* p, std::size_t n) const {
		std::size_t i = 0;
		while((i < n) && (p[i] != esc)) {
			++i;
		}
		return i;
	}

	bool handle_esc_sequence(const char* p, std::size_t n) {
		m_parsed_params.resize(0);
		assert(n > 3u);
		p += 2u; // after esc [
		n -= 2u;
		const char command = p[n-1];
		n--; // don't need to parse that any more
		while(n > 0) { // rest is parameters
			if (p[0] == ';') { // param separator
				++p;
				--n;
				continue;
			}
			char* end_ptr = nullptr;
			const int param = static_cast<int>(std::strtol(p, &end_ptr, 10));
			if (end_ptr > p) { // number parsed successfully
				const auto num_parsed = static_cast<std::size_t>(end_ptr - p);
				m_parsed_params.push_back(param);
				p += num_parsed;
				n -= num_parsed;
			}
			else {
				return false;
			}
		}
		if (m_parsed_params.empty()) {
			m_parsed_params.push_back(0); // when no params specified, the default is zero
		}
		switch(command) {
			case 'm': return handle_color_sequence();
			case 'H': // fall through
			case 'f': return handle_cursor_position();
			case 'A': // fall through
			case 'B': // fall through
			case 'C': // fall through
			case 'D': return handle_cursor_movement(command);
			case 'J': return clear_screen();
			case 'K': return clear_line();
			default:
				break;
		}
		return false;
	}

	bool handle_color_sequence() {
		WORD attributes = m_previous_text_attributes;
		for (int param: m_parsed_params) {
			switch(param) {
				case 0: // reset all
					attributes = initial_console_settings.wAttributes;
					break;
				case 1: // bright
					attributes |= FOREGROUND_INTENSITY;
					break;
				case 2: // dim, not supported by Windows, so use normal brightness and fall through:
				case 22: // normal brightness
					attributes &= ~FOREGROUND_INTENSITY; // DIM (2) not supported in Windows, let it look like normal intensity
					break;
				case 30: // fg black
					attributes &= ~(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
					break;
				case 31: // fg red
					attributes &= ~(FOREGROUND_BLUE | FOREGROUND_GREEN);
					attributes |= FOREGROUND_RED;
					break;
				case 32: // fg green
					attributes &= ~(FOREGROUND_BLUE | FOREGROUND_RED);
					attributes |= FOREGROUND_GREEN;
					break;
				case 33: // fg yellow
					attributes &= ~FOREGROUND_BLUE;
					attributes |= (FOREGROUND_RED | FOREGROUND_GREEN);
					break;
				case 34: // fg blue
					attributes &= ~(FOREGROUND_RED | FOREGROUND_GREEN);
					attributes |= FOREGROUND_BLUE;
					break;
				case 35: // fg magenta
					attributes &= ~FOREGROUND_GREEN;
					attributes |= (FOREGROUND_RED | FOREGROUND_BLUE);
					break;
				case 36: // fg cyan
					attributes &= ~FOREGROUND_RED;
					attributes |= (FOREGROUND_GREEN | FOREGROUND_BLUE);
					break;
				case 37: // fg white
					attributes |= (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
					break;
				case 39: // fg reset
					attributes &= ~(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
					attributes |= (initial_console_settings.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE));
					break;
				case 40: // bg black
					attributes &= ~(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
					break;
				case 41: // bg red
					attributes &= ~(BACKGROUND_GREEN | BACKGROUND_BLUE);
					attributes |= BACKGROUND_RED;
					break;
				case 42: // bg green
					attributes &= ~(BACKGROUND_RED | BACKGROUND_BLUE);
					attributes |= BACKGROUND_GREEN;
					break;
				case 43: // bg yellow
					attributes &= ~BACKGROUND_BLUE;
					attributes |= (BACKGROUND_RED | BACKGROUND_GREEN);
					break;
				case 44: // bg blue
					attributes &= ~(BACKGROUND_RED | BACKGROUND_GREEN);
					attributes |= BACKGROUND_BLUE;
					break;
				case 45: // bg magenta
					attributes &= ~BACKGROUND_GREEN;
					attributes |= (BACKGROUND_RED | BACKGROUND_BLUE);
					break;
				case 46: // bg cyan
					attributes &= ~BACKGROUND_RED;
					attributes |= (BACKGROUND_GREEN | BACKGROUND_BLUE);
					break;
				case 47: // bg white
					attributes |= (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
					break;
				case 49: // bg reset
					attributes &= ~(BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
					attributes |= (initial_console_settings.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE));
					break;
				default:
					return false; // param not understood
			}
		}
		::SetConsoleTextAttribute(h_console, attributes);
		m_previous_text_attributes = attributes;
		return true;
	}

	bool handle_cursor_position() {
		if (m_parsed_params.size() != 2u) {
			return false;
		}
		int y = m_parsed_params[0];
		int x = m_parsed_params[1];
		if ((y < 0) || (x < 0)) {
			return false;
		}
		if (x == 0) {
			x = 1; // ANSI is 1-based, but 0 work also as 1 (tried on Linux console)
		}
		if (y == 0) {
			y = 1; // ANSI is 1-based, but 0 work also as 1 (tried on Linux console)
		}
		CONSOLE_SCREEN_BUFFER_INFO info;
		::GetConsoleScreenBufferInfo(h_console, &info); // Use that to adjust for scrolling position
		COORD c;
		c.X = static_cast<SHORT>(x - 1) + info.srWindow.Left;
		c.Y = static_cast<SHORT>(y - 1) + info.srWindow.Top;
		::SetConsoleCursorPosition(h_console, c);
		return true;
	}

	bool handle_cursor_movement(char command) {
		if (m_parsed_params.size() != 1u) {
			return false;
		}
		const auto param = static_cast<SHORT>(m_parsed_params[0]);
		CONSOLE_SCREEN_BUFFER_INFO info;
		::GetConsoleScreenBufferInfo(h_console, &info);
		const auto max_y = info.srWindow.Bottom - info.srWindow.Top;
		const auto max_x = info.srWindow.Right - info.srWindow.Left;
		auto pos = info.dwCursorPosition;
		switch(command) {
			case 'A': {
				if (pos.Y >= param) {
					pos.Y -= param;
				}
				else {
					pos.Y = 0;
				}
				break;
			}
			case 'B': {
				if ((pos.Y + param) <= max_y) {
					pos.Y += param;
				}
				else
				{
					pos.Y = max_y;
				}
				break;
			}
			case 'C': {
				if ((pos.X + param) <= max_x) {
					pos.X += param;
				}
				else
				{
					pos.X = max_x;
				}
				break;
			}
			case 'D': {
				if (pos.X >= param) {
					pos.X -= param;
				}
				else {
					pos.X = 0;
				}
				break;
			}
			default: return false;
		}
		::SetConsoleCursorPosition(h_console, pos);
		return true;
	}

	bool clear_screen() {
		if (m_parsed_params.size() != 1u) {
			return false;
		}
		const auto param = m_parsed_params[0];
		CONSOLE_SCREEN_BUFFER_INFO info;
		::GetConsoleScreenBufferInfo(h_console, &info);
		const auto cells_in_screen = static_cast<int>(info.dwSize.X) * static_cast<int>(info.dwSize.Y);
		const auto cells_before_cursor = info.dwSize.X * info.dwCursorPosition.Y + info.dwCursorPosition.X;
		COORD from;
		int cells_to_erase = 0;
		switch(param) {
			case 0: from = info.dwCursorPosition; cells_to_erase = cells_in_screen - cells_before_cursor; break;
			case 1: from.X = from.Y = 0; cells_to_erase = cells_before_cursor; break;
			case 2: from.X = from.Y = 0; cells_to_erase = cells_in_screen; break;
			default: return false;
		}
		DWORD num_written = 0;
		::FillConsoleOutputCharacterW(h_console, L' ', static_cast<DWORD>(cells_to_erase), from, &num_written);
		if (param == 2) {
			COORD c;
			c.X = info.srWindow.Left;
			c.Y = info.srWindow.Top;
			::SetConsoleCursorPosition(h_console, c);
		}
		return true;
	}

	bool clear_line() {
		if (m_parsed_params.size() != 1u) {
			return false;
		}
		const auto param = m_parsed_params[0];
		CONSOLE_SCREEN_BUFFER_INFO info;
		::GetConsoleScreenBufferInfo(h_console, &info);
		COORD from;
		int cells_to_erase = 0;
		switch(param) {
			case 0: from = info.dwCursorPosition; cells_to_erase = info.dwSize.X - info.dwCursorPosition.X; break;
			case 1: from.X = 0; from.Y = info.dwCursorPosition.Y; cells_to_erase = info.dwCursorPosition.X; break;
			case 2: from.X = 0; from.Y = info.dwCursorPosition.Y; cells_to_erase = info.dwSize.X; break;
			default: return false;
		}
		DWORD num_written = 0;
		::FillConsoleOutputCharacterW(h_console, L' ', static_cast<DWORD>(cells_to_erase), from, &num_written);
		::FillConsoleOutputAttribute(h_console, m_previous_text_attributes, static_cast<DWORD>(cells_to_erase), from, &num_written);
		return true;
	}

	std::vector<char> m_buf;
	std::vector<int> m_parsed_params; // kept as member to avoid repetitive allocations
	WORD m_previous_text_attributes = 0;
};

class ostreambuf_to_wcout: public ostreambuf {
public:
	ostreambuf_to_wcout(std::size_t buf_size)
		:ostreambuf(buf_size)
	{
		m_translated_buf.resize(m_buf.size());
	}

	virtual ~ostreambuf_to_wcout() {
		std::wcout.flush();
	}

protected:
	void output(const char* p, std::size_t n) override {
		if (n == 0) {
			return;
		}
		auto result = ::MultiByteToWideChar(CP_UTF8, 0, p, static_cast<int>(n), m_translated_buf.data(), static_cast<int>(m_translated_buf.size()));
		if (result == 0) { // call failed because buffer is too small
			const auto needed = static_cast<std::size_t>(::MultiByteToWideChar(CP_UTF8, 0, p, static_cast<int>(n), nullptr, 0));
			m_translated_buf.resize(needed);
			// this time it won't fail:
			result = ::MultiByteToWideChar(CP_UTF8, 0, p, static_cast<int>(n), m_translated_buf.data(), static_cast<int>(needed));
		}
		std::wcout.write(m_translated_buf.data(), static_cast<std::streamsize>(result));
		std::wcout.flush();
	}

	std::vector<wchar_t> m_translated_buf; // kept as member to avoid repetitive allocations
};

class ostreambuf_to_cout: public ostreambuf {
public:
	using ostreambuf::ostreambuf; // inherit ctor

	virtual ~ostreambuf_to_cout() {
		old_cout_buf->pubsync();
	}

protected:
	void output(const char* p, std::size_t n) override {
		while(n > 0) {
			assert(old_cout_buf != nullptr);
			const auto num_written = old_cout_buf->sputn(p, static_cast<std::streamsize>(n));
			if (num_written >= 0) {
				old_cout_buf->pubsync();
				p += static_cast<std::size_t>(num_written);
				n -= static_cast<std::size_t>(num_written);
			}
			else {
				return;
			}
		}
	}
};

class istreambuf: public std::basic_streambuf<char> {
public:
	using base = std::basic_streambuf<char>;

	istreambuf() {
		base::setg(m_utf8_buf, m_utf8_buf, m_utf8_buf); // zero bytes available
	}

protected:

	virtual void read_next_unicode_char() = 0;

	int_type underflow() override {
		read_next_unicode_char();
		base::setg(m_utf8_buf, m_utf8_buf, m_utf8_buf + m_utf8_size);
		return static_cast<int_type>(static_cast<unsigned char>(m_utf8_buf[0]));
	}

	char m_utf8_buf[4u]; // 4 because that's the max. length of a unicode char encoded in UTF-8
	std::size_t m_utf8_size = 0;
};

class istreambuf_from_wcin: public istreambuf {
public:
	using istreambuf::istreambuf;

protected:

	virtual void read_next_unicode_char() {
		wchar_t c;
		std::wcin.get(c);
		const auto result = ::WideCharToMultiByte(CP_UTF8, 0, &c, 1, m_utf8_buf, sizeof(m_utf8_buf), nullptr, nullptr);
		assert((result >= 1) && (result <= 4));
		m_utf8_size = static_cast<std::size_t>(result);
	}
};

}

namespace colmc {

void teardown();

void setup(config cfg) {
	if (is_setup) {
		return;
	}
	win_utf8 = cfg.win_utf8;
	h_console = ::GetStdHandle(STD_OUTPUT_HANDLE);
	stdout_redirected = is_stdout_redirected();
	if (!stdout_redirected) {
		if (cfg.raw_input_mode) {
			::GetConsoleMode(h_console, &old_console_mode);
			::SetConsoleMode(h_console, old_console_mode & (~ENABLE_ECHO_INPUT)); // turn echo off
			raw_input_mode = true;
		}
		::GetConsoleScreenBufferInfo(h_console, &initial_console_settings);
		if (win_utf8) {
			// Make std::wcin/wcout also handle non-ASCII correctly
			old_cin_mode = _setmode(_fileno(stdin), _O_WTEXT);
			old_cout_mode = _setmode(_fileno(stdout), _O_WTEXT);
			// translate UTF-8 to UTF-16 and use std::wcout as output:
			cout_buf = std::make_unique<ostreambuf_to_wcout>(default_buf_size);
			old_cout_buf = std::cout.rdbuf(cout_buf.get());
			cin_buf = std::make_unique<istreambuf_from_wcin>();
			old_cin_buf = std::cin.rdbuf(cin_buf.get());
		}
		else {
			cout_buf = std::make_unique<ostreambuf_to_cout>(default_buf_size);
			old_cout_buf = std::cout.rdbuf(cout_buf.get());
		}
		allow_styles = cfg.allow_styles;
	}
	std::atexit(teardown);
	is_setup = true;
}

void teardown() {
	if (!stdout_redirected) {
		// restore old console setting
		if (old_cin_buf != nullptr) {
			std::cin.rdbuf(old_cin_buf);
		}
		if (old_cout_buf != nullptr) {
			std::cout.rdbuf(old_cout_buf);
		}
		cin_buf.reset();
		cout_buf.reset();
		old_cin_buf = nullptr;
		old_cout_buf = nullptr;
		if (win_utf8) {
			_setmode(_fileno(stdin), old_cin_mode);
			_setmode(_fileno(stdout), old_cout_mode);
			win_utf8 = false;
		}
		::SetConsoleTextAttribute(h_console, initial_console_settings.wAttributes);
		std::memset(&initial_console_settings, 0, sizeof(initial_console_settings));
		::SetConsoleMode(h_console, old_console_mode);
	}
	allow_styles = false;
	h_console = nullptr;
	stdout_redirected = false;
	raw_input_mode = false;
	is_setup = false;
}

bool key_pressed() {
	if (!raw_input_mode) {
		return false;
	}
	return (::_kbhit() != 0);
}

key get_key(bool block_until_pressed) {
	key result;
	if (!raw_input_mode) {
		return {};
	}
	if ((!block_until_pressed) && (!::_kbhit())) {
		return {};
	}
	std::cout.flush();
	const std::wint_t ch = ::_getwch();
	if ((ch == 0) || (ch == 1)) { // happens with the F1-F12 keys or with CTRL
		result.special = key_enum::unknown;
		return result;
	}
	if (ch != 224) // no special character
	{
		result.special = key_enum::regular;
		auto wc = static_cast<wchar_t>(ch);
		if (wc == L'\r') {
			wc = L'\n'; // because ENTER is translated into LF on posix rather than CR on Windows...
		}
		const auto num = ::WideCharToMultiByte(CP_UTF8, 0, &wc, 1, result.regular.bytes, 4u, nullptr, nullptr);
		assert((num > 0) && (num <= 4));
		result.regular.bytes[num] = '\0';
	}
	else  {
		const std::wint_t control_char = ::_getwch();
		switch(control_char) {
			case 71: result.special = key_enum::home; break;
			case 72: result.special = key_enum::up; break;
			case 73: result.special = key_enum::page_up; break;
			case 75: result.special = key_enum::left; break;
			case 77: result.special = key_enum::right; break;
			case 79: result.special = key_enum::end; break;
			case 80: result.special = key_enum::down; break;
			case 81: result.special = key_enum::page_down; break;
			case 82: result.special = key_enum::insert; break;
			case 83: result.special = key_enum::del; break;
			default: result.special = key_enum::unknown; break;
		}
	}
	return result;
}

terminal_size estimate_terminal_size(const terminal_size& default_if_not_gettable) {
	terminal_size result = default_if_not_gettable;
	if (h_console != nullptr) {
		CONSOLE_SCREEN_BUFFER_INFO info;
		::GetConsoleScreenBufferInfo(h_console, &info);
		result.columns = static_cast<int>(info.srWindow.Right - info.srWindow.Left + 1);
		result.rows = static_cast<int>(info.srWindow.Bottom - info.srWindow.Top + 1);
	}
	return result;
}

bool add_style(const std::string& tag_name, const std::string& escape_sequence) {
	for (const auto c: tag_name) {
		if (!(((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_'))) {
			return false;
		}
	}
	std::unique_lock<std::mutex> lock{style_mutex};
	if (styles.find(tag_name) != styles.end()) { // already exists
		return false;
	}
	styles[tag_name] = escape_sequence;
	return true;
}

bool remove_style(const std::string& tag_name) {
	std::unique_lock<std::mutex> lock{style_mutex};
	const auto style = styles.find(tag_name);
	if (style != styles.end()) {
		styles.erase(style);
		return true;
	}
	return false;
}
std::string get_style(const std::string& tag_name) {
	std::unique_lock<std::mutex> lock{style_mutex};
	std::string result;
	const auto style = styles.find(tag_name);
	if (style != styles.end()) {
		result = style->second;
	}
	return result;
}

std::vector<std::string> get_current_style_stack() {
	std::unique_lock<std::mutex> lock{style_mutex};
	std::vector<std::string> result = style_stack;
	return result;
}

}

#endif
