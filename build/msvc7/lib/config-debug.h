#ifndef SMALLCHANGE_DEBUG
#error The define SMALLCHANGE_DEBUG needs to be defined to true or false
#endif

#if SMALLCHANGE_DEBUG
#include "config-debug.h"
#else /* !SMALLCHANGE_DEBUG */
#include "config-release.h"
#endif /* !SMALLCHANGE_DEBUG */
