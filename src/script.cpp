
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include <string>
#include <exception>
#include <cstdio>
#include <cerrno>

#include "script.hpp"

#include "Console.hpp"

bool ScriptContext::Prepare(std::string function)
{
	int mainfunc;

	SCRIPT_ASSERT(mainfunc = this->mod->GetFunctionIdByName(function.c_str()),
		"(%s): function %s() not found.", this->mod->GetName(), function.c_str());

	this->as->Prepare(mainfunc);

	return true;
}

void *ScriptContext::Execute()
{
	try
	{
		int r = this->as->Execute();

		if (r == asEXECUTION_EXCEPTION)
		{
			Console::Err("SCRIPT ERROR (%s row:%i): %s.", this->mod->GetName(), this->as->GetExceptionLineNumber(), this->as->GetExceptionString());
			return 0;
		}

		if (r != asEXECUTION_FINISHED)
		{
			Console::Err("SCRIPT ERROR (%s): Failed to execute script.", this->mod->GetName());
			return 0;
		}
	}
	catch (std::exception &e)
	{
		Console::Err("SCRIPT ERROR (%s row:%i): C++ exception (%s).", this->mod->GetName(), this->as->GetCurrentLineNumber(), e.what());
		return 0;
	}
	catch (...)
	{
		Console::Err("SCRIPT ERROR (%s row:%i): C++ exception.", this->mod->GetName(), this->as->GetCurrentLineNumber());
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

	if (!this->as)
	{
		Console::Err("Failed to initialize script engine.");
		return;
	}

	if (this->as->SetMessageCallback(asMETHOD(ScriptEngine, MessageCallback), this, asCALL_THISCALL) < 0)
	{
		Console::Err("Failed to set message callback, script errors may not be reported.");
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

	Console::Err("SCRIPT %s (%s row:%i col:%i): %s", typestr, msg->section, msg->row, msg->col, msg->message);
}

ScriptContext *ScriptEngine::Build(std::string filename)
{
	asIScriptModule *mod = this->as->GetModule(filename.c_str(), asGM_ALWAYS_CREATE);

	if (!mod)
	{
		Console::Err("SCRIPT ERROR (%s): Could not open create module.", filename.c_str());
		return 0;
	}

	asIScriptContext *ctx = this->as->CreateContext();

	if (!ctx)
	{
		Console::Err("SCRIPT ERROR (%s): Could not open create context.", filename.c_str());
		return 0;
	}

	std::string script;

	FILE *fh = std::fopen((this->scriptpath + filename).c_str(), "rb");

	if (!fh)
	{
		Console::Err("SCRIPT ERROR (%s): Could not open script file. (errno = %i)", filename.c_str(), errno);
		return 0;
	}

	char buf[4096] = {0};

	while (!feof(fh))
	{
		int read = std::fread(buf, sizeof(char), 4096, fh);

		if (read <= 0)
		{
			Console::Err("SCRIPT ERROR (%s): Script loading failed. (errno = %i)", filename.c_str(), errno);
			fclose(fh);
			return 0;
		}

		script.append(buf, read);
	}

	std::fclose(fh);

	SCRIPT_ASSERT(mod->AddScriptSection(filename.c_str(), script.c_str(), script.size()),
		"(%s): Failed to add script section.", filename.c_str());

	SCRIPT_ASSERT(mod->Build(),
		"(%s): Failed to compile script.", filename.c_str());

	return new ScriptContext(this, ctx, mod);
}

ScriptEngine::~ScriptEngine()
{
	this->as->Release();
}
