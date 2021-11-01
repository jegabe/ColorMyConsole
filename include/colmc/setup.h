// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#ifndef colmc_setup_h_INCLUDED
#define colmc_setup_h_INCLUDED

#include <string>

#include <colmc/push_warnings.h>

namespace colmc {

struct config {
	bool win_utf8		= true;  //!< Convert UTF-8 to UTF-16 under Windows and map cout/cin to wcout/wcin
	bool raw_input_mode = false; //!< If true, see <colmc/raw_input.h> for details on how to use this mode
	// bool allow_styles = true; //!< Allow styles (see below)
};

extern void setup(config cfg = config{});

// Not implemented yet. In future: Support HTML-like style tags, for example std::cout << "<info>Informational</info>" << std::endl
// and provide a mapping of tags like <info> to escape sequences
// void add_style(const std::string& tag_name, const std::string& escape_sequence);
// void remove_style(const std::string& tag_name);

}

#include <colmc/pop_warnings.h>

#endif
