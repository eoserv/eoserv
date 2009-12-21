
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "hook.hpp"

Hook_Call::operator bool()
{
	UTIL_PTR_LIST_FOREACH(this->hookmanager.hooks[this->trigger], Hook, hook)
	{
		this->hookmanager.current_context = hook->ctx;

		bool *ret = hook->ctx->Execute<bool>();

		this->hookmanager.current_context = 0;

		if (ret && *ret)
		{
			return true;
		}
	}

	return false;
}

void HookManager::Register(std::string trigger, Hook *hook)
{
	this->hooks[trigger].push_back(hook);
}

void HookManager::Unregister(std::string trigger, Hook *hook)
{

}

void HookManager::Register_(std::string trigger, std::string function)
{
	Hook *hook = new Hook(this->current_context, function);
	this->Register(trigger, hook);
}

void HookManager::Unregister_(std::string trigger, std::string function)
{

}

void HookManager::InitCall(std::string filename)
{
	ScriptContext *ctx = this->engine.Build(filename);

	if (!ctx)
	{
		return;
	}

	this->current_context = ctx;

	if (ctx->Prepare("init") < 0)
	{
		return;
	}

	ctx->as->SetArgObject(0, this);
	ctx->Execute();

	this->current_context = 0;
}

Hook_Call HookManager::Call(std::string trigger)
{
	return Hook_Call(*this, trigger);
}

template <> void Hook_Call::SetArg<asBYTE>(ScriptContext *ctx, int argc, asBYTE arg)   { ctx->as->SetArgByte(argc, arg);   }
	template <> void Hook_Call::SetArg<char>(ScriptContext *ctx, int argc, char arg)     { return SetArg(ctx, argc, (asBYTE)arg); }
template <> void Hook_Call::SetArg<asWORD>(ScriptContext *ctx, int argc, asWORD arg)   { ctx->as->SetArgWord(argc, arg);   }
	template <> void Hook_Call::SetArg<short>(ScriptContext *ctx, int argc, short arg)   { return SetArg(ctx, argc, (asWORD)arg); }
template <> void Hook_Call::SetArg<asDWORD>(ScriptContext *ctx, int argc, asDWORD arg) { ctx->as->SetArgDWord(argc, arg);  }
	template <> void Hook_Call::SetArg<int>(ScriptContext *ctx, int argc, int arg)       { return SetArg(ctx, argc, (asDWORD)arg); }
	template <> void Hook_Call::SetArg<long>(ScriptContext *ctx, int argc, long arg)     { return SetArg(ctx, argc, (asDWORD)arg); }
	template <> void Hook_Call::SetArg<bool>(ScriptContext *ctx, int argc, bool arg)     { return SetArg(ctx, argc, (asDWORD)arg); }
//template <> void Hook_Call::SetArg<asQWORD>(ScriptContext *ctx, int argc, asQWORD arg) { ctx->as->SetArgQWord(argc, arg);  }
template <> void Hook_Call::SetArg<float>(ScriptContext *ctx, int argc, float arg)     { ctx->as->SetArgFloat(argc, arg);  }
template <> void Hook_Call::SetArg<double>(ScriptContext *ctx, int argc, double arg)   { ctx->as->SetArgDouble(argc, arg); }
