/* commands/commands.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef COMMANDS_HPP_INCLUDED
#define COMMANDS_HPP_INCLUDED

#include "../character.hpp"
#include "../command_source.hpp"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define COMMAND_HANDLER_PASTE_AUX2(base, id) base##id
#define COMMAND_HANDLER_PASTE_AUX(base, id) COMMAND_HANDLER_PASTE_AUX2(base, id)

#define COMMAND_HANDLER_REGISTER(uid) \
namespace { struct COMMAND_HANDLER_PASTE_AUX(command_handler_register_helper_, uid) : public command_handler_register_helper \
{ \
	COMMAND_HANDLER_PASTE_AUX(command_handler_register_helper_, uid)() \
	{ \
		command_handler_register_init _init; \

#define COMMAND_HANDLER_REGISTER_END(uid) ; \
	} \
} COMMAND_HANDLER_PASTE_AUX(command_handler_register_helper_instance_, uid); }

namespace Commands
{

template <class T> struct empty_vector_init
{
	std::vector<T> v;
	empty_vector_init() { }
	empty_vector_init(std::initializer_list<T> args) : v(args) { }
	empty_vector_init(empty_vector_init&& other) { v = std::move(other.v); }
	empty_vector_init& operator=(empty_vector_init&& other) { v = std::move(other.v); }
	explicit operator std::vector<T>&&() { return std::move(v); }
};

enum command_flags
{
	CMD_FLAG_DUTY_RESTRICT = 0x01
};

struct command_info
{
	std::string name;
	std::vector<std::string> arguments;
	std::vector<std::string> optional_arguments;
	std::size_t partial_min_chars;
	bool require_character;
	bool duty_restrict;

	command_info(std::string name, empty_vector_init<std::string> arguments = empty_vector_init<std::string>(),
	             empty_vector_init<std::string> optional_arguments = empty_vector_init<std::string>(), std::size_t partial_min_chars = 0)
		: name(name)
		, arguments(arguments)
		, optional_arguments(optional_arguments)
		, partial_min_chars(partial_min_chars)
		, require_character(false)
		, duty_restrict(false)
	{ }
};

struct command_handler
{
	command_info info;
	std::function<void(const std::vector<std::string>&, Command_Source*)> f;

	command_handler(command_info info, std::function<void(const std::vector<std::string>&, Command_Source*)> f)
		: info(info)
		, f(f)
	{ }

	command_handler(command_info info, std::function<void(const std::vector<std::string>&, Character*)> f)
		: info(info)
		, f([f](const std::vector<std::string>& arguments, Command_Source* from) { f(arguments, static_cast<Character*>(from)); })
	{
		this->info.require_character = true;
	}

	bool operator <(const command_handler& rhs) const
	{
		// This ordering is very important for making sure the most specific command gets picked
		return (info.partial_min_chars < rhs.info.partial_min_chars) || (info.name.length() < rhs.info.name.length()) || (info.name < rhs.info.name);
	}
};

class command_handler_register
{
	private:
		std::map<std::string, command_handler> handlers;
		std::unordered_map<std::string, std::string> aliases;

	public:
		void Register(command_handler handler, int flags);
		void RegisterAlias(const std::string& alias, const std::string& command);

		bool Handle(std::string command, const std::vector<std::string>& arguments, Command_Source* from, int alias_depth = 0) const;
};

extern command_handler_register *command_handler_register_instance;

class command_handler_register_init
{
	private:
		bool master;

	public:
		command_handler_register_init(bool master = false)
			: master(master)
		{
			static bool initialized;

			if (!initialized)
			{
				initialized = true;
				init();
			}
		}

		void init() const;

		~command_handler_register_init();
};

class command_handler_register_helper
{
	public:
		template <class F> void Register(command_info info, const F& f, int flags = 0)
		{
			command_handler_register_instance->Register(command_handler(info, std::function<void(const std::vector<std::string>&, Command_Source*)>(f)), flags);
		}

		template <class F>  void RegisterCharacter(command_info info, const F& f, int flags = 0)
		{
			command_handler_register_instance->Register(command_handler(info, std::function<void(const std::vector<std::string>&, Character*)>(f)), flags);
		}

		void RegisterAlias(const std::string& alias, const std::string& command)
		{
			command_handler_register_instance->RegisterAlias(alias, command);
		}
};

inline bool Handle(std::string command, const std::vector<std::string>& arguments, Command_Source* from)
{
	return command_handler_register_instance->Handle(command, arguments, from);
}

}

#endif // COMMANDS_HPP_INCLUDED
