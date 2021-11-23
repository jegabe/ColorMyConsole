// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#ifndef colmc_algorithms_h_INCLUDED
#define colmc_algorithms_h_INCLUDED

#include <cstring>
#include <vector>
#include <limits>

namespace colmc {

constexpr std::size_t invalid_end_of_sequence = 0;
constexpr std::size_t max_style_seq_len = 16u;
constexpr std::size_t no_pos = std::numeric_limits<std::size_t>::max();

inline void replace_content(std::vector<char>& buf, std::size_t& num_used_bytes, std::size_t offset, std::size_t len, const char* replacement, std::size_t replacement_len, std::size_t growth_increment) {
	if (replacement_len == len) {
		std::memcpy(buf.data() + offset, replacement, replacement_len);
	}
	else if (replacement_len < len) {
		const auto diff = len - replacement_len;
		std::memcpy(buf.data() + offset, replacement, replacement_len);
		std::memmove(buf.data() + offset + replacement_len, buf.data() + offset + len, num_used_bytes - offset - len);
		num_used_bytes -= diff;
	}
	else { // replacement_len > len
		const auto diff = replacement_len - len;
		const auto buf_cap_rest = (buf.size() - num_used_bytes);
		if (diff > buf_cap_rest) { // we have to realloc because the buffer is too small now
			auto growth = diff;
			if (growth < growth_increment) {
				growth = growth_increment;
			}
			buf.resize(buf.size() + growth);
		}
		num_used_bytes += diff;
		std::memmove(buf.data() + offset + replacement_len, buf.data() + offset + len, num_used_bytes - offset - replacement_len);
		std::memcpy(buf.data() + offset, replacement, replacement_len);
	}
}

inline std::size_t index_of(const void* p, char c, std::size_t n) {
	const char* pos = static_cast<const char*>(std::memchr(p, static_cast<unsigned char>(c), n));
	if (pos != nullptr) {
		return static_cast<std::size_t>(pos - static_cast<const char*>(p));
	}
	return no_pos;
}

inline std::size_t find_end_of_esc_sequence(const char* p, std::size_t n) {
	std::size_t i = 2u; // after esc[
	while((i < n) && (((p[i] >= '0') && (p[i] <= '9')) || (p[i] == ';'))) { // jump over the command parameters
		++i;
	}
	if ((i < n) && (((p[i] >= 'a') && (p[i] <= 'z')) || ((p[i] >= 'A') && (p[i] <= 'Z')))) {
		return i + 1u; // well formed sequence with alpha [A-Za-z] ending
	}
	return invalid_end_of_sequence;
}

inline std::size_t find_end_of_style_sequence(const char* p, std::size_t n) {
	std::size_t i = 1u; // after '<'
	if ((i < n) && (p[i] == '/')) {
		++i;
	}
	while((i < n) && (i < max_style_seq_len) &&
		  (((p[i] >= 'a') && (p[i] <= 'z')) ||
		   ((p[i] >= 'A') && (p[i] <= 'Z')) ||
		   (p[i] == '_')) &&
		  (p[i] != '>')) {
		++i;
	}
	if ((i > 1) && (p[i] == '>')) {
		return i + 1u;
	}
	return invalid_end_of_sequence;
}


}

#endif
