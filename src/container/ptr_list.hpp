
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef LIST_HPP_INCLUDED
#define LIST_HPP_INCLUDED

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <list>

#include "container.hpp"
#include "iterator.hpp"

class GenericPtrList : public GenericPtrContainer
{
	private:
		std::list<value_type> data;
		unsigned long rev;
#ifndef NOSCRIPT
		asIObjectType *script_ot;
#endif // NOSCRIPT

		void ValueAddRef(value_type val)
		{
#ifndef NOSCRIPT
			if (script_ot)
			{
				// I don't understand exactly why, but it's not needed.
				// script_ot->GetEngine()->AddRefScriptObject(val, script_ot->GetSubTypeId());
			}
			else
#endif // NOSCRIPT
			{
				val->AddRef();
			}
		}

		void ValueRelease(value_type val)
		{
#ifndef NOSCRIPT
			if (script_ot)
			{
				script_ot->GetEngine()->ReleaseScriptObject(val, script_ot->GetSubTypeId());
			}
			else
#endif // NOSCRIPT
			{
				val->Release();
			}
		}

	public:
		class SafeIterator : public GenericPtrIterator
		{
			private:
				GenericPtrList *list;
				mutable std::list<value_type>::iterator it;
				std::size_t pos;
				mutable unsigned long rev;

				void Reset() const
				{
					if (rev != list->rev)
					{
						it = list->data.begin();

						for (std::size_t i = 0; i < pos; ++i)
						{
							if (it == list->data.end())
							{
								throw std::out_of_range("GenericPtrList::SafeIterator::Reset");
							}

							++it;
						}

						rev = list->rev;
					}
				}

			public:
				SafeIterator(GenericPtrList &list_) : list(&list_), it(list->data.begin()), pos(0), rev(list->rev) { }

				bool IsEnd() const { return it == list->data.end(); }
				bool InRange() const { Reset(); return !IsEnd(); }

				void Forward() { ++it; ++pos; }
				void Back() { --it; --pos; }

				void Forward(int amount)
				{
					for (int i = 0; i < amount; ++i)
					{
						++it;
					}

					pos += amount;
				}

				void Back(int amount)
				{
					for (int i = 0; i < amount; ++i)
					{
						--it;
					}

					pos += amount;
				}

				void Begin() { it = list->data.begin(); pos = 0; }
				void End() { it = list->data.end(); pos = list->size(); }

				value_type Dereference();
				void Set(const value_type);

#ifndef NOSCRIPT
			static GenericPtrList::SafeIterator *ScriptFactory(asIObjectType *ot, GenericPtrList *v_) { return new GenericPtrList::SafeIterator(*v_); }
#endif // NOSCRIPT

			SCRIPT_REGISTER_REF_TPL_NAMED(GenericPtrList::SafeIterator, "PtrList_Iterator")
				SCRIPT_REGISTER_FACTORY("PtrList_Iterator<T> @f(int &in, const GenericPtrList @)", ScriptFactory);

				// NOTE: This allows invalid conversions
				SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour("GenericPtrIterator", asBEHAVE_REF_CAST, "PtrList_Iterator<T> @f()", asFUNCTION(ScriptPtrCast), asCALL_CDECL_OBJLAST),
					"Failed to register cast from %s for %s", "GenericPtrIterator", ScriptTypeName());

				SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_IMPLICIT_REF_CAST, "GenericPtrIterator @f()", ScriptPtrCast, asCALL_CDECL_OBJLAST);

				SCRIPT_REGISTER_FUNCTION("void opAddAssign(int)", operator +=);
				SCRIPT_REGISTER_FUNCTION("void opSubAssign(int)", operator -=);

				SCRIPT_REGISTER_FUNCTION("bool IsEnd() const", IsEnd);
				SCRIPT_REGISTER_FUNCTION("bool InRange() const", InRange);
				SCRIPT_REGISTER_FUNCTION_PR("void Forward()", Forward, (), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Back()", Back, (), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Forward(int)", Forward, (int), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Back(int)", Back, (int), void);
				SCRIPT_REGISTER_FUNCTION("void Begin()", Begin);
				SCRIPT_REGISTER_FUNCTION("void End()", End);
				SCRIPT_REGISTER_FUNCTION("void Set(const T @)", Set);
				SCRIPT_REGISTER_FUNCTION("T @Dereference() const", Dereference);
			SCRIPT_REGISTER_END()

			friend class GenericPtrList;
		};

		class FastIterator : public GenericPtrIterator
		{
			private:
				GenericPtrList *list;
				std::list<value_type>::iterator it;

			public:
				FastIterator(GenericPtrList &list_) : list(&list_), it(list->data.begin()) { }

				bool IsEnd() const { return it == list->data.end(); }
				bool InRange() const { return !IsEnd(); }

				void Forward() { ++it; }
				void Back() { --it; }

				void Forward(int amount)
				{
					for (int i = 0; i < amount; ++i)
					{
						++it;
					}
				}

				void Back(int amount)
				{
					for (int i = 0; i < amount; ++i)
					{
						--it;
					}
				}

				void Begin() { it = list->data.begin(); }
				void End() { it = list->data.end(); }

				value_type Dereference() { return *it; }
				void Set(const value_type val) { *it = val; }

			friend class GenericPtrList;
		};

		class SafeConstIterator : public GenericPtrConstIterator
		{
			private:
				const GenericPtrList *list;
				mutable std::list<value_type>::const_iterator it;
				std::size_t pos;
				mutable unsigned long rev;

				void Reset() const
				{
					if (rev != list->rev && it != list->data.end())
					{
						it = list->data.begin();

						for (std::size_t i = 0; i < pos; ++i)
						{
							if (it == list->data.end())
							{
								throw std::out_of_range("GenericPtrList::SafeIterator::Reset");
							}

							++it;
						}

						rev = list->rev;
					}
				}

			public:
				SafeConstIterator(const GenericPtrList &list_) : list(&list_), it(list->data.begin()), pos(0), rev(list->rev) { }

				bool IsEnd() const { return it == list->data.end(); }
				bool InRange() const { Reset(); return !IsEnd(); }

				void Forward() { ++it; ++pos; }
				void Back() { --it; --pos; }

				void Forward(int amount)
				{
					for (int i = 0; i < amount; ++i)
					{
						++it;
					}

					pos += amount;
				}

				void Back(int amount)
				{
					for (int i = 0; i < amount; ++i)
					{
						--it;
					}

					pos += amount;
				}

				void Begin() { it = list->data.begin(); pos = 0; }
				void End() { it = list->data.end(); pos = list->size(); }

				const value_type Dereference();

#ifndef NOSCRIPT
			static GenericPtrList::SafeIterator *ScriptFactory(asIObjectType *ot, GenericPtrList *v_) { return new GenericPtrList::SafeIterator(*v_); }
#endif // NOSCRIPT

			SCRIPT_REGISTER_REF_TPL_NAMED(GenericPtrList::SafeIterator, "PtrList_Iterator")
				SCRIPT_REGISTER_FACTORY("PtrList_Iterator<T> @f(int &in, const GenericPtrList @)", ScriptFactory);

				// NOTE: This allows invalid conversions
				SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour("GenericPtrIterator", asBEHAVE_REF_CAST, "PtrList_Iterator<T> @f()", asFUNCTION(ScriptPtrCast), asCALL_CDECL_OBJLAST),
					"Failed to register cast from %s for %s", "GenericPtrIterator", ScriptTypeName());

				SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_IMPLICIT_REF_CAST, "GenericPtrIterator @f()", ScriptPtrCast, asCALL_CDECL_OBJLAST);

				SCRIPT_REGISTER_FUNCTION("void opAddAssign(int)", operator +=);
				SCRIPT_REGISTER_FUNCTION("void opSubAssign(int)", operator -=);

				SCRIPT_REGISTER_FUNCTION("bool IsEnd() const", IsEnd);
				SCRIPT_REGISTER_FUNCTION("bool InRange() const", InRange);
				SCRIPT_REGISTER_FUNCTION_PR("void Forward()", Forward, (), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Back()", Back, (), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Forward(int)", Forward, (int), void);
				SCRIPT_REGISTER_FUNCTION_PR("void Back(int)", Back, (int), void);
				SCRIPT_REGISTER_FUNCTION("void Begin()", Begin);
				SCRIPT_REGISTER_FUNCTION("void End()", End);
				SCRIPT_REGISTER_FUNCTION("void Set(const T @)", Set);
				SCRIPT_REGISTER_FUNCTION("T @Dereference() const", Dereference);
			SCRIPT_REGISTER_END()

			friend class GenericPtrList;
		};

		class FastConstIterator : public GenericPtrConstIterator
		{
			private:
				const GenericPtrList *list;
				std::list<value_type>::const_iterator it;

			public:
				FastConstIterator(const GenericPtrList &list_) : list(&list_), it(list->data.begin()) { }

				bool IsEnd() const { return it == list->data.end(); }
				bool InRange() const { return !IsEnd(); }

				void Forward() { ++it; }
				void Back() { --it; }

				void Forward(int amount)
				{
					for (int i = 0; i < amount; ++i)
					{
						++it;
					}
				}

				void Back(int amount)
				{
					for (int i = 0; i < amount; ++i)
					{
						--it;
					}
				}

				void Begin() { it = list->data.begin(); }
				void End() { it = list->data.end(); }

				const value_type Dereference() { return *it; }

			friend class GenericPtrList;
		};

#ifdef NOSCRIPT
		GenericPtrList() : rev(0) { }
		GenericPtrList(const GenericPtrList &other) : rev(0) { this->assign(other); }
#else // NOSCRIPT
		GenericPtrList() : rev(0), script_ot(0) { }
		GenericPtrList(asIObjectType *ot) : rev(0), script_ot(ot) { if (ot) ot->AddRef(); }
		GenericPtrList(const GenericPtrList &other) : rev(0), script_ot(0) { this->assign(other); }
#endif // NOSCRIPT

		reference front() { return data.front(); }
		const_reference front() const { return data.front(); }
		reference back() { return data.back(); }
		const_reference back() const { return data.back(); }

		size_type size() const { return data.size(); }
		size_type max_size() const { return data.max_size(); }
		bool empty() const { return size() == 0; }

		void resize(size_type);

		void push_front(value_type);
		void pop_front();
		void push_back(value_type);
		void pop_back();

		GenericPtrList &assign(const GenericPtrList &);

		void erase(SafeIterator &);
		void insert(SafeIterator &, value_type);

		void erase(FastIterator &);
		void insert(FastIterator &, value_type);

		GenericPtrList &operator =(const GenericPtrList &other) { return assign(other); }

		virtual ~GenericPtrList()
		{
			clear();

#ifndef NOSCRIPT
			if (script_ot)
				script_ot->Release();
#endif // NOSCRIPT
		}

#ifndef NOSCRIPT
	static GenericPtrList *ScriptFactory(asIObjectType *ot) { return new GenericPtrList; }
#endif // NOSCRIPT

	SCRIPT_REGISTER_REF(GenericPtrList)
		SCRIPT_REGISTER_FACTORY("GenericPtrList @f(int &in)", ScriptFactory);

		SCRIPT_REGISTER_FUNCTION("uint size() const", size);
		SCRIPT_REGISTER_FUNCTION("uint max_size() const", max_size);
		SCRIPT_REGISTER_FUNCTION("bool empty() const", empty);

		SCRIPT_REGISTER_FUNCTION("bool resize(uint)", resize);

		SCRIPT_REGISTER_FUNCTION("void pop_back()", pop_back);

		SCRIPT_REGISTER_FUNCTION("void clear()", clear);
	SCRIPT_REGISTER_END()

	friend class SafeIterator;
	friend class FastIterator;
};

template <class T> class PtrList : public GenericPtrList
{
	public:
		typedef T *value_type;
		typedef value_type *pointer;
		typedef const value_type *const_pointer;
		typedef value_type &reference;
		typedef const value_type &const_reference;

	public:
		class SafeIterator : public GenericPtrList::SafeIterator
		{
			public:
				SafeIterator(GenericPtrList &list) : GenericPtrList::SafeIterator(list) { }

				value_type Dereference() { return static_cast<value_type>(GenericPtrList::SafeIterator::Dereference()); }
				void Set(const value_type val) { GenericPtrList::SafeIterator::Set(val); }

				value_type operator *() { return Dereference(); }
				value_type operator ->() { return Dereference(); }

				SafeIterator &operator =(const value_type val) { SetT(val); return *this; }
		};

		class FastIterator : public GenericPtrList::FastIterator
		{
			public:
				FastIterator(GenericPtrList &list) : GenericPtrList::FastIterator(list) { }

				value_type Dereference() { return static_cast<value_type>(GenericPtrList::FastIterator::Dereference()); }
				void Set(const value_type val) { GenericPtrList::FastIterator::Set(val); }

				value_type operator *() { return Dereference(); }
				value_type operator ->() { return Dereference(); }

				FastIterator &operator =(const value_type val) { SetT(val); return *this; }
		};

		class SafeConstIterator : public GenericPtrList::SafeConstIterator
		{
			public:
				SafeConstIterator(const GenericPtrList &list) : GenericPtrList::SafeConstIterator(list) { }

				const value_type Dereference() { return static_cast<const value_type>(GenericPtrList::SafeConstIterator::Dereference()); }

				const value_type operator *() { return Dereference(); }
				const value_type operator ->() { return Dereference(); }

				SafeConstIterator &operator =(const value_type val) { SetT(val); return *this; }
		};

		class FastConstIterator : public GenericPtrList::FastConstIterator
		{
			public:
				FastConstIterator(const GenericPtrList &list) : GenericPtrList::FastConstIterator(list) { }

				const value_type Dereference() { return static_cast<const value_type>(GenericPtrList::FastConstIterator::Dereference()); }

				const value_type operator *() { return Dereference(); }
				const value_type operator ->() { return Dereference(); }

				FastConstIterator &operator =(const value_type val) { SetT(val); return *this; }
		};

		typedef FastIterator Iterator;
		typedef FastConstIterator ConstIterator;

		PtrList<T>() : GenericPtrList() { }
		PtrList<T>(const PtrList<T> &other) : GenericPtrList(other) { }

		reference front() { return reinterpret_cast<reference>(GenericPtrList::front()); }
		const_reference front() const { return reinterpret_cast<const_reference>(GenericPtrList::front()); }
		reference back() { return reinterpret_cast<reference>(GenericPtrList::back()); }
		const_reference back() const { return reinterpret_cast<const_reference>(GenericPtrList::back()); }

		void push_front(value_type val) { GenericPtrList::push_front(val); }
		void push_back(value_type val) { GenericPtrList::push_back(val); }

		~PtrList<T>() { clear(); }
};

class ScriptPtrList : public GenericPtrList
{
	public:

#ifndef NOSCRIPT
	static GenericPtrList *ScriptFactory(asIObjectType *ot) { return new GenericPtrList(ot); }
#endif // NOSCRIPT

	SCRIPT_REGISTER_REF_TPL_NAMED(ScriptPtrList, "PtrList")
		SCRIPT_REGISTER_FACTORY("PtrList<T> @f(int &in)", ScriptFactory);

		// NOTE: This allows invalid conversions
		SCRIPT_ASSERT(engine.as->RegisterObjectBehaviour("GenericPtrList", asBEHAVE_REF_CAST, "PtrList<T> @f()", asFUNCTION(ScriptPtrCast), asCALL_CDECL_OBJLAST),
			"Failed to register cast from %s for %s", "GenericPtrList", ScriptTypeName());

		SCRIPT_REGISTER_BEHAVIOUR_STATIC_CC(asBEHAVE_IMPLICIT_REF_CAST, "GenericPtrList @f()", ScriptPtrCast, asCALL_CDECL_OBJLAST);

		SCRIPT_REGISTER_FUNCTION("uint size() const", size);
		SCRIPT_REGISTER_FUNCTION("uint max_size() const", max_size);
		SCRIPT_REGISTER_FUNCTION("bool empty()", empty);

		SCRIPT_REGISTER_FUNCTION("bool resize(uint)", resize);

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

		SCRIPT_REGISTER_FUNCTION("void clear()", clear);
	SCRIPT_REGISTER_END()
};

#endif // LIST_HPP_INCLUDED
