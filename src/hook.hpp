
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef HOOK_HPP_INCLUDED
#define HOOK_HPP_INCLUDED

#include <map>
#include <list>

#include "script.hpp"

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

class HookManager : public ScriptRefObject<HookManager>
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

		static const char *Type() { return "HookManager"; }

		static void ScriptRegister(ScriptEngine &engine)
		{
			HookManager::RegisterType(engine);
			HookManager::RegisterBehaviour(engine);
			HookManager::RegisterFunction(engine, "void Register(string, string)", asMETHOD(HookManager, Register_));
			HookManager::RegisterFunction(engine, "void Unregister(string, string)", asMETHOD(HookManager, Unregister_));
		}
};

#endif //HOOK_HPP_INCLUDED
