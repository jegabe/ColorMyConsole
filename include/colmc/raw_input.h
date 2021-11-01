// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#ifndef colmc_raw_input_h_INCLUDED
#define colmc_raw_input_h_INCLUDED

#include <string>
#include <cstring>
#include <ostream>

#include <colmc/push_warnings.h>

namespace colmc {

//! \brief Enumeration of special keys not representable
//! by the default ASCII/UTF-8 set.
enum class key_enum {
	no_key_pressed, //!< Returned when no blocking is used a no kex was hit
	regular,        //!< A regular UTF-8 representable key coded inside key::regular::bytes
	unknown,        //!< A special key not recognized by get_key()
	up,             //!< UP arrow key
	down,           //!< DOWN arrow key
	left,           //!< LEFT arrow key
	right,          //!< RIGHT arrow key
	insert,         //!< INSERT key
	del,            //!< DEL key
	home,           //!< HOME key
	end,            //!< END key
	page_up,        //!< PAGE_UP key
	page_down       //!< PAGE_DOWN key
};

//! \brief to convert a key_enum to a string
inline std::string to_string(key_enum e) {
#define COLMC_ENTRY(e) case key_enum::e: return #e
	switch(e) {
		COLMC_ENTRY(no_key_pressed);
		COLMC_ENTRY(regular);
		COLMC_ENTRY(unknown);
		COLMC_ENTRY(up);
		COLMC_ENTRY(down);
		COLMC_ENTRY(left);
		COLMC_ENTRY(right);
		COLMC_ENTRY(insert);
		COLMC_ENTRY(del);
		COLMC_ENTRY(home);
		COLMC_ENTRY(end);
		COLMC_ENTRY(page_up);
		COLMC_ENTRY(page_down);
		default: break;
	}
#undef COLMC_ENTRY
	return {};
}

//! \brief to be able to stream the special keys
inline std::ostream& operator<<(std::ostream& o, key_enum e) {
	o << to_string(e);
	return o;
}

//! \brief Single UTF-8 encoded char with length 1-4 bytes.
//! Null-terminated to it can easily be streamed or converted into std::string
struct utf8_char {
	char bytes[5] = { '\0' }; // 1-4 bytes and null terminator

	operator std::string() const {
		return std::string{bytes};
	}
};

//! \brief for easy comparison of utf8_char against an ASCII constant
inline bool operator==(const utf8_char& u, char c) {
	return ((u.bytes[0] == c) && (u.bytes[1] == '\0'));
}

inline bool operator!=(const utf8_char& u, char c) {
	return !(u == c);
}

inline bool operator==(char c, const utf8_char& u) {
	return (u == c);
}

inline bool operator!=(char c, const utf8_char& u) {
	return !(u == c);
}

//! \brief for easy comparison of utf8_char against an UTF-8 sequence coded inside a C string
inline bool operator==(const utf8_char& u, const char* p) {
	const auto len = std::strlen(p);
	return ((len == std::strlen(u.bytes)) && (std::memcmp(u.bytes, p, len) == 0));
}

inline bool operator!=(const utf8_char& u, const char* p) {
	return !(u == p);
}

inline bool operator==(const char* p, const utf8_char& u) {
	return (u == p);
}

inline bool operator!=(const char* p, const utf8_char& u) {
	return !(u == p);
}

inline std::ostream& operator<<(std::ostream& o, const utf8_char& c) {
	o << c.bytes;
	return o;
}

//! \brief Representation of a hit key on the keyboard
struct key {
	key_enum special = key_enum::no_key_pressed;
	utf8_char regular; //!< This field is used when special is key_enum::regular

	operator std::string() const {
		if (special == key_enum::regular) {
			return regular;
		}
		return to_string(special);
	}
};

//! \brief For easy streaming of key
inline std::ostream& operator<<(std::ostream& o, const key& c) {
	if (c.special == key_enum::regular) {
		o << c.regular;
	}
	else {
		o << c.special;
	}
	return o;
}

//! \brief for easy comparison of a hit key to an ASCII constant
inline bool operator==(const key& u, char c) {
	return ((u.special == key_enum::regular) && (u.regular == c));
}

inline bool operator!=(const key& u, char c) {
	return !(u == c);
}

inline bool operator==(char c, const key& u) {
	return (u == c);
}

inline bool operator!=(char c, const key& u) {
	return !(u == c);
}

//! \brief for easy comparison of a hit key to an UTF-8 sequence
inline bool operator==(const key& u, const char* p) {
	const auto len = std::strlen(p);
	return ((u.special == key_enum::regular) &&
		    (std::strlen(u.regular.bytes) == len) &&
		    (std::memcmp(u.regular.bytes, p, len) == 0));
}

inline bool operator!=(const key& u, const char* p) {
	return !(u == p);
}

inline bool operator==(const char* p, const key& u) {
	return (u == p);
}

inline bool operator!=(const char* p, const key& u) {
	return !(u == p);
}

inline bool operator==(const key& u, key_enum e) {
	return ((u.special == e));
}

inline bool operator!=(const key& u, key_enum e) {
	return !(u == e);
}

inline bool operator==(key_enum e, const key& u) {
	return (e == u);
}

inline bool operator!=(key_enum e, const key& u) {
	return !(e == u);
}

//! \brief returns true when a key was hit
//! Only available when colmc::setup was configured with raw_input_mode = true
extern bool key_pressed();

//! \brief Returns the pressed key.
//! Only available when colmc::setup was configured with raw_input_mode = true
//! \param block_until_pressed When true and no key was hit, the function waits until a key is hit.
//!                            When false and no key was hit, key_enum::no_key_pressed is returned
extern key get_key(bool block_until_pressed = true);

}

#include <colmc/pop_warnings.h>

#endif
