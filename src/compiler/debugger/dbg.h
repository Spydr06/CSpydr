#ifndef CSPYDR_DBG_H
#define CSPYDR_DBG_H

void debug_info(const char* fmt, ...);
void debug_error(const char* fmt, ...);
void debug_repl(const char* src, const char* bin);

#endif