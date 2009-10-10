
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "hook.hpp"

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
