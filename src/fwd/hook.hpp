
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_HOOK_HPP_INCLUDED
#define FWD_HOOK_HPP_INCLUDED

class Hook;

class HookManager;

#define HOOK_CALL(hm, s) \
{ \
	bool break_hook = false; \
 \
	UTIL_LIST_FOREACH_ALL(hm->hooks[s], Hook *, hook) \
	{ \
		if (hook->ctx->Prepare(hook->func.c_str()) < 0) \
		{ \
			break; \
		} \

#define HOOK_DEFAULT() \
		bool *ret = hook->ctx->Execute<bool>(); \
 \
		if (ret) \
		{ \
			break_hook = *ret; \
 \
			if (break_hook) \
			{ \
				break; \
			} \
		} \
 \
		hook->ctx->as->Release(); \
	} \
 \
	if (!break_hook) \

#define HOOK_CANCEL() \
	else

#define HOOK_CALL_END() \
}

#endif // FWD_HOOK_HPP_INCLUDED
