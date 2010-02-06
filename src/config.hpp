
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include "stdafx.h"

/**
 * Reads configuration data from a file.
 * Does not support sections in files (they're usually ignored).
 */
class Config : public std::tr1::unordered_map<std::string, util::variant>
{
	protected:
		/**
		 * Filename of the configuration file.
		 * Stored in case save support is ever added.
		 */
		std::string filename;

	public:
		/**
		 * Maximum length of a line in the configuration file.
		 */
		static const std::size_t MaxLineLength = 4096;

		/**
		 * Construct an empty Config object which should have Read() called on it
		 */
		Config() { }

		/**
		 * Reads all configuration data from the file to memory.
		 * @param filename File to read from.
		 */
		Config(std::string filename);

		/**
		 * Reads all configuration data from the file to memory.
		 * @param filename File to read from.
		 */
		void Read(std::string filename);

	static Config *ScriptFactory1(std::string filename) { return new Config(filename); }

	SCRIPT_REGISTER_REF_DF(Config)
		SCRIPT_REGISTER_FACTORY("Config @f(string filename)", ScriptFactory1);

		SCRIPT_REGISTER_FUNCTION("void Read(string filename)", Read);
	SCRIPT_REGISTER_END()
};

#endif // CONFIG_HPP_INCLUDED
