
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SCRIPT_HPP_INCLUDED
#define SCRIPT_HPP_INCLUDED

#include <cassert>

#include <angelscript.h>

#include "shared.hpp"

class ScriptEngine;
class ScriptContext;

template <class T> class ScriptObject;
template <class T> class ScriptValueObject;
template <class T> class ScriptRefObject;

#define instance_offsetof(st, m) static_cast<std::size_t>(reinterpret_cast<const char *>(&(st->m)) - reinterpret_cast<const char *>(st))
#define instance_offsetof_v(st, m) static_cast<std::size_t>(reinterpret_cast<volatile const char *>(&(st->m)) - reinterpret_cast<volatile const char *>(st))

#define SCRIPT_REGISTER_TYPESET(type, typer) \
	public: \
		static const char *ScriptTypeName() { return type; } \
		static const char *ScriptTypeNameT() { return type"<T>"; } \
		static const char *ScriptTypeNameR() { return typer(); }

#define SCRIPT_REGISTER_AFTER_TYPE() \
 \
		static void ScriptRegisterType(ScriptEngine &engine) \
		{ \
			static bool done = false; \
			if (done) return; \
			done = true; \
			ScriptType::RegisterType(engine); \
			ScriptType::RegisterDefaultBehaviour(engine); \
		} \
 \
		static bool ScriptRegister(ScriptEngine &engine) \
		{ \
			static bool done = false; \
			if (done) return true; \
			done = true; \
			static unsigned char _script_inst_buf[sizeof(_script_thistype)]; \
			static _script_thistype *_script_inst = reinterpret_cast<_script_thistype *>(_script_inst_buf); \
			(void)_script_inst;

#define SCRIPT_REGISTER(type) typedef type _script_thistype; typedef ScriptValueObject<_script_thistype> ScriptType; SCRIPT_REGISTER_TYPESET(#type, ScriptTypeName) SCRIPT_REGISTER_AFTER_TYPE()
#define SCRIPT_REGISTER_REF(type) typedef type _script_thistype; typedef ScriptRefObject<_script_thistype> ScriptType; SCRIPT_REGISTER_TYPESET(#type, ScriptTypeName) SCRIPT_REGISTER_AFTER_TYPE()
#define SCRIPT_REGISTER_REF_DF(type) SCRIPT_DEFAULT_FACTORY(type) typedef type _script_thistype; typedef ScriptRefObject<_script_thistype> ScriptType; SCRIPT_REGISTER_TYPESET(#type, ScriptTypeName) SCRIPT_REGISTER_AFTER_TYPE() SCRIPT_REGISTER_DEFAULT_FACTORY();
#define SCRIPT_REGISTER_REF_TPL(type) typedef type _script_thistype; typedef ScriptRefTplObject<_script_thistype> ScriptType; SCRIPT_REGISTER_TYPESET(#type, ScriptTypeNameT) SCRIPT_REGISTER_AFTER_TYPE()
#define SCRIPT_REGISTER_NAMED(type, name) typedef type _script_thistype; typedef ScriptValueObject<_script_thistype> ScriptType; SCRIPT_REGISTER_TYPESET(name, ScriptTypeName) SCRIPT_REGISTER_AFTER_TYPE()
#define SCRIPT_REGISTER_REF_NAMED(type, name) typedef type _script_thistype; typedef ScriptRefObject<_script_thistype> ScriptType; SCRIPT_REGISTER_TYPESET(name, ScriptTypeName) SCRIPT_REGISTER_AFTER_TYPE()
#define SCRIPT_REGISTER_REF_DF_NAMED(type, name) SCRIPT_DEFAULT_FACTORY(type) typedef type _script_thistype; typedef ScriptRefObject<_script_thistype> ScriptType; SCRIPT_REGISTER_TYPESET(name, ScriptTypeName) SCRIPT_REGISTER_AFTER_TYPE() SCRIPT_REGISTER_DEFAULT_FACTORY();
#define SCRIPT_REGISTER_REF_TPL_NAMED(type, name) typedef type _script_thistype; typedef ScriptRefTplObject<_script_thistype> ScriptType; SCRIPT_REGISTER_TYPESET(name, ScriptTypeNameT) SCRIPT_REGISTER_AFTER_TYPE()

#define SCRIPT_REGISTER_END() \
			return true; \
		}

#define SCRIPT_DEFAULT_CONSTRUCTOR(type) \
		static void ScriptConstructor(type *memory) \
		{ \
			new (memory) type; \
		}

#define SCRIPT_DEFAULT_FACTORY(type) \
		static type *ScriptFactory() \
		{ \
			return new type; \
		}

#define SCRIPT_REGISTER_FUNCTION(declaration, function) ScriptType::RegisterFunction(engine, declaration, asMETHOD(_script_thistype, function))
#define SCRIPT_REGISTER_FUNCTION_PR(declaration, function, p, r) ScriptType::RegisterFunction(engine, declaration, asMETHODPR(_script_thistype, function, p, r))

#define SCRIPT_REGISTER_VARIABLE(type, name, varname) ScriptType::RegisterVariable(engine, type, name, instance_offsetof(_script_inst, varname))

#define SCRIPT_REGISTER_ENUM(name) engine.as->RegisterEnum(name);
#define SCRIPT_REGISTER_ENUM_VALUE(name, val) engine.as->RegisterEnumValue(name, #val, val);

#define SCRIPT_REGISTER_BEHAVIOUR(behaviour, declaration, function) ScriptType::RegisterBehaviour(engine, behaviour, declaration, asMETHOD(_script_thistype, function), asCALL_THISCALL)
#define SCRIPT_REGISTER_BEHAVIOUR_PR(behaviour, declaration, function, p, r) ScriptType::RegisterBehaviour(engine, behaviour, declaration, asMETHODPR(_script_thistype, function, p, r), asCALL_THISCALL)
#define SCRIPT_REGISTER_BEHAVIOUR_STATIC(behaviour, declaration, function) ScriptType::RegisterBehaviour(engine, behaviour, declaration, asFUNCTION(function), asCALL_CDECL)
#define SCRIPT_REGISTER_BEHAVIOUR_STATIC_PR(behaviour, declaration, function, p, r) ScriptType::RegisterBehaviour(engine, behaviour, declaration, asFUNCTIONPR(function, p, r, asCALL_CDECL))
#define SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(behaviour, declaration, function, cc) ScriptType::RegisterBehaviour(engine, behaviour, declaration, asFUNCTION(function), cc)
#define SCRIPT_REGISTER_BEHAVIOUR_STATIC_PR_CC(behaviour, declaration, function, p, r, cc) ScriptType::RegisterBehaviour(engine, behaviour, declaration, asFUNCTIONPR(function, p, r), cc)

#define SCRIPT_REGISTER_DEFAULT_CONSTRUCTOR() SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_CONSTRUCT, "void f()", _script_thistype::ScriptConstructor, asCALL_CDECL_OBJLAST)
#define SCRIPT_REGISTER_DEFAULT_FACTORY() SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_FACTORY, (std::string(_script_thistype::ScriptTypeName()) + " @f()").c_str(), _script_thistype::ScriptFactory, asCALL_CDECL)

#define SCRIPT_REGISTER_FACTORY(declaration, function) SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_FACTORY, declaration, function, asCALL_CDECL)

#ifndef SCRIPT_ASSERT
#define SCRIPT_ASSERT(expr, format, ...) { int r; if ((r = (expr)) < 0){ Console::Err("SCRIPT ERROR (%i) "format, r, __VA_ARGS__); return false; } }
#endif

#include "fwd/console.hpp"

template<class T, class U> U ScriptStaticCast(T *a)
{
	if (a)
	{
		return static_cast<U>(*a);
	}
	else
	{
		return 0;
	}
};

template<class T, class U> U *ScriptRefCast(T *a)
{
	if (!a)
	{
		return 0;
	}

	U *b = dynamic_cast<U *>(a);

	if (b)
	{
		b->AddRef();
	}

	return b;
};

void *ScriptPtrCast(void *p);

class ScriptContext
{
	public:
		ScriptEngine *engine;
		asIScriptContext *as;
		asIScriptModule *mod;

		ScriptContext(ScriptEngine *engine_, asIScriptContext *ctx, asIScriptModule *mod_) : engine(engine_), as(ctx), mod(mod_) { }

		bool Prepare(std::string function, bool declaration = false);

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
				SCRIPT_ASSERT(engine.as->RegisterObjectMethod(T::ScriptTypeNameR(), declaration, ptr, cc),
					"Failed to register %s for %s", declaration, T::ScriptTypeName());
			}
			else
			{
				SCRIPT_ASSERT(engine.as->RegisterGlobalFunction(declaration, ptr, cc),
					"Failed to register %s for %s", declaration, T::ScriptTypeName());
			}

			return true;
		}

		static bool RegisterVariable(ScriptEngine &engine, const char *type, const char *name, size_t offset)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectProperty(T::ScriptTypeNameR(), (std::string(type) + ' ' + name).c_str(), offset),
				"Failed to register %s %s for %s", type, name, T::ScriptTypeName());

			return true;
		}

		static bool RegisterBehaviour(ScriptEngine &engine, asEBehaviours behaviour, const char *declaration, asSFuncPtr ptr, int cc)
		{
			if (cc == -1)
			{
				switch (ptr.flag)
				{
					case 2: cc = asCALL_CDECL; break;
					case 3: cc = asCALL_THISCALL; break;
					default: cc = asCALL_GENERIC; break;
				}
			}

			SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour(T::ScriptTypeNameR(), behaviour, declaration, ptr, static_cast<asECallConvTypes>(cc)),
				"Failed to register behaviour %i for %s", static_cast<int>(behaviour), T::ScriptTypeName());

			return true;
		}
};

template <class T> class ScriptValueObject : public ScriptObject<T>
{
	public:
		static void ScriptDestruct(asIScriptGeneric *gen)
		{
			T *ptr = static_cast<T *>(gen->GetObject());
			ptr->~T();
		}

		static bool RegisterType(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectType(T::ScriptTypeName(), sizeof(T), asOBJ_VALUE | asOBJ_APP_CLASS_CDA),
				"Failed to register %s for %s", "type", T::ScriptTypeName());

			return true;
		}

		static bool RegisterDefaultBehaviour(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour(T::ScriptTypeNameR(), asBEHAVE_DESTRUCT, "void f()", asFUNCTION(T::ScriptDestruct), asCALL_GENERIC),
				"Failed to register behaviour %i for %s", T::ScriptTypeName());

			return true;
		}
};

template <class T> class ScriptRefObject : public ScriptObject<T>
{
	public:
		ScriptRefObject() : ScriptObject<T>() { }

		static const char *ScriptTypeNameR() { return T::ScriptTypeName(); }

		static bool RegisterType(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectType(T::ScriptTypeName(), sizeof(T), asOBJ_REF),
				"Failed to register %s for %s", "type", T::ScriptTypeName());

			return true;
		}

		static bool RegisterDefaultBehaviour(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour(T::ScriptTypeNameR(), asBEHAVE_ADDREF, "void f()", asMETHOD(Shared, AddRef), asCALL_THISCALL),
				"Failed to register %s for %s", "AddRef", T::ScriptTypeName());

			SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour(T::ScriptTypeNameR(), asBEHAVE_RELEASE, "void f()", asMETHOD(Shared, Release), asCALL_THISCALL),
				"Failed to register %s for %s", "Release", T::ScriptTypeName());

			return true;
		}

		virtual ~ScriptRefObject() { }
};

template <class T> class ScriptRefTplObject : public ScriptRefObject<T>
{
	public:
		ScriptRefTplObject() : ScriptRefObject<T>() { }

		static bool RegisterType(ScriptEngine &engine)
		{
			SCRIPT_ASSERT(engine.as->RegisterObjectType((std::string(T::ScriptTypeName()) + " <class T>").c_str(), sizeof(T), asOBJ_REF | asOBJ_TEMPLATE),
				"Failed to register %s for %s", "template type", T::ScriptTypeName());

			return true;
		}

		virtual ~ScriptRefTplObject() { }
};

#endif // SCRIPT_HPP_INCLUDED
