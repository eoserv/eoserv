
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include <string>
#include <cstdio>

#include "script.hpp"

#include "Console.hpp"

bool ScriptContext::Prepare(std::string function)
{
	int mainfunc = this->mod->GetFunctionIdByName(function.c_str());

	if (mainfunc < 0)
	{
		Console::Err("SCRIPT ERROR (%s): function %s() not found, aborting.\n", this->mod->GetName(), function.c_str());
		return false;
	}

	this->as->Prepare(mainfunc);

	return true;
}

void *ScriptContext::Execute()
{
	if (this->as->Execute() == asEXECUTION_EXCEPTION)
	{
		Console::Err("SCRIPT ERROR (%s row:%i): %s.\n", this->mod->GetName(), this->as->GetExceptionLineNumber(), this->as->GetExceptionString());
		return 0;
	}

	return this->as->GetAddressOfReturnValue();
}

ScriptContext::~ScriptContext()
{
	this->as->Release();
}

ScriptEngine::ScriptEngine(std::string scriptpath)
{
	this->scriptpath = scriptpath;
	this->max_exec_time = 2;
	this->error_reporting = asMSGTYPE_WARNING;

	this->as = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	if (this->as->SetMessageCallback(asMETHOD(ScriptEngine, MessageCallback), this, asCALL_THISCALL) < 0)
	{
		Console::Wrn("Failed to set script message callback, script errors may not be reported.");
	}

	RegisterScriptMath(this->as);
	RegisterStdString(this->as);
	RegisterScriptDictionary(this->as);
}


void ScriptEngine::MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *typestr = "UNKNOWN";

	if (msg->type > this->error_reporting)
	{
		return;
	}

	switch (msg->type)
	{
		case asMSGTYPE_ERROR: typestr = "ERROR"; break;
		case asMSGTYPE_WARNING: typestr = "WARNING"; break;
		case asMSGTYPE_INFORMATION: typestr = "INFO"; break;
	}

	Console::Err("SCRIPT %s (%s row:%i col:%i): %s\n", typestr, msg->section, msg->row, msg->col, msg->message);
}

ScriptContext *ScriptEngine::Build(std::string filename)
{
	CScriptBuilder builder;

	filename = this->scriptpath + filename;

	if (builder.BuildScriptFromFile(this->as, filename.c_str(), filename.c_str()) < 0)
	{
		Console::Err("SCRIPT ERROR (%s): Compilation failed.\n", filename.c_str());
		return 0;
	}

	asIScriptModule *mod = this->as->GetModule(filename.c_str());
	asIScriptContext *ctx = this->as->CreateContext();

	if (mod && ctx)
	{
		return new ScriptContext(this, ctx, mod);
	}
	else
	{
		Console::Err("SCRIPT ERROR (%s): Context creation failed.\n", filename.c_str());
		return 0;
	}
}

ScriptEngine::~ScriptEngine()
{
	this->as->Release();
}
