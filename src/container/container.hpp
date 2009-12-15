
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef CONTAINER_HPP_INCLUDED
#define CONTAINER_HPP_INCLUDED

#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <typeinfo>

#include "shared.hpp"

class GenericPtrContainer : public Shared
{
	public:
		typedef Shared *value_type;
		typedef value_type *pointer;
		typedef const value_type *const_pointer;
		typedef value_type &reference;
		typedef const value_type &const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

	public:
		virtual size_type size() const { throw std::logic_error("GenericPtrContainer::size"); }
		virtual size_type max_size() const { return std::numeric_limits<size_type>::max() / sizeof(value_type); }
		virtual bool empty() const { return size() == 0; }

		virtual void resize(size_type) { throw std::logic_error("GenericPtrContainer::resize"); }

		virtual void push_front(value_type) { throw std::logic_error("GenericPtrContainer::push_front"); }
		virtual void pop_front() { throw std::logic_error("GenericPtrContainer::pop_front"); }
		virtual void push_back(value_type) { throw std::logic_error("GenericPtrContainer::push_back"); }
		virtual void pop_back() { throw std::logic_error("GenericPtrContainer::pop_back"); }

		virtual size_type capacity() const { throw std::logic_error("GenericPtrContainer::capacity"); }
		virtual void reserve(size_type) { throw std::logic_error("GenericPtrContainer::reserve"); }

		virtual GenericPtrContainer &assign(const GenericPtrContainer &) { throw std::logic_error("GenericPtrContainer::assign"); }

		virtual void clear() { resize(0); }

		virtual ~GenericPtrContainer() { }
};

#endif // CONTAINER_HPP_INCLUDED
