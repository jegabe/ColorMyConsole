// (c) 2021 Jens Ganter-Benzing. Licensed under the MIT license.
#ifndef colmc_term_size_h_INCLUDED
#define colmc_term_size_h_INCLUDED

#include <colmc/push_warnings.h>

namespace colmc {

struct terminal_size {
	int columns = -1;
	int rows = -1;
};

//! \brief As the name suggests, requests the OS to return the size of the current terminal,
//!  if possible.
//! If not possible, the parameter default_if_not_gettable is returned instead
terminal_size estimate_terminal_size(const terminal_size& default_if_not_gettable = {});

}

#include <colmc/pop_warnings.h>

#endif
