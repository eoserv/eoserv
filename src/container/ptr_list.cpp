
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "ptr_list.hpp"

#include <cstdlib>
#include <cstring>

GenericPtrList::value_type GenericPtrList::SafeIterator::Dereference()
{
	if (!this->InRange())
	{
		throw std::out_of_range("GenericPtrList::SafeIterator::Dereference");
	}

	if (this->list->script_ot)
	{
		this->list->script_ot->GetEngine()->AddRefScriptObject(*it, this->list->script_ot->GetSubTypeId());
	}

	return *this->it;
}

void GenericPtrList::SafeIterator::Set(GenericPtrList::value_type val)
{
	if (!this->InRange())
	{
		throw std::out_of_range("GenericPtrList::SafeIterator::Set");
	}

	*this->it = val;
}

const GenericPtrList::value_type GenericPtrList::SafeConstIterator::Dereference()
{
	if (!this->InRange())
	{
		throw std::out_of_range("GenericPtrList::SafeIterator::Dereference");
	}

	if (this->list->script_ot)
	{
		this->list->script_ot->GetEngine()->AddRefScriptObject(*this->it, this->list->script_ot->GetSubTypeId());
	}

	return *this->it;
}

void GenericPtrList::resize(size_type size)
{
	if (size < this->size())
	{
		std::list<value_type>::iterator it = this->data.end();

		for (std::size_t i = size; i < this->size(); ++i)
		{
			--it;
			this->ValueRelease(*it);
		}
	}

	this->data.resize(size);
	++this->rev;
}

void GenericPtrList::push_front(value_type val)
{
	this->ValueAddRef(val);
	this->data.push_front(val);
	++this->rev;
}

void GenericPtrList::pop_front()
{
	this->ValueRelease(this->data.front());
	this->data.pop_front();
	++this->rev;
}

void GenericPtrList::push_back(value_type val)
{
	this->ValueAddRef(val);
	this->data.push_back(val);
	++this->rev;
}

void GenericPtrList::pop_back()
{
	this->ValueRelease(this->data.front());
	this->data.pop_back();
	++this->rev;
}

GenericPtrList &GenericPtrList::assign(const GenericPtrList &other)
{
	for (std::list<value_type>::const_iterator it = this->data.begin(); it != this->data.end(); ++it)
	{
		this->ValueRelease(*it);
	}

	for (std::list<value_type>::const_iterator it = other.data.begin(); it != other.data.end(); ++it)
	{
		this->ValueAddRef(*it);
	}

	this->data = other.data;

	return *this;
}

void GenericPtrList::erase(GenericPtrList::SafeIterator &it)
{
	if (!it.InRange())
	{
		throw std::out_of_range("GenericPtrList::erase");
	}

	this->ValueRelease(*it.it);
	it.it = this->data.erase(it.it);
	++this->rev;
}

void GenericPtrList::insert(GenericPtrList::SafeIterator &it, GenericPtrList::value_type val)
{
	if (!it.InRange())
	{
		throw std::out_of_range("GenericPtrList::erase");
	}

	this->ValueAddRef(val);
	this->data.insert(it.it, val);
	++this->rev;
}

void GenericPtrList::erase(GenericPtrList::FastIterator &it)
{
	this->ValueRelease(*it.it);
	it.it = this->data.erase(it.it);
	++this->rev;
}

void GenericPtrList::insert(GenericPtrList::FastIterator &it, GenericPtrList::value_type val)
{
	this->ValueAddRef(val);
	this->data.insert(it.it, val);
	++this->rev;
}
