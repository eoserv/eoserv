
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef ALGORITHM_HPP_INCLUDED
#define ALGORITHM_HPP_INCLUDED

#include <stdexcept>

#include "iterator.hpp"
#include "shared.hpp"

template <class C, class T> bool erase_first(C &c, T val)
{
	for (typename C::Iterator it(c); it; ++it)
	{
		if (*it == val)
		{
			c.erase(it);
			return true;
		}
	}

	return false;
}

template <class C, class T> int erase_all(C &c, T val)
{
	int erased = 0;

	for (typename C::Iterator it(c); it; ++it)
	{
		if (*it == val)
		{
			c.erase(it);
			++erased;
		}
	}

	return erased;
}

#endif // ALGORITHM_HPP_INCLUDED
