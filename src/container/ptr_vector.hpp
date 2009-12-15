
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef VECTOR_HPP_INCLUDED
#define VECTOR_HPP_INCLUDED

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <vector>

#include "container.hpp"
#include "iterator.hpp"
#include "script.hpp"

class GenericPtrVector : public GenericPtrContainer
{
	private:
		std::vector<value_type> data;
		asIObjectType *script_ot;

		void ValueAddRef(value_type val)
		{
			if (script_ot)
			{
				// I don't understand exactly why, but it's not needed.
				// script_ot->GetEngine()->AddRefScriptObject(val, script_ot->GetSubTypeId());
			}
			else
			{
				val->AddRef();
			}
		}

		void ValueRelease(value_type val)
		{
			if (script_ot)
			{
				script_ot->GetEngine()->ReleaseScriptObject(val, script_ot->GetSubTypeId());
			}
			else
			{
				val->Release();
			}
		}

	protected:
		void Delete(value_type &val)
		{
			if (val)
			{
				ValueRelease(val);
			}
		}

	public:
		class SafeIterator : public GenericPtrIterator
		{
			private:
				GenericPtrVector *v;
				size_type pos;

			public:
				SafeIterator(GenericPtrVector &v_) : v(&v_), pos(0) { }

				bool IsEnd() const { return pos == v->size(); }
				bool InRange() const { return (pos >= 0 && pos < v->size()); }

				void Forward() { ++pos; }
				void Back() { --pos; }
				void Forward(int amount) { pos += amount; }
				void Back(int amount) { pos -= amount; }
				void Begin() { pos = 0; }
				void End() { pos = v->size(); }

				value_type Dereference();
				void Set(const value_type);

			static GenericPtrVector::SafeIterator *ScriptFactory(asIObjectType *ot, GenericPtrVector *v_)
			{
				GenericPtrVector::SafeIterator *temp = new GenericPtrVector::SafeIterator(*v_);
				temp->AddRef();
				return temp;
			}

			SCRIPT_REGISTER_REF_TPL_NAMED(GenericPtrVector::SafeIterator, "PtrVector_Iterator")
				SCRIPT_REGISTER_FACTORY("PtrVector_Iterator<T> @f(int &in, const GenericPtrVector @)", ScriptFactory);

				// NOTE: This allows invalid conversions
				SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour("GenericPtrIterator", asBEHAVE_REF_CAST, "PtrVector_Iterator<T> @f()", asFUNCTION(ScriptPtrCast), asCALL_CDECL_OBJLAST),
					"Failed to register cast from %s for %s", "GenericPtrIterator", ScriptTypeName());

				SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_IMPLICIT_REF_CAST, "GenericPtrIterator @f()", ScriptPtrCast, asCALL_CDECL_OBJLAST);

				SCRIPT_REGISTER_FUNCTION("void opAddAssign(int)", operator +=);
				SCRIPT_REGISTER_FUNCTION("void opSubAssign(int)", operator -=);

				SCRIPT_REGISTER_FUNCTION("bool InRange() const", InRange);
				SCRIPT_REGISTER_FUNCTION_PR("void Forward()", Forward, (), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Back()", Back, (), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Forward(int)", Forward, (int), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Back(int)", Back, (int), void);
				SCRIPT_REGISTER_FUNCTION("void Begin()", Begin);
				SCRIPT_REGISTER_FUNCTION("void End()", End);
				SCRIPT_REGISTER_FUNCTION("void Set(T @)", Set);
				SCRIPT_REGISTER_FUNCTION("T @Dereference() const", Dereference);
			SCRIPT_REGISTER_END()

			friend class GenericPtrVector;
		};

		class FastIterator : public GenericPtrIterator
		{
			private:
				GenericPtrVector *v;
				std::vector<value_type>::iterator pos;

			public:
				FastIterator(GenericPtrVector &v_) : v(&v_), pos(v->data.begin()) { }

				bool IsEnd() const { return pos == v->data.end(); }
				bool InRange() const { return !IsEnd(); }

				void Forward() { ++pos; }
				void Back() { --pos; }
				void Forward(int amount) { pos += amount; }
				void Back(int amount) { pos -= amount; }
				void Begin() { pos = v->data.begin(); }
				void End() { pos = v->data.end(); }

				value_type Dereference() { return *pos; }
				void Set(const value_type val) { *pos = val; }

			friend class GenericPtrVector;
		};

		class SafeConstIterator : public GenericPtrConstIterator
		{
			private:
				const GenericPtrVector *v;
				size_type pos;

			public:
				SafeConstIterator(const GenericPtrVector &v_) : v(&v_), pos(0) { }

				bool IsEnd() const { return pos == v->size(); }
				bool InRange() const { return (pos >= 0 && pos < v->size()); }

				void Forward() { ++pos; }
				void Back() { --pos; }
				void Forward(int amount) { pos += amount; }
				void Back(int amount) { pos -= amount; }
				void Begin() { pos = 0; }
				void End() { pos = v->size(); }

				const value_type Dereference();

			static GenericPtrVector::SafeConstIterator *ScriptFactory(asIObjectType *ot, GenericPtrVector *v_)
			{
				GenericPtrVector::SafeConstIterator *temp = new GenericPtrVector::SafeConstIterator(*v_);
				temp->AddRef();
				return temp;
			}

			SCRIPT_REGISTER_REF_TPL_NAMED(GenericPtrVector::SafeConstIterator, "PtrVector_ConstIterator")
				SCRIPT_REGISTER_FACTORY("PtrVector_ConstIterator<T> @f(int &in, const GenericPtrVector @)", ScriptFactory);

				// NOTE: This allows invalid conversions
				SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour("GenericPtrIterator", asBEHAVE_REF_CAST, "PtrVector_Iterator<T> @f()", asFUNCTION(ScriptPtrCast), asCALL_CDECL_OBJLAST),
					"Failed to register cast from %s for %s", "GenericPtrIterator", ScriptTypeName());

				SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_IMPLICIT_REF_CAST, "GenericPtrIterator @f()", ScriptPtrCast, asCALL_CDECL_OBJLAST);

				SCRIPT_REGISTER_FUNCTION("void opAddAssign(int)", operator +=);
				SCRIPT_REGISTER_FUNCTION("void opSubAssign(int)", operator -=);

				SCRIPT_REGISTER_FUNCTION("bool InRange() const", InRange);
				SCRIPT_REGISTER_FUNCTION_PR("void Forward()", Forward, (), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Back()", Back, (), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Forward(int)", Forward, (int), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Back(int)", Back, (int), void);
				SCRIPT_REGISTER_FUNCTION("void Begin()", Begin);
				SCRIPT_REGISTER_FUNCTION("void End()", End);
				SCRIPT_REGISTER_FUNCTION("const T @Dereference() const", Dereference);
			SCRIPT_REGISTER_END()

			friend class GenericPtrVector;
		};

		class FastConstIterator : public GenericPtrConstIterator
		{
			private:
				const GenericPtrVector *v;
				std::vector<value_type>::const_iterator pos;

			public:
				FastConstIterator(const GenericPtrVector &v_) : v(&v_), pos(v->data.begin()) { }

				bool IsEnd() const { return pos == v->data.end(); }
				bool InRange() const { return !IsEnd(); }

				void Forward() { ++pos; }
				void Back() { --pos; }
				void Forward(int amount) { pos += amount; }
				void Back(int amount) { pos -= amount; }
				void Begin() { pos = v->data.begin(); }
				void End() { pos = v->data.end(); }

				const value_type Dereference() { return *pos; }

			friend class GenericPtrVector;
		};

		GenericPtrVector() : script_ot(0) { }
		GenericPtrVector(asIObjectType *ot) : script_ot(ot) { if (ot) ot->AddRef(); }
		GenericPtrVector(const GenericPtrVector &other) : script_ot(0) { this->assign(other); }

		size_type size() const { return data.size(); }
		size_type max_size() const { return data.max_size(); }
		bool empty() const { return data.empty(); }

		void resize(size_type);

		reference at(size_type i) { return data.at(i); }
		const_reference at(size_type i) const { return data.at(i); }
		reference front() { return at(0); }
		const_reference front() const { return at(0); }
		reference back() { return at(size() - 1); }
		const_reference back() const { return at(size() - 1); }

		void push_front(value_type val) { ValueAddRef(val); data.insert(data.begin(), val); }
		void pop_front() { Delete(data.front()); data.erase(data.begin()); }
		void push_back(value_type val) { ValueAddRef(val); data.push_back(val); }
		void pop_back() { Delete(data.back()); data.pop_back(); }

		size_type capacity() const { return data.capacity(); }
		void reserve(size_type capacity) { return data.reserve(capacity); }

		GenericPtrVector &assign(const GenericPtrVector &);

		void erase(SafeIterator &);
		void insert(SafeIterator &, value_type);

		void erase(FastIterator &it) { Delete(*it.pos); data.erase(it.pos); }
		void insert(FastIterator &it, value_type val) { ValueAddRef(val); data.insert(it.pos, val); }

		reference operator [](size_type i) { return data[i]; }
		const_reference operator [](size_type i) const { throw const_cast<const value_type>(data[i]); }
		GenericPtrVector &operator =(const GenericPtrVector &other) { return assign(other); }

		virtual ~GenericPtrVector() { clear(); if (script_ot) script_ot->Release(); }

	static GenericPtrVector *ScriptFactory(asIObjectType *ot)
	{
		GenericPtrVector *temp = new GenericPtrVector(ot);
		temp->AddRef();
		return temp;
	}

	SCRIPT_REGISTER_REF(GenericPtrVector)
		SCRIPT_REGISTER_FACTORY("GenericPtrVector @f(int &in)", ScriptFactory);

		SCRIPT_REGISTER_FUNCTION("uint size() const", size);
		SCRIPT_REGISTER_FUNCTION("uint max_size() const", max_size);
		SCRIPT_REGISTER_FUNCTION("bool empty() const", empty);

		SCRIPT_REGISTER_FUNCTION("bool resize(uint)", resize);

		SCRIPT_REGISTER_FUNCTION("void pop_front()", pop_front);
		SCRIPT_REGISTER_FUNCTION("void pop_back()", pop_back);

		SCRIPT_REGISTER_FUNCTION("uint capacity()", capacity);
		SCRIPT_REGISTER_FUNCTION("void reserve(uint)", reserve);

		SCRIPT_REGISTER_FUNCTION("void clear()", clear);
	SCRIPT_REGISTER_END()

	friend class SafeIterator;
	friend class FastIterator;
};

template <class T> class PtrVector : public GenericPtrVector
{
	public:
		typedef T *value_type;
		typedef value_type *pointer;
		typedef const value_type *const_pointer;
		typedef value_type &reference;
		typedef const value_type &const_reference;

	public:
		class SafeIterator : public GenericPtrVector::SafeIterator
		{
			public:
				SafeIterator(GenericPtrVector &v_) : GenericPtrVector::SafeIterator(v_) { }

				value_type Dereference() { return static_cast<value_type>(GenericPtrVector::SafeIterator::Dereference()); }
				void Set(const value_type val) { GenericPtrVector::SafeIterator::Set(val); }

				value_type operator *() { return Dereference(); }
				value_type operator ->() { return Dereference(); }

				SafeIterator &operator =(const value_type val) { Set(val); return *this; }
		};

		class FastIterator : public GenericPtrVector::FastIterator
		{
			public:
				FastIterator(GenericPtrVector &v_) : GenericPtrVector::FastIterator(v_) { }

				value_type Dereference() { return static_cast<value_type>(GenericPtrVector::FastIterator::Dereference()); }
				void Set(const value_type val) { Set(val); }

				value_type operator *() { return Dereference(); }
				value_type operator ->() { return Dereference(); }

				FastIterator &operator =(const value_type val) { Set(val); return *this; }
		};

		class SafeConstIterator : public GenericPtrVector::SafeConstIterator
		{
			public:
				SafeConstIterator(const GenericPtrVector &v_) : GenericPtrVector::SafeConstIterator(v_) { }

				const value_type Dereference() { return static_cast<const value_type>(GenericPtrVector::SafeConstIterator::Dereference()); }

				const value_type operator *() { return Dereference(); }
				const value_type operator ->() { return Dereference(); }
		};

		class FastConstIterator : public GenericPtrVector::FastConstIterator
		{
			public:
				FastConstIterator(const GenericPtrVector &v_) : GenericPtrVector::FastConstIterator(v_) { }

				const value_type Dereference() { return static_cast<const value_type>(GenericPtrVector::FastConstIterator::Dereference()); }

				const value_type operator *() { return Dereference(); }
				const value_type operator ->() { return Dereference(); }
		};

		typedef FastIterator Iterator;
		typedef FastConstIterator ConstIterator;

		PtrVector<T>() : GenericPtrVector() { }
		PtrVector<T>(const PtrVector<T> &other) : GenericPtrVector(other) { }

		reference at(size_type i) { return reinterpret_cast<reference>(GenericPtrVector::at(i)); }
		const_reference at(size_type i) const { return reinterpret_cast<const_reference>(GenericPtrVector::at(i)); }
		reference front() { return reinterpret_cast<reference>(GenericPtrVector::front()); }
		const_reference front() const { return reinterpret_cast<const_reference>(GenericPtrVector::front()); }
		reference back() { return reinterpret_cast<reference>(GenericPtrVector::back()); }
		const_reference back() const { return reinterpret_cast<const_reference>(GenericPtrVector::back()); }

		void push_front(value_type val) { GenericPtrVector::push_front(val); }
		void push_back(value_type val) { GenericPtrVector::push_back(val); }
		reference operator [](size_type i) { return reinterpret_cast<reference>(GenericPtrVector::operator [](i)); }
		const_reference operator [](size_type i) const { return reinterpret_cast<const_reference>(GenericPtrVector::operator [](i)); }
};

class ScriptPtrVector : public GenericPtrVector
{
	public:

	static GenericPtrVector *ScriptFactory(asIObjectType *ot)
	{
		GenericPtrVector *temp = new GenericPtrVector(ot);
		temp->AddRef();
		return temp;
	}

	SCRIPT_REGISTER_REF_TPL_NAMED(ScriptPtrVector, "PtrVector")
		SCRIPT_REGISTER_FACTORY("PtrVector<T> @f(int &in)", ScriptFactory);

		// NOTE: This allows invalid conversions
		SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour("GenericPtrVector", asBEHAVE_REF_CAST, "PtrVector<T> @f()", asFUNCTION(ScriptPtrCast), asCALL_CDECL_OBJLAST),
			"Failed to register cast from %s for %s", "GenericPtrVector", ScriptTypeName());

		SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_IMPLICIT_REF_CAST, "GenericPtrVector @f()", ScriptPtrCast, asCALL_CDECL_OBJLAST);

		SCRIPT_REGISTER_FUNCTION("uint size() const", size);
		SCRIPT_REGISTER_FUNCTION("uint max_size() const", max_size);
		SCRIPT_REGISTER_FUNCTION("bool empty()", empty);

		SCRIPT_REGISTER_FUNCTION("bool resize(uint)", resize);

		SCRIPT_REGISTER_FUNCTION_PR("T @&at(uint)", at, (size_type), reference);
		SCRIPT_REGISTER_FUNCTION_PR("const T @&at(uint) const", at, (size_type) const, const_reference);
		SCRIPT_REGISTER_FUNCTION_PR("T @&front()", front, (), reference);
		SCRIPT_REGISTER_FUNCTION_PR("const T @&front() const", front, () const, const_reference);
		SCRIPT_REGISTER_FUNCTION_PR("T @&back()", back, (), reference);
		SCRIPT_REGISTER_FUNCTION_PR("const T @&back() const", back, () const, const_reference);

		SCRIPT_REGISTER_FUNCTION("void push_front(const T @)", push_front);
		SCRIPT_REGISTER_FUNCTION("void pop_front()", pop_front);
		SCRIPT_REGISTER_FUNCTION("void push_back(const T @)", push_back);
		SCRIPT_REGISTER_FUNCTION("void pop_back()", pop_back);

		SCRIPT_REGISTER_FUNCTION_PR("void insert(GenericPtrIterator @, T @)", insert, (SafeIterator &, value_type), void);
		SCRIPT_REGISTER_FUNCTION_PR("void erase(GenericPtrIterator @)", erase, (SafeIterator &), void);

		SCRIPT_REGISTER_FUNCTION("uint capacity()", capacity);
		SCRIPT_REGISTER_FUNCTION("void reserve()", reserve);

		SCRIPT_REGISTER_FUNCTION("void clear()", clear);
	SCRIPT_REGISTER_END()
};

#endif // VECTOR_HPP_INCLUDED
