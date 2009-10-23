
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_SCRIPT_HPP_INCLUDED
#define FWD_SCRIPT_HPP_INCLUDED

class ScriptEngine;
class ScriptContext;

template <class T> class ScriptObject;
template <class T> class ScriptValueObject;
template <class T> class ScriptRefObject;

#define instance_offsetof(st, m) static_cast<std::size_t>(reinterpret_cast<const char *>(&(st->m)) - reinterpret_cast<const char *>(st))

#define SCRIPT_CLASS_VALUE(type) class type : public ScriptValueObject<type>
#define SCRIPT_STRUCT_VALUE(type) struct type : public ScriptValueObject<type>
#define SCRIPT_CLASS_REF(type) class type : public ScriptRefObject<type>
#define SCRIPT_STRUCT_REF(type) struct type : public ScriptRefObject<type>

#define SCRIPT_REGISTER(type) \
	public: \
		static const char *Type() { return #type; } \
	 \
		typedef type _script_thistype; \
	 \
		static void ScriptRegisterType(ScriptEngine &engine) \
		{ \
			_script_thistype::RegisterType(engine); \
			_script_thistype::RegisterBehaviour(engine); \
		} \
	 \
		static void ScriptRegister(ScriptEngine &engine)

#define SCRIPT_REGISTER_FUNCTION(declaration, function) _script_thistype::RegisterFunction(engine, declaration, asMETHOD(_script_thistype, function));
#define SCRIPT_REGISTER_VARIABLE(type, name, variable) _script_thistype::RegisterVariable(engine, type, name, offsetof(_script_thistype, variable));
#define SCRIPT_REGISTER_VARIABLE_OFFSET(type, name, offset) _script_thistype::RegisterVariable(engine, type, name, offset);

#ifndef SCRIPT_ASSERT
#define SCRIPT_ASSERT(expr, format, ...) { int r; if ((r = (expr)) < 0){ Console::Err("SCRIPT ERROR (%i) "format, r, __VA_ARGS__); return false; } }
#endif

#endif // FWD_SCRIPT_HPP_INCLUDED
