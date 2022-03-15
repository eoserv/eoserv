/* dialog.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "dialog.hpp"

#include "packet.hpp"

#include "util.hpp"

#include <cstddef>
#include <string>
#include <utility>

Dialog::Dialog()
{

}

void Dialog::AddPage(const std::string& text)
{
	this->pages.push_back(text);
}

void Dialog::AddLink(int id, const std::string& text)
{
	this->links.insert(std::make_pair(id, text));
}

bool Dialog::CheckLink(int id) const
{
	return this->links.find(id) != this->links.end();
}

int Dialog::PacketLength() const
{
	std::size_t size = this->pages.size() * 3;
	size += this->links.size() * 5;

	UTIL_FOREACH(this->pages, page)
	{
		size += page.length();
	}

	UTIL_FOREACH(this->links, link)
	{
		size += link.second.length();
	}

	return size;
}

void Dialog::BuildPacket(PacketBuilder& builder) const
{
	builder.ReserveMore(this->PacketLength());

	UTIL_FOREACH(this->pages, page)
	{
		builder.AddShort(DIALOG_TEXT);
		builder.AddBreakString(page);
	}

	UTIL_FOREACH(this->links, link)
	{
		builder.AddShort(DIALOG_LINK);
		builder.AddShort(link.first);
		builder.AddBreakString(link.second);
	}
}

Dialog::~Dialog()
{

}
