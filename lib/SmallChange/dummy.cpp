// This file is only present to trick the Automake setup to link with
// CXXLD instead of CCLD.
//
// Without it, no source code files are in this base directory, which
// makes Automake set us up with C linking of libSmallChange. That has
// been found to at least cause problems with gcc 3.2 on Solaris and mingw.

static void dummy_smallchange_cplusplus_function(void) { }
