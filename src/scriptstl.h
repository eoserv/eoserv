
/* scriptstl.h
 * Copyright 2009 the EOSERV development team (http://eoserv.net/devs)
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef SCRIPTSTL_H
#define SCRIPTSTL_H

// TODO: support more containers
// reverse iterators are not (yet?) supported
// const_iterator returning versions of functions are pre-fixed with c (as in C++0x)

#include <string>
#include <vector>
#include <list>
#include <typeinfo>

#include <angelscript.h>

#include "console.hpp"

#ifndef SCRIPT_ASSERT
#define SCRIPT_ASSERT(expr, format, ...) { int r; if ((r = (expr)) < 0){ Console::Err("SCRIPT ERROR (%i) "format, r, __VA_ARGS__); return false; } }
#endif

BEGIN_AS_NAMESPACE

template <class T> void STLConstructor(T *in) { new (in) T; }
template <class T> void STLCopyConstructor(const T &rhs, T *in) { new (in) T(rhs); }
template <class T> void STLDestructor(T *in) { in->~T(); }

template <class T, class U> bool STLEquals(const T &x, const U &y) { return x == y; }
template <class T, class U> int  STLCmp   (const T &x, const U &y) { return x < y; }

template <class T, class U> T &STLAddAssign (T &x, U y) { return x += y; }
template <class T, class U> T &STLSubAssign (T &x, U y) { return x -= y; }
template <class T, class U> T &STLMulAssign (T &x, U y) { return x *= y; }
template <class T, class U> T &STLDivAssign (T &x, U y) { return x /= y; }
template <class T, class U> T &STLModAssign (T &x, U y) { return x %= y; }
template <class T, class U> T &STLOrAssign  (T &x, U y) { return x |= y; }
template <class T, class U> T &STLAndAssign (T &x, U y) { return x &= y; }
template <class T, class U> T &STLXorAssign (T &x, U y) { return x ^= y; }
template <class T, class U> T &STLShlAssign (T &x, U y) { return x <<= y; }
template <class T, class U> T &STLShrAssign (T &x, U y) { return x >>= y; }
template <class T, class U> T &STLUShrAssign(T &x, U y) { return x >>= y; }

// This entire wrapper is only needed because std::vector<T>::iterator disagrees with AngelScript
// This assumes STLVector<T> is identical to std::vector<T> (fairly safe assumption) (should individual methods be used instead?)
// There is no need to reference this class in your programs (except maybe for assurance of identical-ness)
template <class T> class STLVector : public std::vector<T>
{
	typedef std::vector<T> VT;
	typedef typename VT::iterator VTI;
	typedef typename VT::const_iterator VTCI;

	public:
		class iterator
		{
			public:
				VTI iter;
				char pad1[8]; // Without this AngelScript dies calling a function that returns an iterator

				iterator &operator ++() { iter++; return *this; }
				iterator &operator --() { iter--; return *this; }
				T &operator *() const { return *iter; }

				iterator() : iter(0) { }
				iterator(VTI iter_) : iter(iter_) { }

				operator VTI () { return iter; }

				bool operator ==(const iterator &x) const { return iter == x.iter; }
				bool operator !=(const iterator &x) const { return iter != x.iter; }
				bool operator <(const iterator &x) const { return iter < x.iter; }
				bool operator >(const iterator &x) const { return iter > x.iter; }
				bool operator <=(const iterator &x) const { return iter <= x.iter; }
				bool operator >=(const iterator &x) const { return iter >= x.iter; }
		};

		class const_iterator
		{
			public:
				VTCI iter;
				char pad1[8]; // Without this AngelScript dies calling a function that returns an iterator

				const_iterator &operator ++() { iter++; return *this; }
				const_iterator &operator --() { iter--; return *this; }
				const T &operator *() const { return *iter; }

				const_iterator() : iter(0) { }
				const_iterator(VTCI iter_) : iter(iter_) { }

				operator VTCI () { return iter; }

				bool operator ==(const const_iterator &x) const { return iter == x.iter; }
				bool operator !=(const const_iterator &x) const { return iter != x.iter; }
				bool operator <(const const_iterator &x) const { return iter < x.iter; }
				bool operator >(const const_iterator &x) const { return iter > x.iter; }
				bool operator <=(const const_iterator &x) const { return iter <= x.iter; }
				bool operator >=(const const_iterator &x) const { return iter >= x.iter; }
		};

		iterator begin() { return VT::begin(); }
		const_iterator begin() const { return VT::begin(); }

		iterator end() { return VT::end(); }
		const_iterator end() const { return VT::end(); }

		iterator erase(iterator position) { return VT::erase(position); };
		iterator erase(iterator first, iterator last) { return VT::erase(first, last); };

		iterator insert(iterator position, typename VT::const_reference x) { return VT::insert(position, x); }
		void insert(iterator position, typename VT::size_type n, typename VT::const_reference x) { VT::insert(position, n, x); }
		//template <class I> void insert(iterator position, I first, I last) { VT::insert(position, first, last); }

		// Is there a way to set a default parameter through AngelScript instead?
		void resize(typename VT::size_type n) { VT::resize(n); }
		void resize(typename VT::size_type n, T x) { VT::resize(n, x); }
};

template <typename T> bool RegisterScriptVector(const char *arraytype, const char *type, asIScriptEngine *engine)
{
	typedef STLVector<T> VT;
	typedef typename VT::iterator VTI;
	typedef typename VT::const_iterator VTCI;

	if (sizeof(VT) != sizeof(std::vector<T>))
	{
		SCRIPT_ASSERT(-1, "Cannot register %s, sizes are different", arraytype);
	}

	std::string itarraytype_str = std::string(arraytype) + "_iterator";
	std::string citarraytype_str = std::string(arraytype) + "_const_iterator";
	std::string sizetype_str = std::string(arraytype) + "_size_type";

	const char *itarraytype = itarraytype_str.c_str();
	const char *citarraytype = citarraytype_str.c_str();
	const char *sizetype = sizetype_str.c_str();

	const char *dtype;

	SCRIPT_ASSERT(engine->RegisterObjectType(dtype = arraytype, sizeof(VT), asOBJ_VALUE | asOBJ_APP_CLASS_CDA),
		"Failed to register type for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectType(dtype = itarraytype, sizeof(VTI), asOBJ_VALUE | asOBJ_APP_CLASS_CDA | asOBJ_POD),
		"Failed to register type for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectType(dtype = citarraytype, sizeof(VTI), asOBJ_VALUE | asOBJ_APP_CLASS_CDA | asOBJ_POD),
		"Failed to register type for %s", dtype);

	const char *sizetype_type = "uint";

	if (sizeof(typename VT::size_type) == 8)
	{
		sizetype_type = "uint64";
	}

	SCRIPT_ASSERT(engine->RegisterTypedef(dtype = sizetype, sizetype_type),
		"Failed to register typedef for %s", dtype);

	// Constructor / destructor / assign

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = arraytype, asBEHAVE_CONSTRUCT, "void f()",
		asFUNCTION(STLConstructor<VT>), asCALL_CDECL_OBJFIRST),
			"Failed to register construct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = arraytype, asBEHAVE_CONSTRUCT, (std::string("void f(") + arraytype + " &in)").c_str(),
		asFUNCTION(STLCopyConstructor<VT>), asCALL_CDECL_OBJFIRST),
			"Failed to register copy construct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = arraytype, asBEHAVE_DESTRUCT, "void f()",
		asFUNCTION(STLDestructor<VT>), asCALL_CDECL_OBJFIRST),
			"Failed to register destruct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = itarraytype, asBEHAVE_CONSTRUCT, "void f()",
		asFUNCTION(STLConstructor<VTI>), asCALL_CDECL_OBJFIRST),
			"Failed to register construct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = itarraytype, asBEHAVE_CONSTRUCT, (std::string("void f(") + itarraytype + " &in)").c_str(),
		asFUNCTION(STLCopyConstructor<VTI>), asCALL_CDECL_OBJFIRST),
			"Failed to register copy construct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = itarraytype, asBEHAVE_DESTRUCT, "void f()",
		asFUNCTION(STLDestructor<VTI>), asCALL_CDECL_OBJFIRST),
			"Failed to register destruct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, (std::string(arraytype) + " &opAssign(const " + arraytype + " &in)").c_str(),
		asMETHODPR(VT, operator=, (const VT &), VT &), asCALL_THISCALL),
			"Failed to register assignment operator for %s", dtype);

	// Operators

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, (std::string("bool opEquals(const ") + itarraytype + " &in) const").c_str(),
		asFUNCTION((STLEquals<VTI, VTI>)), asCALL_CDECL_OBJFIRST),
			"Failed to register == operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, (std::string("int opCmp(const ") + itarraytype + " &in) const").c_str(),
		asFUNCTION((STLCmp<VTI, VTI>)), asCALL_CDECL_OBJFIRST),
			"Failed to register < operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, (std::string(itarraytype) + " &opAddAssign(int)").c_str(),
		asFUNCTION((STLAddAssign<typename std::vector<T>::iterator, int>)), asCALL_CDECL_OBJFIRST),
			"Failed to register += operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = citarraytype, (std::string("bool opEquals(const ") + citarraytype + " &in) const").c_str(),
		asFUNCTION((STLEquals<VTI, VTI>)), asCALL_CDECL_OBJFIRST),
			"Failed to register == operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = citarraytype, (std::string("int opCmp(const ") + citarraytype + " &in) const").c_str(),
		asFUNCTION((STLCmp<VTI, VTI>)), asCALL_CDECL_OBJFIRST),
			"Failed to register < operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = citarraytype, (std::string(citarraytype) + " &opAddAssign(int)").c_str(),
		asFUNCTION((STLAddAssign<typename std::vector<T>::iterator, int>)), asCALL_CDECL_OBJFIRST),
			"Failed to register += operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = arraytype, asBEHAVE_INDEX, (std::string(type) + " &f(int)").c_str(),
		asMETHODPR(VT, at, (size_t), T &), asCALL_THISCALL),
			"Failed to register [] operator for %s", dtype);

	// Functions

	const char *decl;

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(itarraytype) + " begin()").c_str(),
		asMETHODPR(VT, begin, (), VTI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(itarraytype) + " end()").c_str(),
		asMETHODPR(VT, end, (), VTI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(citarraytype) + " cbegin() const").c_str(),
		asMETHODPR(VT, begin, () const, VTCI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(citarraytype) + " cend() const").c_str(),
		asMETHODPR(VT, end, () const, VTCI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(sizetype) + " size()").c_str(),
		asMETHODPR(VT, size, () const, std::size_t), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(sizetype) + " max_size()").c_str(),
		asMETHODPR(VT, max_size, () const, std::size_t), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void resize(") + sizetype + " sz)").c_str(),
		asMETHODPR(VT, resize, (std::size_t), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void resize(") + sizetype + " sz, " + type + " c)").c_str(),
		asMETHODPR(VT, resize, (std::size_t, T), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(sizetype) + " capacity()").c_str(),
		asMETHODPR(VT, capacity, () const, std::size_t), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = "bool empty()",
		asMETHODPR(VT, empty, () const, bool), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void reserve(") + sizetype + " sz)").c_str(),
		asMETHODPR(VT, reserve, (std::size_t), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(type) + " &at()").c_str(),
		asMETHODPR(VT, at, (std::size_t), T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(type) + " &front()").c_str(),
		asMETHODPR(VT, front, (), T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(type) + " &back()").c_str(),
		asMETHODPR(VT, back, (), T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void assign(") + sizetype + " n, const " + type + " &in)").c_str(),
		asMETHODPR(VT, assign, (std::size_t, const T &), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void push_back(const ") + type + " &in)").c_str(),
		asMETHODPR(VT, push_back, (const T &), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = "void pop_back()",
		asMETHODPR(VT, pop_back, (), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(itarraytype) + " insert(" + itarraytype + ", " + type + " &in)").c_str(),
		asMETHODPR(VT, insert, (VTI, const T &), VTI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = "void clear()",
		asMETHODPR(VT, clear, (), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(type) + " &get() const").c_str(),
		asMETHODPR(VTI, operator*, () const, T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(itarraytype) + " &inc()").c_str(),
		asMETHODPR(VTI, operator++, (), VTI &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(itarraytype) + " &dec()").c_str(),
		asMETHODPR(VTI, operator--, (), VTI &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = citarraytype, decl = (std::string("const ") + type + " &cget() const").c_str(),
		asMETHODPR(VTCI, operator*, () const, const T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(citarraytype) + " &cinc()").c_str(),
		asMETHODPR(VTCI, operator++, (), VTCI &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(citarraytype) + " &cdec()").c_str(),
		asMETHODPR(VTCI, operator--, (), VTCI &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	return true;
}


// This entire wrapper is only needed because std::list<T>::iterator disagrees with AngelScript
// This assumes STLList<T> is identical to std::list<T> (fairly safe assumption) (should individual methods be used instead?)
// There is no need to reference this class in your programs (except maybe for assurance of identical-ness)
template <class T> class STLList : public std::list<T>
{
	typedef std::list<T> LT;
	typedef typename LT::iterator LTI;
	typedef typename LT::const_iterator LTCI;

	public:
		class iterator
		{
			public:
				LTI iter;
				char pad1[8]; // Without this AngelScript dies calling a function that returns an iterator

				iterator &operator ++() { iter++; return *this; }
				iterator &operator --() { iter--; return *this; }
				T &operator *() const { return *iter; }

				iterator() : iter(0) { }
				iterator(LTI iter_) : iter(iter_) { }

				operator LTI () { return iter; }

				bool operator ==(const iterator &x) const { return iter == x.iter; }
				bool operator !=(const iterator &x) const { return iter != x.iter; }
		};

		class const_iterator
		{
			public:
				LTCI iter;
				char pad1[8]; // Without this AngelScript dies calling a function that returns an iterator

				const_iterator &operator ++() { iter++; return *this; }
				const_iterator &operator --() { iter--; return *this; }
				const T &operator *() const { return *iter; }

				const_iterator() : iter(0) { }
				const_iterator(LTCI iter_) : iter(iter_) { }

				operator LTCI () { return iter; }

				bool operator ==(const const_iterator &x) const { return iter == x.iter; }
				bool operator !=(const const_iterator &x) const { return iter != x.iter; }
		};

		iterator begin() { return LT::begin(); }
		const_iterator begin() const { return LT::begin(); }

		iterator end() { return LT::end(); }
		const_iterator end() const { return LT::end(); }

		iterator erase(iterator position) { return LT::erase(position); };
		iterator erase(iterator first, iterator last) { return LT::erase(first, last); };

		iterator insert(iterator position, typename LT::const_reference x) { return LT::insert(position, x); }
		void insert(iterator position, typename LT::size_type n, typename LT::const_reference x) { LT::insert(position, n, x); }
		//template <class I> void insert(iterator position, I first, I last) { LT::insert(position, first, last); }

		// Is there a way to set a default parameter through AngelScript instead?
		void resize(typename LT::size_type n) { LT::resize(n); }
		void resize(typename LT::size_type n, T x) { LT::resize(n, x); }
};

template <typename T> bool RegisterScriptList(const char *arraytype, const char *type, asIScriptEngine *engine)
{
	typedef STLList<T> LT;
	typedef typename LT::iterator LTI;
	typedef typename LT::const_iterator LTCI;

	if (sizeof(LT) != sizeof(std::list<T>))
	{
		SCRIPT_ASSERT(-1, "Cannot register %s, sizes are different", arraytype);
	}

	std::string itarraytype_str = std::string(arraytype) + "_iterator";
	std::string citarraytype_str = std::string(arraytype) + "_const_iterator";
	std::string sizetype_str = std::string(arraytype) + "_size_type";

	const char *itarraytype = itarraytype_str.c_str();
	const char *citarraytype = citarraytype_str.c_str();
	const char *sizetype = sizetype_str.c_str();

	const char *dtype;

	SCRIPT_ASSERT(engine->RegisterObjectType(dtype = arraytype, sizeof(LT), asOBJ_VALUE | asOBJ_APP_CLASS_CDA),
		"Failed to register type for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectType(dtype = itarraytype, sizeof(LTI), asOBJ_VALUE | asOBJ_APP_CLASS_CDA | asOBJ_POD),
		"Failed to register type for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectType(dtype = citarraytype, sizeof(LTI), asOBJ_VALUE | asOBJ_APP_CLASS_CDA | asOBJ_POD),
		"Failed to register type for %s", dtype);

	const char *sizetype_type = "uint";

	if (sizeof(typename LT::size_type) == 8)
	{
		sizetype_type = "uint64";
	}

	SCRIPT_ASSERT(engine->RegisterTypedef(dtype = sizetype, sizetype_type),
		"Failed to register typedef for %s", dtype);

	// Constructor / destructor / assign

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = arraytype, asBEHAVE_CONSTRUCT, "void f()",
		asFUNCTION(STLConstructor<LT>), asCALL_CDECL_OBJFIRST),
			"Failed to register construct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = arraytype, asBEHAVE_CONSTRUCT, (std::string("void f(") + arraytype + " &in)").c_str(),
		asFUNCTION(STLCopyConstructor<LT>), asCALL_CDECL_OBJFIRST),
			"Failed to register copy construct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = arraytype, asBEHAVE_DESTRUCT, "void f()",
		asFUNCTION(STLDestructor<LT>), asCALL_CDECL_OBJFIRST),
			"Failed to register destruct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = itarraytype, asBEHAVE_CONSTRUCT, "void f()",
		asFUNCTION(STLConstructor<LTI>), asCALL_CDECL_OBJFIRST),
			"Failed to register construct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = itarraytype, asBEHAVE_CONSTRUCT, (std::string("void f(") + itarraytype + " &in)").c_str(),
		asFUNCTION(STLCopyConstructor<LTI>), asCALL_CDECL_OBJFIRST),
			"Failed to register copy construct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectBehaviour(dtype = itarraytype, asBEHAVE_DESTRUCT, "void f()",
		asFUNCTION(STLDestructor<LTI>), asCALL_CDECL_OBJFIRST),
			"Failed to register destruct behaviour for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, (std::string(arraytype) + " &opAssign(const " + arraytype + " &in)").c_str(),
		asMETHODPR(LT, operator=, (const LT &), LT &), asCALL_THISCALL),
			"Failed to register assignment operator for %s", dtype);

	// Operators

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, (std::string("bool opEquals(const ") + itarraytype + " &in) const").c_str(),
		asFUNCTION((STLEquals<LTI, LTI>)), asCALL_CDECL_OBJFIRST),
			"Failed to register == operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, (std::string(itarraytype) + " &opAddAssign(int)").c_str(),
		asFUNCTION((STLAddAssign<typename std::vector<T>::iterator, int>)), asCALL_CDECL_OBJFIRST),
			"Failed to register += operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = citarraytype, (std::string("bool opEquals(const ") + citarraytype + " &in) const").c_str(),
		asFUNCTION((STLEquals<LTI, LTI>)), asCALL_CDECL_OBJFIRST),
			"Failed to register == operator for %s", dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = citarraytype, (std::string(citarraytype) + " &opAddAssign(int)").c_str(),
		asFUNCTION((STLAddAssign<typename std::vector<T>::iterator, int>)), asCALL_CDECL_OBJFIRST),
			"Failed to register += operator for %s", dtype);

	// Functions

	const char *decl;

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(itarraytype) + " begin()").c_str(),
		asMETHODPR(LT, begin, (), LTI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(itarraytype) + " end()").c_str(),
		asMETHODPR(LT, end, (), LTI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(citarraytype) + " cbegin() const").c_str(),
		asMETHODPR(LT, begin, () const, LTCI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(citarraytype) + " cend() const").c_str(),
		asMETHODPR(LT, end, () const, LTCI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(sizetype) + " size()").c_str(),
		asMETHODPR(LT, size, () const, std::size_t), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(sizetype) + " max_size()").c_str(),
		asMETHODPR(LT, max_size, () const, std::size_t), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void resize(") + sizetype + " sz)").c_str(),
		asMETHODPR(LT, resize, (std::size_t), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void resize(") + sizetype + " sz, " + type + " c)").c_str(),
		asMETHODPR(LT, resize, (std::size_t, T), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = "bool empty()",
		asMETHODPR(LT, empty, () const, bool), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(type) + " &front()").c_str(),
		asMETHODPR(LT, front, (), T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(type) + " &back()").c_str(),
		asMETHODPR(LT, back, (), T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void assign(") + sizetype + " n, const " + type + " &in)").c_str(),
		asMETHODPR(LT, assign, (std::size_t, const T &), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void push_front(const ") + type + " &in)").c_str(),
		asMETHODPR(LT, push_front, (const T &), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = "void pop_front()",
		asMETHODPR(LT, pop_front, (), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string("void push_back(const ") + type + " &in)").c_str(),
		asMETHODPR(LT, push_back, (const T &), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = "void pop_back()",
		asMETHODPR(LT, pop_back, (), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = (std::string(itarraytype) + " insert(" + itarraytype + ", " + type + " &in)").c_str(),
		asMETHODPR(LT, insert, (LTI, const T &), LTI), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = arraytype, decl = "void clear()",
		asMETHODPR(LT, clear, (), void), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(type) + " &get() const").c_str(),
		asMETHODPR(LTI, operator*, () const, T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(itarraytype) + " &inc()").c_str(),
		asMETHODPR(LTI, operator++, (), LTI &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(itarraytype) + " &dec()").c_str(),
		asMETHODPR(LTI, operator--, (), LTI &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = citarraytype, decl = (std::string("const ") + type + " &cget() const").c_str(),
		asMETHODPR(LTCI, operator*, () const, const T &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(citarraytype) + " &cinc()").c_str(),
		asMETHODPR(LTCI, operator++, (), LTCI &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	SCRIPT_ASSERT(engine->RegisterObjectMethod(dtype = itarraytype, decl = (std::string(citarraytype) + " &cdec()").c_str(),
		asMETHODPR(LTCI, operator--, (), LTCI &), asCALL_THISCALL),
			"Failed to register %s for %s", decl, dtype);

	return true;
}

END_AS_NAMESPACE

#endif // SCRIPTSTL_H
