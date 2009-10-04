
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SCRIPT_HPP_INCLUDED
#define SCRIPT_HPP_INCLUDED

#include <string>

#include <angelscript.h>

#include <angelscript/scriptbuilder.h>
#include <angelscript/scriptmath.h>
#include <angelscript/scriptstdstring.h>
#include <angelscript/scriptdictionary.h>
#include <angelscript/scriptfile.h>

class ScriptEngine;
class ScriptContext;

template <class T> class ScriptObject;
template <class T> class ScriptValueObject;
template <class T> class ScriptRefObject;

class ScriptContext
{
	public:
		ScriptEngine *engine;
		asIScriptContext *as;
		asIScriptModule *mod;

		ScriptContext(ScriptEngine *engine_, asIScriptContext *ctx, asIScriptModule *mod_) : engine(engine_), as(ctx), mod(mod_) { }

		bool Prepare(std::string function);

		void *Execute();

		template <typename T> T *Execute()
		{
			return static_cast<T *>(this->Execute());
		}

		~ScriptContext();
};

class ScriptEngine
{
	public:
		asIScriptEngine *as;
		std::string scriptpath;
		int max_exec_time;
		int error_reporting;

		ScriptEngine(std::string scriptpath);

		void MessageCallback(const asSMessageInfo *msg, void *param);

		ScriptContext *Build(std::string filename);

		~ScriptEngine();
};

template <class T> class ScriptObject
{
	public:
		static void RegisterFunction(ScriptEngine &engine, const char *declaration, asSFuncPtr ptr)
		{
			asECallConvTypes cc;

			switch (ptr.flag)
			{
				case 2: cc = asCALL_CDECL; break;
				case 3: cc = asCALL_THISCALL; break;
				default: cc = asCALL_GENERIC; break;
			}

			if (cc == asCALL_THISCALL)
			{
				engine.as->RegisterObjectMethod(T::Type(), declaration, ptr, cc);
			}
			else
			{
				engine.as->RegisterGlobalFunction(declaration, ptr, cc);
			}
		}
};

template <class T> class ScriptValueObject : public ScriptObject<T>
{
	public:
		static void ScriptConstruct(asIScriptGeneric *gen)
		{
			new (gen->GetObject()) T();
		}

		static void ScriptDestruct(asIScriptGeneric *gen)
		{
			T *ptr = static_cast<T *>(gen->GetObject());
			ptr->~T();
		}

		static void RegisterType(ScriptEngine &engine)
		{
			engine.as->RegisterObjectType(T::Type(), sizeof(T), asOBJ_VALUE | asOBJ_APP_CLASS_CDA);
		}

		static void RegisterBehaviour(ScriptEngine &engine)
		{
			engine.as->RegisterObjectBehaviour(T::Type(), asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(T::ScriptConstruct), asCALL_GENERIC);
			engine.as->RegisterObjectBehaviour(T::Type(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(T::ScriptDestruct), asCALL_GENERIC);
		}
};

#include <cstdio>
template <class T> class ScriptRefObject : public ScriptObject<T>
{
	public:
		void ScriptAddRef(asIScriptGeneric *gen) { }

		void ScriptRelease(asIScriptGeneric *gen) { }

		static void RegisterType(ScriptEngine &engine)
		{
			engine.as->RegisterObjectType(T::Type(), sizeof(T), asOBJ_REF);
		}

		static void RegisterBehaviour(ScriptEngine &engine)
		{
			engine.as->RegisterObjectBehaviour(T::Type(), asBEHAVE_ADDREF, "void f()", asMETHOD(T, ScriptAddRef), asCALL_THISCALL);
			engine.as->RegisterObjectBehaviour(T::Type(), asBEHAVE_RELEASE, "void f()", asMETHOD(T, ScriptRelease), asCALL_THISCALL);
		}
};

#endif // SCRIPT_HPP_INCLUDED
