// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#ifndef colmc_setup_h_INCLUDED
#define colmc_setup_h_INCLUDED

#include <string>
#include <vector>

#include <colmc/push_warnings.h>

namespace colmc {

struct config {
	bool win_utf8       = true;  //!< Convert UTF-8 to UTF-16 under Windows and map cout/cin to wcout/wcin
	bool raw_input_mode = false; //!< If true, see <colmc/raw_input.h> for details on how to use this mode
	bool allow_styles   = false; //!< Allow styles (see below)
};

extern void setup(config cfg = config{});

bool add_style(const std::string& tag_name, const std::string& escape_sequence);
bool remove_style(const std::string& tag_name);
std::string get_style(const std::string& tag_name);
std::vector<std::string> get_current_style_stack();

}

#include <colmc/pop_warnings.h>

#endif
