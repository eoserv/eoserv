
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SCRIPT_HPP_INCLUDED
#define SCRIPT_HPP_INCLUDED

#include "stdafx.h"

#include <angelscript.h>

#include <angelscript/scriptbuilder.h>
#include <angelscript/scriptmath.h>
#include <angelscript/scriptstdstring.h>
#include <angelscript/scriptdictionary.h>
#include <angelscript/scriptfile.h>
#include "./scriptlibc.h"
#include "./scriptstl.h"

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
		static bool RegisterFunction(ScriptEngine &engine, const char *declaration, asSFuncPtr ptr)
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
				SCRIPT_ASSERT(engine.as->RegisterObjectMethod(T::Type(), declaration, ptr, cc),
					"Failed to register method %s for %s", declaration, T::Type());
			}
			else
			{
				SCRIPT_ASSERT(engine.as->RegisterGlobalFunction(declaration, ptr, cc),
					"Failed to register function %s for %s", declaration, T::Type());
			}

			return true;
		}

		static bool RegisterVariable(ScriptEngine &engine, const char *type, const char *name, size_t offset)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectProperty(T::Type(), (std::string(type) + ' ' + name).c_str(), offset),
				"Failed to register property %s %s for %s", type, name, T::Type());

			return true;
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

		static bool RegisterType(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectType(T::Type(), sizeof(T), asOBJ_VALUE | asOBJ_APP_CLASS_CDA),
				"Failed to register type for %s", T::Type());

			SCRIPT_ASSERT(engine.as->RegisterObjectType((std::string(T::Type()) + "_ptr").c_str(), sizeof(T *), asOBJ_VALUE | asOBJ_APP_CLASS_CDA | asOBJ_POD),
				"Failed to register pointer for %s_ptr", T::Type());

			return true;
		}

		static T &ScriptPointerDeref(T *ptr)
		{
			return *ptr;
		}

		static bool RegisterBehaviour(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour(T::Type(), asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(T::ScriptConstruct), asCALL_GENERIC),
				"Failed to register construct behaviour for %s", T::Type());

			SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour(T::Type(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(T::ScriptDestruct), asCALL_GENERIC),
				"Failed to register destruct behaviour for %s", T::Type());

			SCRIPT_ASSERT(engine.as->RegisterObjectMethod((std::string(T::Type()) + "_ptr").c_str(), (std::string(T::Type()) + " &deref()").c_str(), asFUNCTION(T::ScriptPointerDeref), asCALL_CDECL_OBJFIRST),
				"Failed to register deref behaviour for %s_ptr", T::Type());

			return true;
		}
};

template <class T> class ScriptRefObject : public ScriptObject<T>
{
	public:
		void ScriptAddRef(asIScriptGeneric *gen) { }

		void ScriptRelease(asIScriptGeneric *gen) { }

		static bool RegisterType(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectType(T::Type(), sizeof(T), asOBJ_REF),
				"Failed to register type for %s", T::Type());

			return true;
		}

		static bool RegisterBehaviour(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour(T::Type(), asBEHAVE_ADDREF, "void f()", asMETHOD(T, ScriptAddRef), asCALL_THISCALL),
				"Failed to register AddRef function for %s", T::Type());

			SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour(T::Type(), asBEHAVE_RELEASE, "void f()", asMETHOD(T, ScriptRelease), asCALL_THISCALL),
				"Failed to register Release function for %s", T::Type());

			return true;
		}
};

#endif // SCRIPT_HPP_INCLUDED
