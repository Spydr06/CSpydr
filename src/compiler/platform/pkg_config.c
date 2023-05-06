#include "pkg_config.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pkgconf/libpkgconf/libpkgconf.h>

#include "error/error.h"
#include "hashmap.h"
#include "io/log.h"
#include "globals.h"
#include "lexer/token.h"
#include "list.h"
#include "mem/mem.h"

typedef struct {
    char* libname;
    char* flag;
} LibOverride_T;

const LibOverride_T OVERRIDES[] = {
    {"c", "-lc"},
    {"m", "-lm"},
    {"pthread", "-lpthread"},
    {NULL, NULL}
};

static bool pkg_error_handler(const char* msg, const pkgconf_client_t* client, const void *data) {
    if(!data)
        return false;
    Token_T* token = (Token_T*) data;

    throw_error(ERR_PKG_CONFIG, token, msg);
    return true;
}

static const char* pkg_errf_str(int errf) {
    switch(errf) {
        case PKGCONF_PKG_ERRF_PACKAGE_NOT_FOUND:
            return "package not found";
        case PKGCONF_PKG_ERRF_PACKAGE_VER_MISMATCH:
            return "package versions don't match";
        case PKGCONF_PKG_ERRF_PACKAGE_CONFLICT:
            return "package conflicts";
        case PKGCONF_PKG_ERRF_DEPGRAPH_BREAK:
            return "dependency graph broken";
        default:
            return "";
    }
}

static char* fragment_quote(const pkgconf_fragment_t *frag)
{
	const char *src = frag->data;
	ssize_t outlen = strlen(src) + 10;
	char *out, *dst;

	if (frag->data == NULL)
		return NULL;

	out = dst = calloc(outlen, 1);

	for (; *src; src++)
	{
		if (((*src < ' ') ||
		    (*src >= (' ' + (frag->merged ? 1 : 0)) && *src < '$') ||
		    (*src > '$' && *src < '(') ||
		    (*src > ')' && *src < '+') ||
		    (*src > ':' && *src < '=') ||
		    (*src > '=' && *src < '@') ||
		    (*src > 'Z' && *src < '\\') ||
#ifndef _WIN32
		    (*src == '\\') ||
#endif
		    (*src > '\\' && *src < '^') ||
		    (*src == '`') ||
		    (*src > 'z' && *src < '~') ||
		    (*src > '~')))
			*dst++ = '\\';

		*dst++ = *src;

		if ((ptrdiff_t)(dst - out) + 2 > outlen)
		{
			ptrdiff_t offset = dst - out;
			outlen *= 2;
			out = realloc(out, outlen);
			dst = out + offset;
		}
	}

	*dst = 0;
	return out;
}

void pkg_config(const char* name, Token_T* token)
{
    for(LibOverride_T const* lib = OVERRIDES; lib->libname != NULL; lib++) {
        if(strcmp(lib->libname, name) == 0) {
            list_push(global.linker_flags, lib->flag);
            return;
        }
    }

    if(hashmap_get(global.included_libs, (char*) name) == (void*) 1)
        return;
    hashmap_put(global.included_libs, (char*) name, (void*) 1);

    // resolve package
    pkgconf_client_t* client = pkgconf_client_new(pkg_error_handler, token, pkgconf_cross_personality_default());
    pkgconf_client_dir_list_build(client, pkgconf_cross_personality_default());

    pkgconf_pkg_t* package = pkgconf_pkg_find(client, name);
    if(!package) {
        pkgconf_error(client, "could not find package `%s`", name);
        return;
    }

    pkgconf_list_t libs = PKGCONF_LIST_INITIALIZER;
    int err = pkgconf_pkg_libs(client, package, &libs, -1);
    if(err != PKGCONF_PKG_ERRF_OK) {
        pkgconf_error(client, "error resolving libraries from package `%s`: %s", name, pkg_errf_str(err));
    }

    pkgconf_node_t* node;
    PKGCONF_FOREACH_LIST_ENTRY(libs.head, node)
    {
        const pkgconf_fragment_t* fragment = node->data;
        char* quoted = fragment_quote(fragment);
        size_t len = strlen(quoted) + 3;

        char* buf = calloc(len, sizeof(char));
        mem_add_ptr(buf);

        snprintf(buf, len, "-%c%s", fragment->type, quoted);
        list_push(global.linker_flags, buf);
        free(quoted);
    }

    pkgconf_fragment_free(&libs);
    pkgconf_pkg_free(client, package);
    pkgconf_client_free(client);
}
