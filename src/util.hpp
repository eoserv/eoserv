
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <string>
#include <stack>
#include <tr1/array>
#include <tr1/unordered_map>
#include <vector>

#include "script.hpp"
#include "shared.hpp"

/**
 * Utility functions to assist with common tasks
 */
namespace util
{

// A gigantic pile of fail
// Uglified it to make it generate standard C++ code

#define UTIL_FOREACH_GENERIC(iterator, start, end, type, as) \
	for (int util_i = 0; util_i < 1; ++util_i) \
		for (iterator util_start = start; util_i < 1; ++util_i) \
			for (iterator util_end = end; util_i < 1; ++util_i) \
				for (type as = *util_start; util_i < 1; ++util_i) \
					for(iterator util_iter = util_start; util_iter != util_end; as = (++util_iter == util_end)?*util_start:*util_iter)

#define UTIL_IFOREACH_GENERIC(iterator, start, end, as) \
	for (int util_i = 0; util_i < 1; ++util_i) \
		for (iterator util_start = start; util_i < 1; ++util_i) \
			for (iterator util_end = end; util_i < 1; ++util_i) \
				for (iterator as = util_start; as != util_end; ++as)

#define UTIL_FOREACH_GENERIC2(iterator, iterator2, start, end, type, as) \
	for (int util_i = 0; util_i < 1; ++util_i) \
		for (iterator,iterator2 util_start = start; util_i < 1; ++util_i) \
			for (iterator,iterator2 util_end = end; util_i < 1; ++util_i) \
				for (type as = *util_start; util_i < 1; ++util_i) \
					for(iterator,iterator2 util_iter = util_start; util_iter != util_end; as = (++util_iter == util_end)?*util_start:*util_iter)

#define UTIL_FOREACH_GENERIC22(iterator, iterator2, start, end, type, type2, as) \
	for (int util_i = 0; util_i < 1; ++util_i) \
		for (iterator,iterator2 util_start = start; util_i < 1; ++util_i) \
			for (iterator,iterator2 util_end = end; util_i < 1; ++util_i) \
				for (type, type2 as = *util_start; util_i < 1; ++util_i) \
					for(iterator,iterator2 util_iter = util_start; util_iter != util_end; as = (++util_iter == util_end)?*util_start:*util_iter)

#define UTIL_IFOREACH_GENERIC2(iterator, iterator2, start, end, as) \
	for (int util_i = 0; util_i < 1; ++util_i) \
		for (iterator,iterator2 util_start = start; util_i < 1; ++util_i) \
			for (iterator,iterator2 util_end = end; util_i < 1; ++util_i) \
				for (iterator,iterator2 as = util_start; as != util_end; ++as)

#define UTIL_ARRAY_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(STD_TR1::array<type, type2 >::iterator, start, end, type, as)
#define UTIL_DEQUE_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::deque<type >::iterator, start, end, type, as)
#define UTIL_LIST_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::list<type >::iterator, start, end, type, as)
#define UTIL_MAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC22(std::map<type, type2 >::iterator, start, end, std::pair<type, type2 >, as)
#define UTIL_MULTIMAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(std::multimap<type >::iterator, start, end, type, as)
#define UTIL_SET_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::set<type >::iterator, start, end, type, as)
#define UTIL_MULTISET_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::multiset<type >::iterator, start, end, type, as)
#define UTIL_STACK_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::stack<type >::iterator, start, end, type, as)
#define UTIL_VECTOR_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::vector<type >::iterator, start, end, type, as)
#define UTIL_UNORDERED_MAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC22(STD_TR1::unordered_map<type, type2 >::iterator, start, end, std::pair<type, type2 >, as)

#define UTIL_ARRAY_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(STD_TR1::array<type, type2 >::iterator, start, end, as)
#define UTIL_DEQUE_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::deque<type >::iterator, start, end, as)
#define UTIL_LIST_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::list<type >::iterator, start, end, as)
#define UTIL_MAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(std::map<type, type2 >::iterator, start, end, as)
#define UTIL_MULTIMAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(std::multimap<type, type2 >::iterator, start, end, as)
#define UTIL_SET_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::set<type >::iterator, start, end, as)
#define UTIL_MULTISET_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::multiset<type >::iterator, start, end, as)
#define UTIL_STACK_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::stack<type >::iterator, start, end, as)
#define UTIL_VECTOR_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::vector<type >::iterator, start, end, as)
#define UTIL_UNORDERED_MAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(STD_TR1::unordered_map<type, type2 >::iterator, start, end, as)

#define UTIL_ARRAY_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_ARRAY_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_DEQUE_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_DEQUE_FOREACH(container.begin(), container.end() type, as)
#define UTIL_LIST_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_LIST_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_MAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_MAP_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_MULTIMAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_MULTIMAP_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_SET_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_SET_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_MULTISET_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_MULTISET_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_STACK_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_STACK_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_VECTOR_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_VECTOR_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_UNORDERED_MAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_UNORDERED_MAP_FOREACH(container.begin(), container.end(), type, type2, as)

#define UTIL_ARRAY_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_ARRAY_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_DEQUE_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_DEQUE_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_LIST_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_LIST_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_MAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_MAP_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_MULTIMAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_MULTIMAP_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_SET_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_SET_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_MULTISET_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_MULTISET_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_STACK_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_STACK_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_VECTOR_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_VECTOR_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_UNORDERED_MAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_UNORDERED_MAP_IFOREACH(container.begin(), container.end(), type, type2, as)

#define UTIL_TPL_ARRAY_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(class STD_TR1::array<type, type2 >::iterator, start, end, type, as)
#define UTIL_TPL_DEQUE_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::deque<type >::iterator, start, end, type, as)
#define UTIL_TPL_LIST_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::list<type >::iterator, start, end, type, as)
#define UTIL_TPL_MAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC22(class std::map<type >::iterator, start, end, std::pair<type, type2 >, as)
#define UTIL_TPL_MULTIMAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(class std::multimap<type >::iterator, start, end, type, as)
#define UTIL_TPL_SET_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::set<type >::iterator, start, end, type, as)
#define UTIL_TPL_MULTISET_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::multiset<type >::iterator, start, end, type, as)
#define UTIL_TPL_STACK_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::stack<type >::iterator, start, end, type, as)
#define UTIL_TPL_VECTOR_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::vector<type >::iterator, start, end, type, as)
#define UTIL_TPL_UNORDERED_MAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC22(class STD_TR1::unordered_map<type >::iterator, start, end, std::pair<type, type2 >, as)

#define UTIL_TPL_ARRAY_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(class STD_TR1::array<type, type2 >::iterator, start, end, as)
#define UTIL_TPL_DEQUE_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::deque<type >::iterator, start, end, as)
#define UTIL_TPL_LIST_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::list<type >::iterator, start, end, as)
#define UTIL_TPL_MAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(class std::map<type, type2 >::iterator, start, end, as)
#define UTIL_TPL_MULTIMAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(class std::multimap<type, type2 >::iterator, start, end, as)
#define UTIL_TPL_SET_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::set<type >::iterator, start, end, as)
#define UTIL_TPL_MULTISET_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::multiset<type >::iterator, start, end, as)
#define UTIL_TPL_STACK_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::stack<type >::iterator, start, end, as)
#define UTIL_TPL_VECTOR_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::vector<type >::iterator, start, end, as)
#define UTIL_TPL_UNORDERED_MAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(class STD_TR1::unordered_map<type, type2 >::iterator, start, end, as)

#define UTIL_TPL_ARRAY_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_ARRAY_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_DEQUE_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_DEQUE_FOREACH(container.begin(), container.end() type, as)
#define UTIL_TPL_LIST_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_LIST_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_MAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_MAP_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_MULTIMAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_MULTIMAP_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_SET_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_SET_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_MULTISET_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_MULTISET_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_STACK_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_STACK_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_VECTOR_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_VECTOR_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_UNORDERED_MAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_UNORDERED_MAP_FOREACH(container.begin(), container.end(), type, type2, as)

#define UTIL_TPL_ARRAY_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_ARRAY_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_DEQUE_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_DEQUE_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_LIST_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_LIST_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_MAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_MAP_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_MULTIMAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_MULTIMAP_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_SET_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_SET_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_MULTISET_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_MULTISET_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_STACK_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_STACK_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_VECTOR_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_VECTOR_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_UNORDERED_MAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_UNORDERED_MAP_IFOREACH(container.begin(), container.end(), type, type2, as)

#define UTIL_CARRAY_FOREACH(array, start, end, type, as) \
	for (int util_i = 0; util_i < 1; ++util_i) \
		for (type as = array[start]; util_i < 1; ++util_i) \
			for(std::size_t util_iter = start; util_iter <= end; as = array[++util_iter])

#define UTIL_CARRAY_FOREACH_ALL(array, type, as) \
	for (int util_i = 0; util_i < 1; ++util_i) \
		for (type as = array[0]; util_i < 1; ++util_i) \
			for(std::size_t util_iter = 0; util_iter < sizeof(array)/sizeof(type); as = array[++util_iter])

#define UTIL_REPEAT(amt) \
	for (int util_i = 0; util_i < amt; ++util_i)

#define UTIL_PTR_LIST_FOREACH(container, type, as) for (PtrList<type>::Iterator as(container); as; as ? ++as : 0)
#define UTIL_PTR_VECTOR_FOREACH(container, type, as) for (PtrVector<type>::Iterator as(container); as; as ? ++as : 0)

#define UTIL_PTR_LIST_FOREACH_SAFE(container, type, as) for (PtrList<type>::SafeIterator as(container); as; as ? ++as : 0)
#define UTIL_PTR_VECTOR_FOREACH_SAFE(container, type, as) for (PtrVector<type>::SafeIterator as(container); as; as ? ++as : 0)

#define UTIL_TPL_PTR_LIST_FOREACH(container, type, as) for (class PtrList<type>::Iterator as(container); as; as ? ++as : 0)
#define UTIL_TPL_PTR_VECTOR_FOREACH(container, type, as) for (class PtrVector<type>::Iterator as(container); as; as ? ++as : 0)

#define UTIL_TPL_PTR_LIST_FOREACH_SAFE(container, type, as) for (class PtrList<type>::SafeIterator as(container); as; as ? ++as : 0)
#define UTIL_TPL_PTR_VECTOR_FOREACH_SAFE(container, type, as) for (class PtrVector<type>::SafeIterator as(container); as; as ? ++as : 0)

// DEPRECATED
// Relies on a GCC (typeof) or c++0x (decltype) only feature, do not use
#ifdef __GNUC__
#define UTIL_FOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (typeof(*(container.begin())) as; util_i < 1; ++util_i) for (typeof(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = *(util_it)):(as = *container.begin())), util_it != container.end(); as = *(util_it++))
#define UTIL_IFOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (typeof(container.begin()) as; util_i < 1; ++util_i) for (typeof(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = util_it):(as = container.begin())), util_it != container.end(); as = util_it++)
#else // __GNUC__
#define UTIL_FOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (decltype(*(container.begin())) as; util_i < 1; ++util_i) for (decltype(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = *(util_it)):(as = *container.begin())), util_it != container.end(); as = *(util_it++))
#define UTIL_IFOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (decltype(container.begin()) as; util_i < 1; ++util_i) for (decltype(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = util_it):(as = container.begin())), util_it != container.end(); as = util_it++)
#endif // __GNUC__

/**
 * A type that can store any numeric/string value and convert between them.
 * It takes way too much effort to use, so it's only used by the Config class.
 */
class variant : public Shared
{
	protected:
		/**
		 * Value stored as an integer.
		 */
		int val_int;

		/**
		 * Value stored as a float.
		 */
		double val_float;

		/**
		 * Value stored as a string.
		 */
		std::string val_string;

		/**
		 * Value stored as a bool.
		 */
		bool val_bool;

		enum var_type
		{
			type_int,
			type_float,
			type_string,
			type_bool
		};

		bool cache_val[4];

		/**
		 * Current type the value is stored as.
		 * Accessing as this type will need no conversion.
		 */
		var_type type;

		/**
		 * Invalidates the cache values and changes the type.
		 */
		void SetType(var_type);

		/**
		 * Helper function that returns the string length of a number in decimal format.
		 */
		int int_length(int);

	public:
		/**
		 * Return the value as an integer, casting if neccessary.
		 */
		int GetInt();

		/**
		 * Return the value as a float, casting if neccessary.
		 */
		double GetFloat();

		/**
		 * Return the value as a string, casting if neccessary.
		 */
		std::string GetString();

		/**
		 * Return the value as a bool, casting if neccessary.
		 */
		bool GetBool();

		/**
		 * Set the value to an integer.
		 */
		variant &SetInt(int);

		/**
		 * Set the value to a float.
		 */
		variant &SetFloat(double);

		/**
		 * Set the value to a string.
		 */
		variant &SetString(const std::string &);

		/**
		 * Set the value to a bool.
		 */
		variant &SetBool(bool);

		/**
		 * Initialize the variant to an integer with the value 0.
		 */
		variant();

		/**
		 * Initialize the variant to an integer with the specified value.
		 */
		variant(int);

		/**
		 * Initialize the variant to a float with the specified value.
		 */
		variant(double);

		/**
		 * Initialize the variant to a string with the specified value.
		 */
		variant(const std::string &);

		/**
		 * Initialize the variant to a string with the specified value.
		 */
		variant(const char *);

		/**
		 * Initialize the variant to a bool with the specified value.
		 */
		variant(bool);

		/**
		 * Set the value to an integer.
		 */
		variant &operator =(int);

		/**
		 * Set the value to a float.
		 */
		variant &operator =(double);

		/**
		 * Set the value to a string.
		 */
		variant &operator =(const std::string &);

		/**
		 * Set the value to a string.
		 */
		variant &operator =(const char *);

		/**
		 * Set the value to a bool.
		 */
		variant &operator =(bool);

		/**
		 * Return the value as an integer, casting if neccessary.
		 */
		operator int();

		/**
		 * Return the value as a float, casting if neccessary.
		 */
		operator double();

		/**
		 * Return the value as a string, casting if neccessary.
		 */
		operator std::string();

		/**
		 * Return the value as an boolean, casting if neccessary.
		 */
		operator bool();

	static variant *ScriptFactoryInt(int val) { return new variant(val); }
	static variant *ScriptFactoryDouble(double val) { return new variant(val); }
	static variant *ScriptFactoryString(std::string val) { return new variant(val); }
	static variant *ScriptFactoryBool(bool val) { return new variant(val); }

	SCRIPT_REGISTER_REF_DF(variant)
		SCRIPT_REGISTER_FACTORY("variant @f(int)", ScriptFactoryInt);
		SCRIPT_REGISTER_FACTORY("variant @f(double)", ScriptFactoryDouble);
		SCRIPT_REGISTER_FACTORY("variant @f(string)", ScriptFactoryString);
		SCRIPT_REGISTER_FACTORY("variant @f(bool)", ScriptFactoryBool);

		SCRIPT_REGISTER_FUNCTION("int GetInt()", GetInt);
		SCRIPT_REGISTER_FUNCTION("double GetFloat()", GetFloat);
		SCRIPT_REGISTER_FUNCTION("string GetString()", GetString);
		SCRIPT_REGISTER_FUNCTION("bool GetBool()", GetBool);
		SCRIPT_REGISTER_FUNCTION("variant @SetInt(int)", SetInt);
		SCRIPT_REGISTER_FUNCTION("variant @SetFloat(double)", SetFloat);
		SCRIPT_REGISTER_FUNCTION("variant @SetString(string)", SetString);
		SCRIPT_REGISTER_FUNCTION("variant @SetBool(bool)", SetBool);
	SCRIPT_REGISTER_END();
};

/**
 * Trims whitespace from the left of a string.
 * Whitespace is defined as space, tab, CR and LF.
 */
std::string ltrim(const std::string &);

/**
 * Trims whitespace from the right of a string.
 * Whitespace is defined as space, tab, CR and LF.
 */
std::string rtrim(const std::string &);

/**
 * Trims whitespace from both sides of a string.
 * Whitespace is defined as space, tab, CR and LF.
 */
std::string trim(const std::string &);

/**
 * Split a string in to a vector with a specified delimiter
 */
std::vector<std::string> explode(char delimiter, std::string);

/**
 * Split a string in to a vector with a specified delimiter
 */
std::vector<std::string> explode(std::string delimiter, std::string);

/**
 * Alternate name for variant.
 */
typedef variant var;

/**
 * Parse a string time period to a number
 * @param timestr amount of time in a human readable format (eg. 2h30m)
 * @return number of seconds
 */
double tdparse(std::string timestr);

std::stack<util::variant> rpn_parse(std::string expr);
double rpn_eval(std::stack<util::variant>, STD_TR1::unordered_map<std::string, double> vars);

int to_int(const std::string &);
unsigned int to_uint_raw(const std::string &);
double to_float(const std::string &);

std::string to_string(int);
std::string to_string(double);

std::string lowercase(std::string);

std::string uppercase(std::string);

std::string ucfirst(std::string);

int rand(int min, int max);
double rand(double min, double max);

double round(double);

std::string timeago(double time, double current_time);

void sleep(double seconds);

int text_width(std::string string);
int text_max_word_width(std::string string);
std::string text_cap(std::string string, int width, std::string elipses = "[...]");
std::string text_word_wrap(std::string string, int width);

/**
 * Finds the distance IN TILES between a pair of x,y coordinates
 */
inline int path_length(int x1, int y1, int x2, int y2)
{
	int dx = x1 - x2;
	int dy = y1 - y2;

	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;

	return dx - dy;
}

inline void ScriptRegister(ScriptEngine &engine)
{
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string ltrim(string &in)", ltrim);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string rtrim(string &in)", rtrim);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string trim(string &in)", trim);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("double tdparse(string timestr)", tdparse);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("int to_int(string &in)", to_int);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("double to_float(string &in)", to_float);
	SCRIPT_REGISTER_GLOBAL_FUNCTION_PR("string to_string(int)", to_string, (int), std::string);
	SCRIPT_REGISTER_GLOBAL_FUNCTION_PR("string to_string(double)", to_string, (double), std::string);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string lowercase(string)", lowercase);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string uppercase(string)", uppercase);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string ucfirst(string)", ucfirst);
	SCRIPT_REGISTER_GLOBAL_FUNCTION_PR("int rand(int min, int max)", rand, (int, int), int);
	SCRIPT_REGISTER_GLOBAL_FUNCTION_PR("double rand(double min, double max)", rand, (double, double), double);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("double round(double)", round);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("void sleep(double seconds)", sleep);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string timeago(double time, double current_time)", timeago);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("int path_length(int x1, int x2, int y1, int y2)", path_length);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("int text_width(string)", text_width);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("int text_max_word_width(string)", text_max_word_width);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string text_cap(string, int width, string elipses)", text_cap);
	SCRIPT_REGISTER_GLOBAL_FUNCTION("string text_word_wrap(string, int width)", text_word_wrap);
}

}

#endif // UTIL_HPP_INCLUDED
