
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef ITERATOR_HPP_INCLUDED
#define ITERATOR_HPP_INCLUDED

#include <stdexcept>

#include "container.hpp"
#include "script.hpp"

class GenericPtrIterator : public Shared
{
	private:
		operator int();

	public:
		virtual bool InRange() const { throw std::logic_error("GenericIterator::InRange"); }
		virtual bool IsEnd() const { throw std::logic_error("GenericIterator::IsEnd"); }
		virtual void Forward() { throw std::logic_error("GenericIterator::Forward"); }
		virtual void Back() { throw std::logic_error("GenericIterator::Back"); }
		virtual void Seek(int amount) { throw std::logic_error("GenericIterator::Seek"); }
		virtual void Forward(int amount) { throw std::logic_error("GenericIterator::Forward(int)"); }
		virtual void Back(int amount) { throw std::logic_error("GenericIterator::Back(int)"); }
		virtual void Begin() { throw std::logic_error("GenericIterator::Begin"); }
		virtual void End() { throw std::logic_error("GenericIterator::End"); }
		virtual GenericPtrContainer::value_type Dereference() { throw std::logic_error("GenericIterator::Dereference"); }

		GenericPtrContainer::value_type operator *() { return Dereference(); }
		GenericPtrContainer::value_type operator ->() { return Dereference(); }

		bool operator +=(int amount) { Forward(amount); return InRange(); }
		bool operator -=(int amount) { Back(amount); return InRange(); }
		bool operator ++() { Forward(); return InRange(); }
		bool operator --() { Back(); return InRange(); }

		operator bool() { return InRange(); }

		virtual ~GenericPtrIterator() { }

	SCRIPT_REGISTER_REF_DF(GenericPtrIterator)
		SCRIPT_REGISTER_FUNCTION("void opAddAssign(int)", operator +=);
		SCRIPT_REGISTER_FUNCTION("void opSubAssign(int)", operator -=);
	SCRIPT_REGISTER_END()
};

class GenericPtrConstIterator : public Shared
{
	private:
		operator int();

	public:
		virtual bool InRange() const { throw std::logic_error("GenericIterator::InRange"); }
		virtual bool IsEnd() const { throw std::logic_error("GenericIterator::IsEnd"); }
		virtual void Forward() { throw std::logic_error("GenericIterator::Forward"); }
		virtual void Back() { throw std::logic_error("GenericIterator::Back"); }
		virtual void Seek(int amount) { throw std::logic_error("GenericIterator::Seek"); }
		virtual void Forward(int amount) { throw std::logic_error("GenericIterator::Forward(int)"); }
		virtual void Back(int amount) { throw std::logic_error("GenericIterator::Back(int)"); }
		virtual void Begin() { throw std::logic_error("GenericIterator::Begin"); }
		virtual void End() { throw std::logic_error("GenericIterator::End"); }
		virtual const GenericPtrContainer::value_type Dereference() { throw std::logic_error("GenericIterator::Dereference"); }

		const GenericPtrContainer::value_type operator *() { return Dereference(); }
		const GenericPtrContainer::value_type operator ->() { return Dereference(); }

		bool operator +=(int amount) { Forward(amount); return InRange(); }
		bool operator -=(int amount) { Back(amount); return InRange(); }
		bool operator ++() { Forward(); return InRange(); }
		bool operator --() { Back(); return InRange(); }

		operator bool() { return InRange(); }

		virtual ~GenericPtrConstIterator() { }

	SCRIPT_REGISTER_REF_DF(GenericPtrIterator)
		SCRIPT_REGISTER_FUNCTION("void opAddAssign(int)", operator +=);
		SCRIPT_REGISTER_FUNCTION("void opSubAssign(int)", operator -=);
	SCRIPT_REGISTER_END()
};

#endif // ITERATOR_HPP_INCLUDED
