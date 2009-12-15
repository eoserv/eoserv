
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef HOOK_HPP_INCLUDED
#define HOOK_HPP_INCLUDED

#include "stdafx.h"

#include "script.hpp"

class Hook;
class Hook_Call;
class HookManager;

class Hook : public Shared
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

class HookManager : public Shared
{
	public:
		std::map<std::string, PtrList<Hook> > hooks;

		ScriptEngine engine;
		ScriptContext *current_context;

		HookManager(std::string scriptpath) : engine(scriptpath), current_context(0) { }

		void Register(std::string trigger, Hook *hook);
		void Unregister(std::string trigger, Hook *hook);

		void Register_(std::string trigger, std::string function);
		void Unregister_(std::string trigger, std::string function);

		void InitCall(std::string filename);
		Hook_Call Call(std::string trigger);

		SCRIPT_REGISTER_REF(HookManager)
			SCRIPT_REGISTER_FUNCTION("void Register(string, string)", Register_);
			SCRIPT_REGISTER_FUNCTION("void Unregister(string, string)", Unregister_);
		SCRIPT_REGISTER_END()
};

class Hook_Call : public Shared
{
	private:
		HookManager &hookmanager;
		std::string trigger;

	public:
		int argc;

		Hook_Call(HookManager &hookmanager_, std::string trigger_) : hookmanager(hookmanager_), trigger(trigger_), argc(0)
		{
			UTIL_PTR_LIST_FOREACH(this->hookmanager.hooks[this->trigger], Hook, hook)
			{
				if (hook->ctx->Prepare(hook->func) < 0)
				{
					return;
				}
			}
		}

		template <typename T> static void SetArg(ScriptContext *ctx, int argc, T arg) { ctx->as->SetArgObject(argc, arg); }

		template <typename T> Hook_Call &operator [](T arg)
		{
			UTIL_PTR_LIST_FOREACH(this->hookmanager.hooks[this->trigger], Hook, hook)
			{
				Hook_Call::SetArg<T>(hook->ctx, argc, arg);
			}

			++argc;

			return *this;
		}

		operator bool();
};

template <> void Hook_Call::SetArg<Shared *>(ScriptContext *ctx, int argc, Shared *arg);
template <> void Hook_Call::SetArg<asBYTE>(ScriptContext *ctx, int argc, asBYTE arg);
	template <> void Hook_Call::SetArg<char>(ScriptContext *ctx, int argc, char arg);
template <> void Hook_Call::SetArg<asWORD>(ScriptContext *ctx, int argc, asWORD arg);
	template <> void Hook_Call::SetArg<short>(ScriptContext *ctx, int argc, short arg);
template <> void Hook_Call::SetArg<asDWORD>(ScriptContext *ctx, int argc, asDWORD arg);
	template <> void Hook_Call::SetArg<int>(ScriptContext *ctx, int argc, int arg);
	template <> void Hook_Call::SetArg<long>(ScriptContext *ctx, int argc, long arg);
	template <> void Hook_Call::SetArg<bool>(ScriptContext *ctx, int argc, bool arg);
template <> void Hook_Call::SetArg<asQWORD>(ScriptContext *ctx, int argc, asQWORD arg);
template <> void Hook_Call::SetArg<float>(ScriptContext *ctx, int argc, float arg);
template <> void Hook_Call::SetArg<double>(ScriptContext *ctx, int argc, double arg);

#endif //HOOK_HPP_INCLUDED
