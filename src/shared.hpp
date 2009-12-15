
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SHARED_HPP_INCLUDED
#define SHARED_HPP_INCLUDED

extern int shared_objects_allocated_;
extern int shared_references_;

class Shared
{
	protected:
		struct RefCount
		{
			int i;

			RefCount() : i(1) { ++shared_objects_allocated_; ++shared_references_; }

			int operator =(int j) { return i = j; }
			RefCount &operator --() { --i; --shared_references_; return *this; }
			RefCount &operator ++() { ++i; ++shared_references_; return *this; }

			operator int() { return i; }

			~RefCount() { --shared_objects_allocated_; shared_references_ -= i; }
		};

		RefCount refcount;

	public:
		Shared() { }
		Shared(const Shared &other) { }
		Shared &operator =(const Shared &other) { return *this; }

		void AddRef()
		{
			++refcount;
		}

		void Release()
		{
			if (--refcount == 0)
			{
				delete this;
				return;
			}
		}

		virtual ~Shared() { }
};

#endif // SHARED_HPP_INCLUDED
