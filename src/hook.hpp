
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef HOOK_HPP_INCLUDED
#define HOOK_HPP_INCLUDED

#include "fwd/hook.hpp"

#include <string>
#include <tr1/unordered_map>

#include "container/ptr_list.hpp"
#include "script.hpp"
#include "shared.hpp"
#include "util.hpp"

#define HOOK_BEGIN(type, hm_, hook_) \
{ \
	type *e = new type; \
	HookManager *hm = hm_; \
	const char *hook = hook_;

#define HOOK_CALL hm->Call(hook)[e]

#define HOOK_END() \
	e->Release(); \
}

class Hook : public Shared
{
	public:
#ifndef NOSCRIPT
		ScriptContext *ctx;
#endif // NOSCRIPT
		std::string func;
		bool autofree;

#ifdef NOSCRIPT
		Hook(void *ctx_, std::string func_, bool autofree_ = true) : func(func_), autofree(autofree_) { }
#else // NOSCRIPT
		Hook(ScriptContext *ctx_, std::string func_, bool autofree_ = true) : ctx(ctx_), func(func_), autofree(autofree_) { }
#endif // NOSCRIPT

#ifndef NOSCRIPT
		~Hook()
		{
			if (autofree)
			{
				delete this->ctx;
			}
		}
#endif // NOSCRIPT
};

class HookManager : public Shared
{
	public:
		STD_TR1::unordered_map<std::string, PtrList<Hook> > hooks;

#ifdef NOSCRIPT
		HookManager(std::string scriptpath) { }

		void Register(std::string trigger, Hook *hook) { (void)trigger; (void)hook; }
		void Unregister(std::string trigger, Hook *hook) { (void)trigger; (void)hook; }

		void Register_(std::string trigger, std::string function) { (void)trigger; (void)function; }
		void Unregister_(std::string trigger, std::string function) { (void)trigger; (void)function; }

		void InitCall(std::string filename) { (void)filename; }
		Hook_Call Call(std::string trigger);
#else // NOSCRIPT
		ScriptEngine engine;
		ScriptContext *current_context;

		HookManager(std::string scriptpath) : engine(scriptpath), current_context(0) { }

		void Register(std::string trigger, Hook *hook);
		void Unregister(std::string trigger, Hook *hook);

		void Register_(std::string trigger, std::string function);
		void Unregister_(std::string trigger, std::string function);

		void InitCall(std::string filename);
		Hook_Call Call(std::string trigger);
#endif // NOSCRIPT

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
#ifndef NOSCRIPT
				if (hook->ctx->Prepare(hook->func) < 0)
				{
					return;
				}
#endif // NOSCRIPT
			}
		}

#ifndef NOSCRIPT
		template <typename T> static void SetArg(ScriptContext *ctx, int argc, T arg)
		{
			ctx->as->SetArgObject(argc, arg);
		}
#endif // NOSCRIPT

		template <typename T> Hook_Call &operator [](T arg)
		{
#ifndef NOSCRIPT
			UTIL_PTR_LIST_FOREACH(this->hookmanager.hooks[this->trigger], Hook, hook)
			{
				Hook_Call::SetArg<T>(hook->ctx, argc, arg);
			}
#endif // NOSCRIPT

			++argc;

			return *this;
		}

		bool operator ()();
};

#ifndef NOSCRIPT
template <> void Hook_Call::SetArg<Shared *>(ScriptContext *ctx, int argc, Shared *arg);
template <> void Hook_Call::SetArg<asBYTE>(ScriptContext *ctx, int argc, asBYTE arg);
	template <> void Hook_Call::SetArg<char>(ScriptContext *ctx, int argc, char arg);
template <> void Hook_Call::SetArg<asWORD>(ScriptContext *ctx, int argc, asWORD arg);
	template <> void Hook_Call::SetArg<short>(ScriptContext *ctx, int argc, short arg);
template <> void Hook_Call::SetArg<asDWORD>(ScriptContext *ctx, int argc, asDWORD arg);
	template <> void Hook_Call::SetArg<int>(ScriptContext *ctx, int argc, int arg);
	template <> void Hook_Call::SetArg<long>(ScriptContext *ctx, int argc, long arg);
	template <> void Hook_Call::SetArg<bool>(ScriptContext *ctx, int argc, bool arg);
//template <> void Hook_Call::SetArg<asQWORD>(ScriptContext *ctx, int argc, asQWORD arg);
template <> void Hook_Call::SetArg<float>(ScriptContext *ctx, int argc, float arg);
template <> void Hook_Call::SetArg<double>(ScriptContext *ctx, int argc, double arg);
#endif // NOSCRIPT

#endif //HOOK_HPP_INCLUDED
