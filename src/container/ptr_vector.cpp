
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "ptr_vector.hpp"

#include <cstdlib>
#include <cstring>

GenericPtrVector::value_type GenericPtrVector::SafeIterator::Dereference()
{
	if (!this->InRange())
	{
		throw std::out_of_range("GenericPtrVector::SafeIterator::Dereference");
	}

#ifndef NOSCRIPT
	if (this->v->script_ot)
	{
		this->v->script_ot->GetEngine()->AddRefScriptObject(this->v->data[this->pos], this->v->script_ot->GetSubTypeId());
	}
#endif // NOSCRIPT

	return this->v->data[this->pos];
}

void GenericPtrVector::SafeIterator::Set(GenericPtrVector::value_type val)
{
	if (!this->InRange())
	{
		throw std::out_of_range("GenericPtrVector::SafeIterator::Set");
	}

	this->v->data[this->pos] = val;
}

void GenericPtrVector::resize(size_type size)
{
	std::size_t oldsize = this->size();

	if (size < oldsize)
	{
		for (std::size_t i = size; i < oldsize; ++i)
		{
			this->Delete(this->data[i]);
		}
	}

	this->data.resize(size);
}

GenericPtrVector &GenericPtrVector::assign(const GenericPtrVector &other)
{
	for (std::vector<value_type>::const_iterator it = this->data.begin(); it != this->data.end(); ++it)
	{
		this->ValueRelease(*it);
	}

	for (std::vector<value_type>::const_iterator it = other.data.begin(); it != other.data.end(); ++it)
	{
		this->ValueAddRef(*it);
	}

	this->data = other.data;

	return *this;
}

void GenericPtrVector::erase(GenericPtrVector::SafeIterator &it)
{
	if (!it.InRange())
	{
		throw std::out_of_range("GenericPtrVector::erase");
	}

	std::vector<value_type>::iterator vit = this->data.begin() + it.pos;
	this->Delete(*vit);
	this->data.erase(vit);
}

void GenericPtrVector::insert(GenericPtrVector::SafeIterator &it, GenericPtrVector::value_type val)
{
	if (!it.InRange() && !it.IsEnd())
	{
		throw std::out_of_range("GenericPtrVector::insert");
	}

	this->ValueAddRef(val);
	this->data.insert(this->data.begin() + it.pos, val);
}
