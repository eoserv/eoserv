#ifndef CONFIG_HPP_INCLUDED
#define CONFIG_HPP_INCLUDED

#include <string>
#include <map>
#include <cstddef>

class Config;

#include "util.hpp"

/**
 * Reads configuration data from a file.
 * Does not support sections in files (they're usually ignored).
 */
class Config : public std::map<std::string, util::variant>
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
		 * Reads all configuration data from the file to memory.
		 * @param filename File to read from.
		 */
		Config(std::string filename);
};

#endif // CONFIG_HPP_INCLUDED
