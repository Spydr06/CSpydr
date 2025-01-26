#include "pkg_config.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <libpkgconf/libpkgconf.h>

#include "error/error.h"
#include "hashmap.h"
#include "lexer/token.h"
#include "list.h"
#include "context.h"

// list of libraries known to not work with pkgconf which should be present on all systems
const char* OVERRIDES[] = {
    "c",
    "m",
    "pthread",
    NULL
};

typedef struct PKG_ERROR_HANDLER_DATA_STRUCT {
    Context_T* context;
    Token_T* token;
} PkgErrorHandlerData_T;

static bool pkg_error_handler(const char* msg, const pkgconf_client_t* client, void *raw_data) {
    if(!raw_data)
        return false;
    PkgErrorHandlerData_T* data = (PkgErrorHandlerData_T*) raw_data;

    throw_error(data->context, ERR_PKG_CONFIG, data->token, msg);
    return true;
}

static const char* pkg_errf_str(int errf) {
    switch(errf)
    {
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

static void pkg_try_alternative(Context_T* context, const char* name, Token_T* token)
{
    bool emit_warning = true;
    for(const char** lib = OVERRIDES; *lib != NULL; lib++)
    {
        if(strcmp(*lib, name) == 0)
        {
            emit_warning = false;
            break;
        }
    }

    if(emit_warning)
        throw_error(context, ERR_PKG_CONFIG_WARN, token, "could not find package named `%s`, trying `-l%s`", name, name);
    
    size_t len = strlen(name) + 3;
    char* buf = calloc(len, sizeof(char));
    snprintf(buf, len, "-l%s", name);
    list_push(context->link_mode.libs, buf);
} 

void pkg_config(Context_T* context, const char* name, Token_T* token)
{
    if(hashmap_get(context->included_libs, (char*) name) == (void*) 1)
        return;
    hashmap_put(context->included_libs, (char*) name, (void*) 1);

    pkgconf_cross_personality_t* personality = pkgconf_cross_personality_default(); 
    personality->want_default_static = context->link_mode.mode == LINK_STATIC;

    // resolve package
    pkgconf_client_t* client = pkgconf_client_new(
        pkg_error_handler,
        &(PkgErrorHandlerData_T){.token = token, .context = context},
        personality
    );
    pkgconf_client_dir_list_build(client, pkgconf_cross_personality_default());

    pkgconf_pkg_t* package = pkgconf_pkg_find(client, name);
    if(!package)
    {
        pkg_try_alternative(context, name, token);
        goto free_pkg;
    }

    pkgconf_list_t libs = PKGCONF_LIST_INITIALIZER;
    int err = pkgconf_pkg_libs(client, package, &libs, -1);
    if(err != PKGCONF_PKG_ERRF_OK)
    {
        pkgconf_error(client, "error resolving libraries from package `%s`: %s", name, pkg_errf_str(err));
        goto free_fragment;
    }

    pkgconf_node_t* node;
    PKGCONF_FOREACH_LIST_ENTRY(libs.head, node)
    {
        const pkgconf_fragment_t* fragment = node->data;
        char* quoted = fragment_quote(fragment);
        size_t len = strlen(quoted) + 3;

        char* buf = calloc(len, sizeof(char));
        CONTEXT_ALLOC_REGISTER(context, (void*) buf);

        snprintf(buf, len, "-%c%s", fragment->type, quoted);
        list_push(context->link_mode.libs, buf);
        free(quoted);
    }

free_fragment:
    pkgconf_fragment_free(&libs);
free_pkg:
    pkgconf_pkg_free(client, package);
    pkgconf_client_free(client);
}
