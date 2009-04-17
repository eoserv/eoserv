#ifndef UTIL_HPP_INCLUDED
#define UTIL_HPP_INCLUDED

#include <string>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <vector>
#include <cstring>
#include <cstddef>
#include <cstdarg>
#include <stdexcept>
#include <algorithm>
#include <iterator>

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

#define UTIL_IFOREACH_GENERIC(iterator, start, end, type, as) \
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

#define UTIL_IFOREACH_GENERIC2(iterator, iterator2, start, end, type, as) \
	for (int util_i = 0; util_i < 1; ++util_i) \
		for (iterator,iterator2 util_start = container.begin(); util_i < 1; ++util_i) \
			for (iterator,iterator2 util_end = contianer.end(); util_i < 1; ++util_i) \
				for (iterator,iterator2 as = util_start; as != util_end; ++as)

#define UTIL_ARRAY_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(util::array<type, type2 >::iterator, start, end, type, as)
#define UTIL_DEQUE_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::deque<type >::iterator, start, end, type, as)
#define UTIL_LIST_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::list<type >::iterator, start, end, type, as)
#define UTIL_MAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(std::map<type >::iterator, start, end, type, as)
#define UTIL_MULTIMAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(std::multimap<type >::iterator, start, end, type, as)
#define UTIL_QUEUE_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::queue<type >::iterator, start, end, type, as)
#define UTIL_PRIORITY_QUEUE_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::priority_queue<type >::iterator, start, end, type, as)
#define UTIL_SET_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::set<type >::iterator, start, end, type, as)
#define UTIL_MULTISET_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::multiset<type >::iterator, start, end, type, as)
#define UTIL_STACK_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::stack<type >::iterator, start, end, type, as)
#define UTIL_VECTOR_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(std::vector<type >::iterator, start, end, type, as)

#define UTIL_ARRAY_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(util::array<type, type2 >::iterator, start, end, type, as)
#define UTIL_DEQUE_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::deque<type >::iterator, start, end, type, as)
#define UTIL_LIST_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::list<type >::iterator, start, end, type, as)
#define UTIL_MAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(std::map<type, type2 >::iterator, start, end, type, as)
#define UTIL_MULTIMAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(std::multimap<type, type2 >::iterator, start, end, type, as)
#define UTIL_QUEUE_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::queue<type >::iterator, start, end, type, as)
#define UTIL_PRIORITY_QUEUE_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::priority_queue<type >::iterator, start, end, type, as)
#define UTIL_SET_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::set<type >::iterator, start, end, type, as)
#define UTIL_MULTISET_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::multiset<type >::iterator, start, end, type, as)
#define UTIL_STACK_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::stack<type >::iterator, start, end, type, as)
#define UTIL_VECTOR_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(std::vector<type >::iterator, start, end, type, as)

#define UTIL_ARRAY_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_ARRAY_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_DEQUE_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_DEQUE_FOREACH(container.begin(), container.end() type, as)
#define UTIL_LIST_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_LIST_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_MAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_MAP_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_MULTIMAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_MULTIMAP_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_QUEUE_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_QUEUE_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_PRIORITY_QUEUE_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_PRIORITY_QUEUE_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_SET_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_SET_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_MULTISET_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_MULTISET_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_STACK_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_STACK_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_VECTOR_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_VECTOR_FOREACH(container.begin(), container.end(), type, as)

#define UTIL_ARRAY_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_ARRAY_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_DEQUE_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_DEQUE_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_LIST_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_LIST_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_MAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_MAP_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_MULTIMAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_MULTIMAP_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_QUEUE_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_QUEUE_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_PRIORITY_QUEUE_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_PRIORITY_QUEUE_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_SET_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_SET_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_MULTISET_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_MULTISET_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_STACK_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_STACK_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_VECTOR_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_VECTOR_IFOREACH(container.begin(), container.end(), type, as)

#define UTIL_TPL_ARRAY_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(class util::array<type, type2 >::iterator, start, end, type, as)
#define UTIL_TPL_DEQUE_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::deque<type >::iterator, start, end, type, as)
#define UTIL_TPL_LIST_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::list<type >::iterator, start, end, type, as)
#define UTIL_TPL_MAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(class std::map<type >::iterator, start, end, type, as)
#define UTIL_TPL_MULTIMAP_FOREACH(start, end, type, type2, as) UTIL_FOREACH_GENERIC2(class std::multimap<type >::iterator, start, end, type, as)
#define UTIL_TPL_QUEUE_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::queue<type >::iterator, start, end, type, as)
#define UTIL_TPL_PRIORITY_QUEUE_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::priority_queue<type >::iterator, start, end, type, as)
#define UTIL_TPL_SET_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::set<type >::iterator, start, end, type, as)
#define UTIL_TPL_MULTISET_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::multiset<type >::iterator, start, end, type, as)
#define UTIL_TPL_STACK_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::stack<type >::iterator, start, end, type, as)
#define UTIL_TPL_VECTOR_FOREACH(start, end, type, as) UTIL_FOREACH_GENERIC(class std::vector<type >::iterator, start, end, type, as)

#define UTIL_TPL_ARRAY_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(class util::array<type, type2 >::iterator, start, end, type, as)
#define UTIL_TPL_DEQUE_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::deque<type >::iterator, start, end, type, as)
#define UTIL_TPL_LIST_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::list<type >::iterator, start, end, type, as)
#define UTIL_TPL_MAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(class std::map<type, type2 >::iterator, start, end, type, as)
#define UTIL_TPL_MULTIMAP_IFOREACH(start, end, type, type2, as) UTIL_IFOREACH_GENERIC2(class std::multimap<type, type2 >::iterator, start, end, type, as)
#define UTIL_TPL_QUEUE_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::queue<type >::iterator, start, end, type, as)
#define UTIL_TPL_PRIORITY_QUEUE_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::priority_queue<type >::iterator, start, end, type, as)
#define UTIL_TPL_SET_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::set<type >::iterator, start, end, type, as)
#define UTIL_TPL_MULTISET_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::multiset<type >::iterator, start, end, type, as)
#define UTIL_TPL_STACK_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::stack<type >::iterator, start, end, type, as)
#define UTIL_TPL_VECTOR_IFOREACH(start, end, type, as) UTIL_IFOREACH_GENERIC(class std::vector<type >::iterator, start, end, type, as)

#define UTIL_TPL_ARRAY_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_ARRAY_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_DEQUE_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_DEQUE_FOREACH(container.begin(), container.end() type, as)
#define UTIL_TPL_LIST_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_LIST_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_MAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_MAP_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_MULTIMAP_FOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_MULTIMAP_FOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_QUEUE_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_QUEUE_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_PRIORITY_QUEUE_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_PRIORITY_QUEUE_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_SET_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_SET_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_MULTISET_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_MULTISET_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_STACK_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_STACK_FOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_VECTOR_FOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_VECTOR_FOREACH(container.begin(), container.end(), type, as)

#define UTIL_TPL_ARRAY_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_ARRAY_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_DEQUE_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_DEQUE_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_LIST_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_LIST_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_MAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_MAP_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_MULTIMAP_IFOREACH_ALL(container, type, type2, as) if (!container.empty()) UTIL_TPL_MULTIMAP_IFOREACH(container.begin(), container.end(), type, type2, as)
#define UTIL_TPL_QUEUE_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_QUEUE_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_PRIORITY_QUEUE_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_PRIORITY_QUEUE_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_SET_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_SET_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_MULTISET_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_MULTISET_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_STACK_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_STACK_IFOREACH(container.begin(), container.end(), type, as)
#define UTIL_TPL_VECTOR_IFOREACH_ALL(container, type, as) if (!container.empty()) UTIL_TPL_VECTOR_IFOREACH(container.begin(), container.end(), type, as)

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

// DEPRECATED
// Relies on a GCC (typeof) or c++0x (decltype) only feature, do not use
#ifdef __GNUC__
#define UTIL_FOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (typeof(*(container.begin())) as; util_i < 1; ++util_i) for (typeof(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = *(util_it)):(as = *container.begin())), util_it != container.end(); as = *(util_it++))
#define UTIL_IFOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (typeof(container.begin()) as; util_i < 1; ++util_i) for (typeof(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = util_it):(as = container.begin())), util_it != container.end(); as = util_it++)
#else // __GNUC__
#define UTIL_FOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (decltype(*(container.begin())) as; util_i < 1; ++util_i) for (decltype(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = *(util_it)):(as = *container.begin())), util_it != container.end(); as = *(util_it++))
#define UTIL_IFOREACH(container, as) if (!container.empty()) for (int util_i = 0; util_i < 1; ++util_i) for (decltype(container.begin()) as; util_i < 1; ++util_i) for (decltype(container.begin()) util_it = container.begin(); ((util_it != container.end())?(as = util_it):(as = container.begin())), util_it != container.end(); as = util_it++)
#endif // __GNUC__

template <typename T, std::size_t N> class array;
class variant;

/**
 * Generic and simple array class.
 */
template <typename T, std::size_t N> class array
{
	private:
		T elements[N];

	public:
		typedef T value_type;
		typedef T *iterator;
		typedef const T *const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef T &reference;
		typedef const T &const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		iterator begin()
		{
			return this->elements;
		}

		const_iterator begin() const
		{
			return this->elements;
		}

		iterator end()
		{
			return this->elements + N;
		}

		const_iterator end() const
		{
			return this->elements + N;
		}

		reverse_iterator rbegin()
		{
			return reverse_iterator(this->end());
		}

		const_reverse_iterator rbegin() const
		{
			return const_reverse_iterator(end());
		}

		reverse_iterator rend()
		{
			return reverse_iterator(begin());
		}

		const_reverse_iterator rend() const
		{
			return const_reverse_iterator(begin());
		}


		array() {}

		array(const_reference value)
		{
			this->assign(value);
		}

		array(T init[N])
		{
			std::memcpy(this->elements, init, sizeof(value_type) * N);
		}

		reference at(size_type index)
		{
			if (index >= N)
			{
				throw std::out_of_range("util::array::at");
			}

			return this->elements[index];
		}

		const_reference at(size_type index) const
		{
			if (index >= N)
			{
				throw std::out_of_range("util::array::at");
			}

			return this->elements[index];
		}

		reference operator[](size_type index)
		{
			return this->at(index);
		}

		const_reference operator[](size_type index) const
		{
			return this->at(index);
		}

		reference front()
		{
			return this->begin();
		}

		const_reference front() const
		{
			return this->begin();
		}

		reference back()
		{
			return this->elements[this->end() - 1];
		}

		const_reference back() const
		{
			return this->elements[this->end() - 1];
		}

		size_type size() const
		{
			return N;
		}

		size_type max_size() const
		{
			return N;
		}

		bool empty() const
		{
			return this->size() == 0;
		}

		void assign(const_reference value)
		{
			std::fill_n(this->begin(), this->size(), value);
		}

		void swap(util::array<T, N> &second)
		{
			std::swap_ranges(this->begin(), this->end(), second.begin());
		}

		const T *data() const
		{
			return this->elements;
		}

		T *data()
		{
			return this->data;
		}
};

/**
 * A type that can store any numeric/string value and convert between them.
 * It takes way too much effort to use, so it's only used by the Config class.
 */
class variant
{
	protected:
		/**
		 * Integer and float values are a union to save on a few bytes of memory.
		 */
		union
		{
			/**
			 * Value stored as an integer.
			 */
			int val_int;

			/**
			 * Value stored as a float.
			 */
			double val_float;
		};

		/**
		 * Value stored as a string.
		 */
		std::string val_string;

		enum var_type
		{
			type_int,
			type_float,
			type_string
		};

		/**
		 * Current type the value is stored as.
		 * Accessing as this type will need no conversion.
		 */
		int type;

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
		 * Helper function that returns the string length of a number in decimal format.
		 */
		int int_length(int);

	public:
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
};

/**
 * Commonly used array type in EOSERV.
 */
typedef array<unsigned char, 2> pairchar;

/**
 * Commonly used array type in EOSERV.
 */
typedef array<unsigned char, 4> quadchar;

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
 * Alternate name for variant.
 */
typedef variant var;

/**
 * Parse a string time period to a number
 * @param timestr amount of time in a human readable format (eg. 2h30m)
 * @return number of seconds
 */
double tdparse(std::string timestr);

int to_int(const std::string &);

std::string to_string(int);

void lowercase(std::string &);

void uppercase(std::string &);

}

#endif // UTIL_HPP_INCLUDED
