
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "guild.hpp"

#include "character.hpp"
#include "eoclient.hpp"
#include "packet.hpp"
#include "world.hpp"
#include "player.hpp"

std::string RankSerialize(util::array<std::string, 9> list)
{
	std::string serialized;

	UTIL_ARRAY_FOREACH_ALL(list, std::string, 9, rank)
	{
		serialized.append(rank);
		serialized.append(",");
	}

	return serialized;
}

util::array<std::string, 9> RankUnserialize(std::string serialized)
{
	util::array<std::string, 9> list;
	std::size_t p = 0;
	std::size_t lastp = std::numeric_limits<std::size_t>::max();
	int i = 0;

	if (!serialized.empty() && *(serialized.end()-1) != ',')
	{
		serialized.push_back(',');
	}

	while ((p = serialized.find_first_of(',', p+1)) != std::string::npos)
	{
		list[i++] = serialized.substr(lastp+1, p-lastp-1);
		lastp = p;
	}

	return list;
}

Guild_Create::Guild_Create(GuildManager *manager, std::string tag, std::string name, Character *leader)
{
	tag = util::uppercase(tag);

	this->manager = manager;
	manager->AddRef();
	this->tag = tag;
	this->name = name;
	this->leader = leader;
	leader->AddRef();
	Guild_Member *member = new Guild_Member(leader->name, 0);
	this->members.push_back(member);
	member->Release();
}

bool Guild_Create::HasMember(std::string character)
{
	character = util::lowercase(character);

	UTIL_PTR_VECTOR_FOREACH(this->members, Guild_Member, check)
	{
		if (character == check->name)
		{
			return true;
		}
	}

	return false;
}

void Guild_Create::AddMember(std::string character)
{
	character = util::lowercase(character);

	Guild_Member *member = new Guild_Member(character, 9);
	this->members.push_back(member);
	member->Release();
}

Guild_Create::~Guild_Create()
{
	this->manager->CancelCreate(this);
	this->leader->Release();

	if (!this->manager->cache_clearing)
	{
		std::map<std::string, Guild *>::iterator findentry = this->manager->cache.find(tag);

		if (findentry != this->manager->cache.end())
		{
			findentry->second->Release();
			this->manager->cache.erase(findentry);
		}
	}

	this->manager->Release();
}

Guild *GuildManager::GetGuild(std::string tag)
{
	tag = util::uppercase(tag);

	std::map<std::string, Guild *>::iterator findguild = this->cache.find(tag);

	if (findguild != this->cache.end())
	{
		findguild->second->AddRef();
		return findguild->second;
	}
	else
	{
		Database_Result res = this->world->db.Query("SELECT `tag`, `name`, `description`, `created`, `ranks`, `bank` FROM `guilds` WHERE `tag` = '$'", tag.c_str());

		if (res.empty())
		{
			return 0;
		}

		std::map<std::string, util::variant> row = res.front();
		Guild *guild = new Guild(this);
		guild->tag = static_cast<std::string>(row["tag"]);
		guild->name = static_cast<std::string>(row["name"]);
		guild->description = static_cast<std::string>(row["description"]);
		guild->created = static_cast<int>(row["created"]);
		guild->ranks = RankUnserialize(static_cast<std::string>(row["ranks"]));
		guild->bank = static_cast<int>(row["bank"]);

		res = this->world->db.Query("SELECT `name`, `guild_rank` FROM `characters` WHERE `guild` = '$' ORDER BY `guild_rank` ASC, `name` ASC", tag.c_str());

		typedef std::map<std::string, util::variant> Database_Row;
		UTIL_VECTOR_FOREACH_ALL(res, Database_Row, row)
		{
			Guild_Member *member = new Guild_Member(row["name"], row["guild_rank"]);
			guild->members.push_back(member);
			member->Release();
		}

		return this->cache[tag] = guild;
	}
}

Guild *GuildManager::GetGuildName(std::string name)
{
	name = util::lowercase(name);

	UTIL_MAP_IFOREACH_ALL(this->cache, std::string, Guild *, entry)
	{
		if (entry->second->name == name)
		{
			entry->second->AddRef();
			return entry->second;
		}
	}

	Database_Result res = this->world->db.Query("SELECT `tag`, `name`, `description`, `created`, `ranks`, `bank` FROM `guilds` WHERE `name` = '$'", name.c_str());

	if (res.empty())
	{
		return 0;
	}

	std::map<std::string, util::variant> row = res.front();
	Guild *guild = new Guild(this);
	guild->tag = static_cast<std::string>(row["tag"]);
	guild->name = static_cast<std::string>(row["name"]);
	guild->description = static_cast<std::string>(row["description"]);
	guild->created = static_cast<int>(row["created"]);
	guild->ranks = RankUnserialize(static_cast<std::string>(row["ranks"]));
	guild->bank = static_cast<int>(row["bank"]);

	res = this->world->db.Query("SELECT `name`, `guild_rank` FROM `characters` WHERE `guild` = '$' ORDER BY `guild_rank` ASC, `name` ASC", static_cast<std::string>(row["tag"]).c_str());

	typedef std::map<std::string, util::variant> Database_Row;
	UTIL_VECTOR_FOREACH_ALL(res, Database_Row, row)
	{
		Guild_Member *member = new Guild_Member(row["name"], row["guild_rank"]);
		guild->members.push_back(member);
		member->Release();
	}

	return this->cache[row["tag"]] = guild;
}

Guild_Create *GuildManager::GetCreate(std::string tag)
{
	tag = util::uppercase(tag);

	std::map<std::string, Guild_Create *>::iterator findcreate = this->create_cache.find(tag);

	if (findcreate != this->create_cache.end())
	{
		findcreate->second->AddRef();
		return findcreate->second;
	}
	else
	{
		return 0;
	}
}

Guild_Create *GuildManager::BeginCreate(std::string tag, std::string name, Character *leader)
{
	tag = util::uppercase(tag);
	name = util::lowercase(name);

	Guild_Create *create = new Guild_Create(this, tag, name, leader);
	this->create_cache[tag] = create;
	create->AddRef();
	return create;
}

void GuildManager::CancelCreate(Guild_Create *create)
{
	this->create_cache.erase(create->tag);
	create->Release();
}

Guild *GuildManager::CreateGuild(Guild_Create *create, std::string description)
{
	PtrVector<Guild_Member> members;

	description = util::text_word_wrap(description, this->world->config["GuildMaxWidth"]);

	this->world->db.Query("INSERT INTO `guilds` (`tag`, `name`, `description`, `created`, `ranks`) VALUES ('$', '$', '$', #, '$')", create->tag.c_str(), create->name.c_str(), description.c_str(), time(0), static_cast<std::string>(this->world->config["GuildDefaultRanks"]).c_str());

	this->create_cache.erase(create->tag);

	Guild *guild = this->GetGuild(create->tag);

	UTIL_PTR_VECTOR_FOREACH(create->members, Guild_Member, member)
	{
		Character *character = this->world->GetCharacter(member->name);

		if (character)
		{
			guild->AddMember(character, create->leader, false, (character == create->leader) ? 0 : 9);
		}
	}

	this->create_cache.erase(create->tag);

	return guild;
}

void GuildManager::SaveAll()
{
	UTIL_MAP_IFOREACH_ALL(this->cache, std::string, Guild *, entry)
	{
		entry->second->Save();
	}
}

GuildManager::~GuildManager()
{
	this->cache_clearing = true;

	UTIL_MAP_IFOREACH_ALL(this->create_cache, std::string, Guild_Create *, entry)
	{
		entry->second->Release();
	}

	UTIL_MAP_IFOREACH_ALL(this->cache, std::string, Guild *, entry)
	{
		entry->second->Release();
	}

	this->world->Release();
}

bool Guild::ValidName(std::string name)
{
	name = util::lowercase(name);

	if (name.length() < 4)
	{
		return false;
	}

	for (std::size_t i = 0; i < name.length(); ++i)
	{
		if ((name[i] < 'a' || name[i] > 'z') && name[i] != ' ')
		{
			return false;
		}
	}

	return true;
}

bool Guild::ValidTag(std::string tag)
{
	tag = util::uppercase(tag);

	if (tag.length() < 2 || tag.length() > 3)
	{
		return false;
	}

	for (std::size_t i = 0; i < tag.length(); ++i)
	{
		if (tag[i] < 'A' || tag[i] > 'Z')
		{
			return false;
		}
	}

	return true;
}

void Guild::AddMember(Character *joined, Character *recruiter, bool alert, int rank)
{
	joined->guild = this;
	this->AddRef();
	joined->guild_rank = rank;

	Guild_Member *member = new Guild_Member(joined->name, rank);
	this->members.push_back(member);
	member->Release();

	if (recruiter != joined) // Leader of new guild
	{
		PacketBuilder builder(PACKET_GUILD, PACKET_AGREE);
		builder.AddShort(recruiter->player->id);
		builder.AddByte(255);
		builder.AddBreakString(this->tag);
		builder.AddBreakString(this->name);
		builder.AddBreakString(this->GetRank(rank));
		joined->player->client->SendBuilder(builder);
	}

	if (alert && this->manager->world->config["GuildAnnounce"])
	{
		std::string name = joined->name;

		std::string msg = "***  ";
		msg += util::ucfirst(name);
		msg += " has joined the guild.";

		if (recruiter)
		{
			msg += " Recruited by ";
			msg += util::ucfirst(recruiter->name);
		}

		this->Msg(0, msg);
	}
}

void Guild::DelMember(std::string kicked, Character *kicker, bool alert)
{
	kicked = util::lowercase(kicked);

	if (alert && this->manager->world->config["GuildAnnounce"])
	{
		std::string msg = "***  ";
		msg += util::ucfirst(kicked);
		msg += " left the guild.";

		if (kicker)
		{
			msg += " Kicked by ";
			msg += util::ucfirst(kicker->name);
		}

		this->Msg(0, msg);
	}

	UTIL_PTR_VECTOR_FOREACH(this->manager->world->characters, Character, character)
	{
		if (character->name == kicked)
		{
			character->guild->Release();
			character->guild = 0;
			character->guild_rank = 0;
		}
	}

	UTIL_PTR_VECTOR_FOREACH(this->members, Guild_Member, member)
	{
		if (member->name == kicked)
		{
			this->members.erase(member);
			break;
		}
	}
}

void Guild::SetMemberRank(std::string name, int rank)
{
	Guild_Member *member = GetMember(name);

	if (member)
	{
		member->rank = rank;

		UTIL_PTR_VECTOR_FOREACH(this->manager->world->characters, Character, character)
		{
			if (character->name == name)
			{
				character->guild_rank = rank;
			}
		}
	}
}

void Guild::AddBank(int gold)
{
	if (gold > 0 && this->bank + gold >= 0)
	{
		this->needs_save = true;
		this->bank += gold;
		this->bank = std::min<int>(this->bank, this->manager->world->config["GuildBankMax"]);
	}
}

void Guild::DelBank(int gold)
{
	if (gold > 0 && this->bank >= 0 && this->bank - gold > 0)
	{
		this->needs_save = true;
		this->bank -= gold;
	}
}

Guild_Member *Guild::GetMember(std::string name)
{
	name = util::lowercase(name);

	UTIL_PTR_VECTOR_FOREACH(this->members, Guild_Member, member)
	{
		if (member->name == name)
		{
			return *member;
		}
	}

	return 0;
}

void Guild::SetDescription(std::string description)
{
	this->needs_save = true;

	for (std::string::iterator i = description.begin(); i != description.end(); ++i)
	{
		if (static_cast<unsigned char>(*i) == 0xFF)
		{
			*i = 'y';
		}
	}

	this->description = util::text_word_wrap(description, this->manager->world->config["GuildMaxWidth"]);
}

void Guild::Msg(Character *from, std::string message, bool echo)
{
	message = util::text_cap(message, static_cast<int>(this->manager->world->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from ? from->name : "Server") + "  "));

	PacketBuilder builder(PACKET_TALK, PACKET_REQUEST);
	builder.AddBreakString(from ? from->name : "Server");
	builder.AddBreakString(message);

	UTIL_PTR_VECTOR_FOREACH(this->manager->world->characters, Character, character)
	{
		if (!echo && *character == from)
		{
			continue;
		}

		if (character->guild == this)
		{
			character->player->client->SendBuilder(builder);
		}
	}
}

void Guild::Save()
{
	if (this->needs_save)
	{
		this->manager->world->db.Query("UPDATE `guilds` SET `description` = '$', `ranks` = '$', `bank` = # WHERE tag = '$'", this->description.c_str(), RankSerialize(this->ranks).c_str(), this->bank, this->tag.c_str());
		this->needs_save = false;
	}
}

Guild::~Guild()
{
	if (!this->manager->cache_clearing)
	{
		std::map<std::string, Guild *>::iterator findentry = this->manager->cache.find(tag);

		if (findentry != this->manager->cache.end())
		{
			findentry->second->Release();
			this->manager->cache.erase(findentry);
		}
	}

	this->Save();

	this->manager->Release();
}
