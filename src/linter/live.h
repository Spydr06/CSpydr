#ifndef CSPL_LIVE_H
#define CSPL_LIVE_H

#include <stdbool.h>
#include "context.h"

void live_session(Context_T* context, const char* path, const char* std_path, bool prompt_on_quit);

#endif