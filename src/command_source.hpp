/* command_source.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef COMMAND_SOURCE_HPP_INCLUDED
#define COMMAND_SOURCE_HPP_INCLUDED

#include "fwd/command_source.hpp"

#include "fwd/character.hpp"
#include "fwd/world.hpp"

#include <string>

class Command_Source
{
	public:
		virtual AdminLevel SourceAccess() const = 0;
		virtual AdminLevel SourceDutyAccess() const = 0;
		virtual std::string SourceName() const = 0;
		virtual Character* SourceCharacter() = 0;
		virtual World* SourceWorld() = 0;

		virtual void ServerMsg(std::string) = 0;
		virtual void StatusMsg(std::string) = 0;

		virtual ~Command_Source() { }
};

class System_Command_Source : public Command_Source
{
	private:
		World* world;

	public:
		System_Command_Source(World* world);

		AdminLevel SourceAccess() const;
		AdminLevel SourceDutyAccess() const;
		std::string SourceName() const;
		Character* SourceCharacter();
		World* SourceWorld();

		void ServerMsg(std::string);
		void StatusMsg(std::string);
};

#endif // COMMAND_SOURCE_HPP_INCLUDED
