
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef HOOK_HPP_INCLUDED
#define HOOK_HPP_INCLUDED

#include <map>
#include <list>

#include "script.hpp"

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

class Hook
{
	public:
		ScriptContext *ctx;
		std::string func;
		bool autofree;

		Hook(ScriptContext *ctx_, std::string func_, bool autofree_ = true) : ctx(ctx_), func(func_), autofree(autofree_) { }

		~Hook()
		{
			if (autofree)
			{
				delete this->ctx;
			}
		}
};

SCRIPT_CLASS_REF(HookManager)
{
	public:
		std::map<std::string, std::list<Hook *> > hooks;

		ScriptEngine engine;
		ScriptContext *current_context;

		HookManager(std::string scriptpath) : engine(scriptpath), current_context(0) { }

		void Register(std::string trigger, Hook *hook);
		void Unregister(std::string trigger, Hook *hook);

		void Register_(std::string trigger, std::string function);
		void Unregister_(std::string trigger, std::string function);

		void InitCall(std::string filename);

		SCRIPT_REGISTER(HookManager)
		{
			SCRIPT_REGISTER_FUNCTION("void Register(string, string)", Register_);
			SCRIPT_REGISTER_FUNCTION("void Unregister(string, string)", Unregister_);
		}
};

#endif //HOOK_HPP_INCLUDED
