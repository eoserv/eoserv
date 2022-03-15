/* guild.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "guild.hpp"

#include "character.hpp"
#include "config.hpp"
#include "eoclient.hpp"
#include "eoserver.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "world.hpp"

#include "util.hpp"
#include "util/variant.hpp"

#include <algorithm>
#include <array>
#include <ctime>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

std::string RankSerialize(std::array<std::string, 9> list)
{
	std::string serialized;

	UTIL_FOREACH(list, rank)
	{
		serialized.append(rank);
		serialized.append(",");
	}

	return serialized;
}

std::array<std::string, 9> RankUnserialize(std::string serialized)
{
	std::array<std::string, 9> list;
	std::size_t p = 0;
	std::size_t lastp = std::numeric_limits<std::size_t>::max();
	int i = 0;

	if (!serialized.empty() && *(serialized.end()-1) != ',')
	{
		serialized.push_back(',');
	}

	while (i < 9 && (p = serialized.find_first_of(',', lastp+1)) != std::string::npos)
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
	this->tag = tag;
	this->name = name;
	this->leader = leader;

	this->AddMember(leader->real_name, 0);
}

Guild_Member *Guild_Create::GetMember(std::string character)
{
	character = util::lowercase(character);

	UTIL_FOREACH(this->members, check)
	{
		if (character == check->name)
		{
			return check.get();
		}
	}

	return 0;
}

void Guild_Create::AddMember(std::string character, int rank)
{
	character = util::lowercase(character);

	this->members.push_back(std::make_shared<Guild_Member>(character, rank));
}

Guild_Create::~Guild_Create()
{
	this->manager->CancelCreate(this->tag);
}

std::shared_ptr<Guild> GuildManager::GetGuild(std::string tag)
{
	tag = util::uppercase(tag);

	std::unordered_map<std::string, std::weak_ptr<Guild>>::iterator findguild = this->cache.find(tag);

	if (findguild != this->cache.end())
	{
		return std::shared_ptr<Guild>(findguild->second);
	}
	else
	{
		Database_Result res = this->world->db.Query("SELECT `tag`, `name`, `description`, `created`, `ranks`, `bank` FROM `guilds` WHERE `tag` = '$'", tag.c_str());

		if (res.empty())
		{
			return std::shared_ptr<Guild>();
		}

		std::unordered_map<std::string, util::variant> row = res.front();
		std::shared_ptr<Guild> guild(new Guild(this));
		guild->tag = static_cast<std::string>(row["tag"]);
		guild->name = static_cast<std::string>(row["name"]);
		guild->description = util::text_word_wrap(static_cast<std::string>(row["description"]), this->world->config["GuildMaxWidth"]);
		guild->created = static_cast<int>(row["created"]);
		guild->ranks = RankUnserialize(static_cast<std::string>(row["ranks"]));
		guild->bank = static_cast<int>(row["bank"]);

		res = this->world->db.Query("SELECT `name`, `guild_rank`, `guild_rank_string` FROM `characters` WHERE `guild` = '$' ORDER BY `guild_rank` ASC, `name` ASC", tag.c_str());

		UTIL_FOREACH_REF(res, row)
		{
			guild->members.push_back(std::make_shared<Guild_Member>(row["name"], row["guild_rank"], row["guild_rank_string"]));
		}

		this->cache[guild->tag] = guild;
		this->cache[guild->name] = guild;

		return guild;
	}
}

std::shared_ptr<Guild> GuildManager::GetGuildName(std::string name)
{
	name = util::lowercase(name);

	std::unordered_map<std::string, std::weak_ptr<Guild>>::iterator findguild = this->cache.find(name);

	if (findguild != this->cache.end())
	{
		return std::shared_ptr<Guild>(findguild->second);
	}
	else
	{
		Database_Result res = this->world->db.Query("SELECT `tag`, `name`, `description`, `created`, `ranks`, `bank` FROM `guilds` WHERE `name` = '$'", name.c_str());

		if (res.empty())
		{
			return std::shared_ptr<Guild>();
		}

		std::unordered_map<std::string, util::variant> row = res.front();
		std::shared_ptr<Guild> guild(new Guild(this));
		guild->tag = static_cast<std::string>(row["tag"]);
		guild->name = static_cast<std::string>(row["name"]);
		guild->description = static_cast<std::string>(row["description"]);
		guild->created = static_cast<int>(row["created"]);
		guild->ranks = RankUnserialize(static_cast<std::string>(row["ranks"]));
		guild->bank = static_cast<int>(row["bank"]);

		res = this->world->db.Query("SELECT `name`, `guild_rank`, `guild_rank_string` FROM `characters` WHERE `guild` = '$' ORDER BY `guild_rank` ASC, `name` ASC", static_cast<std::string>(row["tag"]).c_str());

		UTIL_FOREACH_REF(res, row)
		{
			guild->members.push_back(std::make_shared<Guild_Member>(row["name"], row["guild_rank"], row["guild_rank_string"]));
		}

		this->cache[guild->tag] = guild;
		this->cache[guild->name] = guild;

		return guild;
	}
}

std::shared_ptr<Guild_Create> GuildManager::GetCreate(std::string tag)
{
	tag = util::uppercase(tag);

	std::unordered_map<std::string, std::weak_ptr<Guild_Create>>::iterator findcreate = this->create_cache.find(tag);

	if (findcreate != this->create_cache.end())
	{
		return std::shared_ptr<Guild_Create>(findcreate->second);
	}
	else
	{
		return std::shared_ptr<Guild_Create>();
	}
}

std::shared_ptr<Guild_Create> GuildManager::BeginCreate(std::string tag, std::string name, Character *leader)
{
	tag = util::uppercase(tag);
	name = util::lowercase(name);

	std::shared_ptr<Guild_Create> create(new Guild_Create(this, tag, name, leader));

	this->create_cache[tag] = create;

	return create;
}

void GuildManager::CancelCreate(std::string create)
{
	this->create_cache.erase(create);
}

std::shared_ptr<Guild> GuildManager::CreateGuild(std::shared_ptr<Guild_Create> create, std::string description)
{
	description = util::text_word_wrap(description, this->world->config["GuildMaxWidth"]);

	this->world->db.Query("INSERT INTO `guilds` (`tag`, `name`, `description`, `created`, `ranks`) VALUES ('$', '$', '$', #, '$')", create->tag.c_str(), create->name.c_str(), description.c_str(), int(std::time(0)), static_cast<std::string>(this->world->config["GuildDefaultRanks"]).c_str());

	this->create_cache.erase(create->tag);

	std::shared_ptr<Guild> guild = this->GetGuild(create->tag);

	if (guild)
	{
		UTIL_FOREACH(create->members, member)
		{
			Character *character = this->world->GetCharacterReal(member->name);

			if (character)
			{
				guild->AddMember(character, create->leader, false, (character == create->leader) ? 0 : 9);
			}
		}

		this->create_cache.erase(create->tag);
	}

	return guild;
}

void GuildManager::SaveAll()
{
	UTIL_FOREACH(this->cache, entry)
	{
		std::shared_ptr<Guild> guild(entry.second);

		if (guild)
			guild->Save();
	}
}

bool GuildManager::ValidName(std::string name)
{
	name = util::lowercase(name);

	if (name.length() < 4 || name.length() > std::size_t(int(this->world->config["GuildMaxNameLength"])))
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

bool GuildManager::ValidTag(std::string tag)
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

bool GuildManager::ValidRank(std::string rank)
{
	rank = util::lowercase(rank);

	if (rank.length() > std::size_t(int(this->world->config["GuildMaxRankLength"])))
	{
		return false;
	}

	for (std::size_t i = 0; i < rank.length(); ++i)
	{
		if ((rank[i] < 'a' || rank[i] > 'z') && rank[i] != ' ')
		{
			return false;
		}
	}

	return true;
}

bool GuildManager::ValidDescription(std::string description)
{
	description = util::lowercase(description);

	if (description.length() > std::size_t(int(this->world->config["GuildMaxDescLength"])))
	{
		return false;
	}

	for (std::size_t i = 0; i < description.length(); ++i)
	{
		char c = description[i];

		if ((c < 'a' || c > 'z') && (c < '0' || c > '9')
		 && c != ' ' && c != '@' && c != '_' && c != '-' && c != '.')
		{
			return false;
		}
	}

	return true;
}

void Guild::AddMember(Character *joined, Character *recruiter, bool alert, int rank)
{
	joined->guild = shared_from_this();
	joined->guild_rank = rank;
	joined->guild_rank_string = this->GetRank(rank);

	this->members.push_back(std::make_shared<Guild_Member>(joined->real_name, rank, joined->guild_rank_string));

	if (recruiter != joined) // Leader of new guild
	{
		PacketBuilder builder(PACKET_GUILD, PACKET_AGREE, 9 + this->name.length() + joined->guild_rank_string.length());
		builder.AddShort(recruiter->PlayerID());
		builder.AddByte(255);
		builder.AddBreakString(this->tag);
		builder.AddBreakString(this->name);
		builder.AddBreakString(joined->guild_rank_string);
		builder.AddByte(255);
		joined->Send(builder);
	}

	if (alert && this->manager->world->config["GuildAnnounce"])
	{
		std::string name = joined->real_name;

		std::string msg = manager->world->i18n.Format("guild_join", util::ucfirst(name));

		if (recruiter)
			msg += " " + manager->world->i18n.Format("guild_recruit", util::ucfirst(recruiter->real_name));

		this->Msg(0, msg);
	}
}

void Guild::DelMember(std::string kicked, Character *kicker, bool alert)
{
	kicked = util::lowercase(kicked);

	if (alert && this->manager->world->config["GuildAnnounce"])
	{
		std::string msg = manager->world->i18n.Format("guild_leave", util::ucfirst(kicked));

		if (kicker)
			msg += " " + manager->world->i18n.Format("guild_kick", util::ucfirst(kicker->real_name));

		this->Msg(0, msg);
	}

	UTIL_IFOREACH(this->members, it)
	{
		std::shared_ptr<Guild_Member> member = *it;

		if (member->name == kicked)
		{
			this->members.erase(it);
			break;
		}
	}

	World* world = this->manager->world;

	UTIL_FOREACH(world->server->clients, client)
	{
		EOClient *eoclient = static_cast<EOClient *>(client);

		if (eoclient->player)
		{
			UTIL_FOREACH(eoclient->player->characters, character)
			{
				if (character->real_name == kicked)
				{
					character->guild.reset();
					character->guild_rank = 0;
					character->guild_rank_string.clear();
					// *this may not be valid after this point

					if (character->online)
						return;
					else
						break;
				}
			}
		}
	}

	world->db.Query("UPDATE `characters` SET `guild` = NULL, `guild_rank` = NULL, `guild_rank_string` = NULL WHERE `name` = '$'", kicked.c_str());
}

void Guild::SetMemberRank(std::string name, int rank)
{
	std::shared_ptr<Guild_Member> member = this->GetMember(name);

	if (member)
	{
		std::string rank_str = this->GetRank(rank);

		member->rank = rank;
		member->rank_string = rank_str;

		World* world = this->manager->world;

		UTIL_FOREACH(world->server->clients, client)
		{
			EOClient *eoclient = static_cast<EOClient *>(client);

			if (eoclient->player)
			{
				UTIL_FOREACH(eoclient->player->characters, character)
				{
					if (character->real_name == name)
					{
						character->guild_rank = rank;
						character->guild_rank_string = rank_str;
						
						if (character->online)
							return;
						else
							break;
					}
				}
			}
		}

		world->db.Query("UPDATE `characters` SET `guild_rank` = #, `guild_rank_string` = '$' WHERE `name` = '$'",
			rank, rank_str.c_str(), name.c_str());
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
	if (gold > 0 && this->bank >= 0 && this->bank - gold >= 0)
	{
		this->needs_save = true;
		this->bank -= gold;
	}
}

void Guild::Disband(Character* disbander)
{
	std::vector<std::shared_ptr<Guild_Member>> disband_members = this->members;

	if (this->manager->world->config["GuildAnnounce"])
	{
		this->Msg(0, manager->world->i18n.Format("guild_disband", util::ucfirst(disbander->real_name)));
	}

	std::shared_ptr<Guild> guild(shared_from_this());

	UTIL_FOREACH(disband_members, member)
	{
		this->DelMember(member->name, disbander);
	}
}

std::shared_ptr<Guild_Member> Guild::GetMember(std::string name)
{
	name = util::lowercase(name);

	UTIL_FOREACH(this->members, member)
	{
		if (member->name == name)
		{
			return member;
		}
	}

	return std::shared_ptr<Guild_Member>();
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
	message = util::text_cap(message, static_cast<int>(this->manager->world->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from ? from->SourceName() : "Server") + "  "));

	std::string from_name = from ? from->SourceName() : "Server";

	PacketBuilder builder(PACKET_TALK, PACKET_REQUEST, 2 + from_name.length() + message.length());
	builder.AddBreakString(from_name);
	builder.AddBreakString(message);

	UTIL_FOREACH(this->manager->world->characters, character)
	{
		if (character->guild.get() == this)
		{
			character->AddChatLog("&", from_name, message);

			if (!echo && character == from)
			{
				continue;
			}

			character->Send(builder);
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
		std::unordered_map<std::string, std::weak_ptr<Guild>>::iterator findentry = this->manager->cache.find(tag);

		if (findentry != this->manager->cache.end())
			this->manager->cache.erase(findentry);

		std::unordered_map<std::string, std::weak_ptr<Guild>>::iterator findentry2 = this->manager->cache.find(name);

		if (findentry2 != this->manager->cache.end())
			this->manager->cache.erase(findentry2);
	}

	if (this->members.size() > 0)
	{
		this->Save();
	}
	else
	{
		this->manager->world->db.Query("UPDATE `characters` SET `guild` = NULL, `guild_rank` = NULL, `guild_rank_string` = NULL WHERE `guild` = '$'", this->tag.c_str());
		this->manager->world->db.Query("DELETE FROM `guilds` WHERE tag = '$'", this->tag.c_str());
	}
}
