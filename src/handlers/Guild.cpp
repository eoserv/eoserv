/* handlers/Guild.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eodata.hpp"
#include "../guild.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <algorithm>
#include <cstddef>
#include <list>
#include <memory>
#include <string>

namespace Handlers
{

// Requested to create a guild
void Guild_Request(Character *character, PacketReader &reader)
{
	/*int session = */reader.GetInt();
	reader.GetByte();
	std::string tag = reader.GetBreakString();
	std::string name = reader.GetBreakString();

	if (tag.length() > 3 || name.length() > std::size_t(int(character->world->config["GuildMaxNameLength"])))
	{
		return;
	}

	if (character->npc_type == ENF::Guild)
	{
		if (!character->guild)
		{
			if (static_cast<int>(character->world->config["GuildCreateMembers"]) > 0)
			{
				std::shared_ptr<Guild> guild(character->world->guildmanager->GetGuild(tag));

				if (!guild)
				{
					guild = character->world->guildmanager->GetGuildName(name);
				}

				if (!guild)
				{
					if (character->world->guildmanager->ValidTag(tag) && character->world->guildmanager->ValidName(name))
					{
						if (character->HasItem(1) >= static_cast<int>(character->world->config["GuildPrice"]))
						{
							std::shared_ptr<class Guild_Create> create = character->world->guildmanager->BeginCreate(tag, name, character);
							character->guild_create = create;

							PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 1);
							reply.AddChar(GUILD_CREATE_BEGIN);
							character->Send(reply);

							PacketBuilder builder(PACKET_GUILD, PACKET_REQUEST, 8 + name.length());
							builder.AddShort(create->leader->PlayerID());
							builder.AddString(util::ucfirst(util::lowercase(name)) + " (" + util::uppercase(tag) + ")");

							UTIL_FOREACH(character->map->characters, updatecharacter)
							{
								if (updatecharacter != character && !updatecharacter->guild)
								{
									updatecharacter->guild_invite = tag;
									updatecharacter->Send(builder);
								}
							}

							if (static_cast<int>(character->world->config["GuildCreateMembers"]) == 1)
							{
								PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
								reply.AddShort(GUILD_CREATE_ADD_CONFIRM);
								reply.AddString("");
								character->Send(reply);
							}
						}
					}
					else
					{
						PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
						reply.AddShort(GUILD_NOT_APPROVED);
						character->Send(reply);
					}
				}
				else
				{
					PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
					reply.AddShort(GUILD_EXISTS);
					character->Send(reply);
				}
			}
		}
	}
}

// Accept guild create invite
void Guild_Accept(Character *character, PacketReader &reader)
{
	(void)reader;

	if (!character->guild_invite.empty())
	{
		std::shared_ptr<class Guild_Create> create = character->world->guildmanager->GetCreate(character->guild_invite);

		if (create)
		{
			create->AddMember(character->real_name);

			PacketBuilder builder(PACKET_GUILD, PACKET_REPLY, 2 + character->real_name.length());

			if (create->members.size() == static_cast<std::size_t>(static_cast<int>(character->world->config["GuildCreateMembers"])))
			{
				builder.AddShort(GUILD_CREATE_ADD_CONFIRM);
			}
			else
			{
				builder.AddShort(GUILD_CREATE_ADD);
			}

			builder.AddString(character->real_name);
			create->leader->Send(builder);
		}

		character->guild_invite = "";
	}
}

// Left the guild
void Guild_Remove(Character *character, PacketReader &reader)
{
	(void)reader;
	/*int session = reader.GetInt();*/

	if (character->npc_type == ENF::Guild)
	{
		if (character->guild)
		{
			character->guild->DelMember(character->real_name, 0, true);
		}
	}
}

// Update guild rank list
void Guild_Agree(Character *character, PacketReader &reader)
{
	/*int session = */reader.GetInt();
	GuildInfoType info = static_cast<GuildInfoType>(reader.GetShort());

	if (character->npc_type == ENF::Guild)
	{
		if (character->guild)
		{
			switch (info)
			{
				case GUILD_INFO_DESCRIPTION:
				{
					if (character->guild_rank <= static_cast<int>(character->world->config["GuildEditRank"]))
					{
						std::string description = reader.GetEndString();

						if (!character->world->guildmanager->ValidDescription(description))
							return;

						character->guild->SetDescription(description);

						PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
						reply.AddShort(GUILD_UPDATED);
						character->Send(reply);
					}
				}
				break;

				case GUILD_INFO_RANKS:
				{
					if (character->guild_rank <= static_cast<int>(character->world->config["GuildEditRank"]))
					{
						decltype(character->guild->ranks) new_ranks(character->guild->ranks);

						for (std::size_t i = 0; i < character->guild->ranks.size(); ++i)
						{
							new_ranks[i] = reader.GetBreakString();

							if (!character->world->guildmanager->ValidRank(new_ranks[i]))
								return;
						}

						for (std::size_t i = 0; i < character->guild->ranks.size(); ++i)
						{
							character->guild->ranks[i] = new_ranks[i];
						}

						character->guild->needs_save = true;

						PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
						reply.AddShort(GUILD_RANKS_UPDATED);
						character->Send(reply);
					}
				}
				break;

				default: ;
			}
		}
	}
}

// Creating a guild
void Guild_Create(Character *character, PacketReader &reader)
{
	/*int session = */reader.GetInt();
	reader.GetByte();
	std::string tag = reader.GetBreakString();
	std::string name = reader.GetBreakString();
	std::string description = reader.GetBreakString();

	if (tag.length() > 3
	 || name.length() > std::size_t(int(character->world->config["GuildMaxNameLength"]))
	 || !character->world->guildmanager->ValidDescription(description))
	{
		return;
	}

	if (character->npc_type == ENF::Guild)
	{
		if (!character->guild
		 && character->HasItem(1) >= static_cast<int>(character->world->config["GuildPrice"]))
		{
			std::shared_ptr<class Guild_Create> create = character->world->guildmanager->GetCreate(tag);

			if (create && create->leader == character
			 && create->members.size() >= static_cast<std::size_t>(static_cast<int>(character->world->config["GuildCreateMembers"])))
			{
				std::shared_ptr<Guild> guild = character->world->guildmanager->CreateGuild(create, description);

				character->DelItem(1, character->world->config["GuildPrice"]);

				std::string rank_str = character->GuildRankString();

				PacketBuilder reply(PACKET_GUILD, PACKET_CREATE, 13 + guild->name.length() + rank_str.length());
				reply.AddShort(create->leader->PlayerID());
				reply.AddByte(255);
				reply.AddBreakString(guild->tag);
				reply.AddBreakString(guild->name);
				reply.AddBreakString(rank_str);
				reply.AddInt(character->HasItem(1));
				character->Send(reply);

				character->guild_create.reset();
			}
		}
	}
}

// Requested to join a guild
void Guild_Player(Character *character, PacketReader &reader)
{
	/*int session = */reader.GetInt();
	reader.GetByte();
	std::string tag = util::uppercase(reader.GetBreakString());
	std::string recruiter_name = util::lowercase(reader.GetBreakString());

	if (tag.length() > 3 || recruiter_name.length() > 12)
	{
		return;
	}

	if (character->npc_type == ENF::Guild)
	{
		if (!character->guild)
		{
			Character *recruiter = character->world->GetCharacterReal(recruiter_name);

			if (recruiter)
			{
				if (!recruiter->nowhere && recruiter->mapid == character->mapid)
				{
					if (recruiter->guild && recruiter->guild->tag == tag)
					{
						if (recruiter->guild_rank <= static_cast<int>(character->world->config["GuildRecruitRank"]))
						{
							character->guild_join = tag;

							PacketBuilder builder(PACKET_GUILD, PACKET_REPLY, 4 + character->real_name.length());
							builder.AddShort(GUILD_JOIN_REQUEST);
							builder.AddShort(character->PlayerID());
							builder.AddString(util::ucfirst(character->real_name));
							recruiter->Send(builder);
						}
						else
						{
							PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
							reply.AddShort(GUILD_NOT_RECRUITER);
							character->Send(reply);
						}
					}
					else
					{
						PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
						reply.AddShort(GUILD_RECRUITER_WRONG_GUILD);
						character->Send(reply);
					}
				}
				else
				{
					PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
					reply.AddShort(GUILD_RECRUITER_NOT_HERE);
					character->Send(reply);
				}
			}
			else
			{
				PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
				reply.AddShort(GUILD_RECRUITER_OFFLINE);
				character->Send(reply);
			}
		}
	}
}

// Requested guild bank/rank list/description
void Guild_Take(Character *character, PacketReader &reader)
{
	/*int session = */reader.GetInt();
	GuildInfoType info = static_cast<GuildInfoType>(reader.GetShort());

	if (character->npc_type == ENF::Guild)
	{
		if (character->guild)
		{
			switch (info)
			{
				case GUILD_INFO_DESCRIPTION:
				{
					if (character->guild_rank <= static_cast<int>(character->world->config["GuildEditRank"]))
					{
						PacketBuilder reply(PACKET_GUILD, PACKET_TAKE, character->guild->description.length());
						reply.AddString(character->guild->description);

						character->Send(reply);
					}
				}
				break;

				case GUILD_INFO_RANKS:
				{
					if (character->guild_rank <= static_cast<int>(character->world->config["GuildEditRank"]))
					{
						PacketBuilder reply(PACKET_GUILD, PACKET_RANK, character->guild->ranks.size() * (1 + int(character->world->config["GuildMaxRankLength"])));

						for (std::size_t i = 0; i < character->guild->ranks.size(); ++i)
						{
							reply.AddBreakString(character->guild->ranks[i]);
						}

						character->Send(reply);
					}
				}
				break;

				case GUILD_INFO_BANK:
				{
					PacketBuilder reply(PACKET_GUILD, PACKET_SELL, 4);
					reply.AddInt(character->guild->bank);

					character->Send(reply);
				}
				break;
			}
		}
	}
}

// Accepted a join request
void Guild_Use(Character *character, PacketReader &reader)
{
	unsigned short pid = reader.GetShort();

	Character *joiner = character->world->GetCharacterPID(pid);

	if (character->guild
	 && character->guild_rank <= static_cast<int>(character->world->config["GuildRecruitRank"])
	 && joiner && !joiner->guild
	 && character->map == joiner->map
	 && joiner->guild_join == character->guild->tag)
	{
		PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);

		if (character->guild->bank >= static_cast<int>(character->world->config["RecruitCost"]))
		{
			if (character->guild->members.size() < static_cast<std::size_t>(static_cast<int>(character->world->config["GuildMaxMembers"])))
			{
				character->guild->AddMember(joiner, character, true);
				character->guild->DelBank(character->world->config["RecruitCost"]);
				reply.AddShort(GUILD_ACCEPTED);
			}
		}
		else
		{
			reply.AddShort(GUILD_ACCOUNT_LOW);
		}

		character->Send(reply);
	}
}

// Deposit gold to the guild bank
void Guild_Buy(Character *character, PacketReader &reader)
{
	if (character->trading) return;
	if (!character->CanInteractItems()) return;

	/*int session = */reader.GetInt();
	int gold = reader.GetInt();

	if (gold < 0)
		return;

	if (character->npc_type == ENF::Guild)
	{
		if (character->guild)
		{
			int max_deposit = int(character->world->config["GuildBankMax"]) - character->guild->bank;

			if (max_deposit <= 0)
				return;

			gold = std::min(gold, max_deposit);

			if (gold >= static_cast<int>(character->world->config["GuildMinDeposit"]) && character->HasItem(1) >= gold)
			{
				character->DelItem(1, gold);
				character->guild->AddBank(gold);

				PacketBuilder reply(PACKET_GUILD, PACKET_BUY, 4);
				reply.AddInt(character->HasItem(1));
				character->Send(reply);
			}
		}
	}
}

// Talked to a guild NPC
void Guild_Open(Character *character, PacketReader &reader)
{
	short id = reader.GetShort();

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (npc->index == id && npc->ENF().type == ENF::Guild)
		{
			character->npc = npc;
			character->npc_type = ENF::Guild;

			PacketBuilder reply(PACKET_GUILD, PACKET_OPEN, 3);
			reply.AddThree(0); // Session token

			character->Send(reply);

			break;
		}
	}
}

// Requested member list of a guild
void Guild_Tell(Character *character, PacketReader &reader)
{
	/*int session = */reader.GetInt();
	std::string tag = reader.GetEndString();

	if (tag.length() > std::size_t(int(character->world->config["GuildMaxNameLength"])))
	{
		return;
	}

	if (character->npc_type == ENF::Guild)
	{
		std::shared_ptr<Guild> guild;

		if (tag.length() == 2 || tag.length() == 3)
		{
			guild = character->world->guildmanager->GetGuild(tag);
		}
		else
		{
			guild = character->world->guildmanager->GetGuildName(tag);
		}

		if (!guild)
		{
			PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 1);
			reply.AddChar(GUILD_NOT_FOUND);
			character->Send(reply);
		}
		else
		{
			PacketBuilder reply(PACKET_GUILD, PACKET_TELL, 3 + guild->members.size() * 19);
			reply.AddShort(guild->members.size());
			reply.AddByte(255);

			std::for_each(UTIL_CRANGE(guild->members), [&](std::shared_ptr<Guild_Member> member)
			{
				reply.AddChar(member->rank);
				reply.AddByte(255);
				reply.AddBreakString(member->name);

				if (character->world->config["GuildCustomRanks"] && !member->rank_string.empty())
				{
					reply.AddBreakString(member->rank_string);
				}
				else
				{
					reply.AddBreakString(guild->GetRank(member->rank));
				}
			});

			character->Send(reply);
		}
	}
}

// Requested information on a guild
void Guild_Report(Character *character, PacketReader &reader)
{
	/*int session =*/ reader.GetInt();
	std::string tag = reader.GetEndString();

	if (tag.length() > std::size_t(int(character->world->config["GuildMaxNameLength"])))
	{
		return;
	}

	if (character->npc_type == ENF::Guild)
	{
		std::shared_ptr<Guild> guild;

		if (tag.length() == 2 || tag.length() == 3)
		{
			guild = character->world->guildmanager->GetGuild(tag);
		}
		else
		{
			guild = character->world->guildmanager->GetGuildName(tag);
		}

		if (!guild)
		{
			PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
			reply.AddShort(GUILD_NOT_FOUND);
			character->Send(reply);
		}
		else
		{
			int leader_rank = std::max(static_cast<int>(character->world->config["GuildEditRank"]), static_cast<int>(character->world->config["GuildKickRank"]));
			int recruiter_rank = character->world->config["GuildRecruitRank"];

			std::list<std::shared_ptr<Guild_Member>> leaders;
			std::list<std::shared_ptr<Guild_Member>> recruiters;

			UTIL_FOREACH(guild->members, member)
			{
				if (member->rank <= leader_rank)
				{
					leaders.push_back(member);
				}
			}

			if (character->world->config["GuildShowRecruiters"])
			{
				UTIL_FOREACH(guild->members, member)
				{
					if (member->rank > leader_rank && member->rank <= recruiter_rank)
					{
						recruiters.push_back(member);
					}
				}
			}

			std::string create_date;
			create_date.resize(31);

			tm *local_time = localtime(&guild->created);
			create_date = create_date.substr(0, strftime(&create_date[0], 31, static_cast<std::string>(character->world->config["GuildDateFormat"]).c_str(), local_time));

			std::string bank_str = util::to_string(guild->bank);

			PacketBuilder reply(PACKET_GUILD, PACKET_REPORT,
				21 + guild->name.length() + create_date.length() + guild->description.length() + bank_str.length()
				+ leaders.size() * 15 + recruiters.size() * 15);

			reply.AddBreakString(guild->name);
			reply.AddBreakString(guild->tag);
			reply.AddBreakString(create_date);
			reply.AddBreakString(guild->description);
			reply.AddBreakString(bank_str);

			for (std::size_t i = 0; i < guild->ranks.size(); ++i)
			{
				std::string rank = guild->ranks[i];

				while (rank.length() < 4)
				{
					rank += ' ';
				}

				reply.AddBreakString(rank);
			}

			reply.AddShort(leaders.size() + recruiters.size());
			reply.AddByte(255);

			UTIL_FOREACH(leaders, member)
			{
				reply.AddChar(1);
				reply.AddByte(255);
				reply.AddBreakString(member->name + (member->rank == 0 ? " (founder)" : ""));
			}

			UTIL_FOREACH(recruiters, member)
			{
				reply.AddChar(2);
				reply.AddByte(255);
				reply.AddBreakString(member->name);
			}

			character->Send(reply);
		}
	}
}

// Disband guild
void Guild_Junk(Character *character, PacketReader &reader)
{
	(void)reader;
	/*int session = reader.GetInt();*/

	if (character->npc_type == ENF::Guild)
	{
		if (character->guild)
		{
			if (character->guild_rank <= static_cast<int>(character->world->config["GuildDisbandRank"]))
			{
				character->guild->Disband(character);
			}
		}
	}
}

// Remove a member
void Guild_Kick(Character *character, PacketReader &reader)
{
	/*int session = */reader.GetInt();
	std::string name = reader.GetEndString();

	if (name.length() > 12)
	{
		return;
	}

	if (character->npc_type == ENF::Guild)
	{
		if (character->guild)
		{
			if (character->guild_rank <= static_cast<int>(character->world->config["GuildKickRank"]))
			{
				std::shared_ptr<Guild_Member> target = character->guild->GetMember(name);

				if (target)
				{
					if (target->rank > character->guild_rank)
					{
						character->guild->DelMember(name, character, true);
						PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
						reply.AddShort(GUILD_REMOVED);
						character->Send(reply);
					}
					else
					{
						PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
						reply.AddShort(GUILD_REMOVE_LEADER);
						character->Send(reply);
					}
				}
				else
				{
					PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
					reply.AddShort(GUILD_REMOVE_NOT_MEMBER);
					character->Send(reply);
				}
			}
		}
	}
}

// Set a member's rank
void Guild_Rank(Character *character, PacketReader &reader)
{
	/*int session = */reader.GetInt();
	int rank = reader.GetChar();
	std::string name = util::lowercase(reader.GetEndString());

	if (rank < 0 || rank > 10 || name.length() > 12)
	{
		return;
	}

	if (character->npc_type == ENF::Guild)
	{
		if (character->guild)
		{
			std::shared_ptr<Guild_Member> target = character->guild->GetMember(name);

			if (target)
			{
				if (target->rank == 0)
				{
					PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
					reply.AddShort(GUILD_RANKING_LEADER);
					character->Send(reply);
				}
				else if (rank == 0)
				{
					if (character->guild_rank == 0 && character->world->config["GuildMultipleFounders"])
					{
						character->guild->SetMemberRank(target->name, 0);
						PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
						reply.AddShort(GUILD_UPDATED);
						character->Send(reply);
					}
				}
				else
				{
					bool has_perm = false;

					if (rank <= target->rank)
					{
						has_perm = has_perm || (character->guild_rank <= int(character->world->config["GuildPromoteRank"]));
					}
					else if (rank >= target->rank)
					{
						has_perm = has_perm || (character->guild_rank <= int(character->world->config["GuildDemoteRank"]));
					}

					if (has_perm)
					{
						if (character->guild_rank == 0 || character->guild_rank < target->rank )
						{
							if (rank > character->guild_rank
							 || (character->guild_rank <= int(character->world->config["GuildPromoteSameRank"]) && character->guild_rank <= rank))
							{
								character->guild->SetMemberRank(target->name, rank);
								PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
								reply.AddShort(GUILD_UPDATED);
								character->Send(reply);
							}
						}
						else
						{
							PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
							reply.AddShort(GUILD_RANKING_LEADER);
							character->Send(reply);
						}
					}
				}
			}
			else
			{
				PacketBuilder reply(PACKET_GUILD, PACKET_REPLY, 2);
				reply.AddShort(GUILD_RANKING_NOT_MEMBER);
				character->Send(reply);
			}
		}
	}
}

PACKET_HANDLER_REGISTER(PACKET_GUILD)
	Register(PACKET_REQUEST, Guild_Request, Playing, 1.0);
	Register(PACKET_ACCEPT, Guild_Accept, Playing);
	Register(PACKET_REMOVE, Guild_Remove, Playing);
	Register(PACKET_AGREE, Guild_Agree, Playing);
	Register(PACKET_CREATE, Guild_Create, Playing, 1.0);
	Register(PACKET_PLAYER, Guild_Player, Playing, 0.5);
	Register(PACKET_TAKE, Guild_Take, Playing);
	Register(PACKET_USE, Guild_Use, Playing);
	Register(PACKET_BUY, Guild_Buy, Playing);
	Register(PACKET_OPEN, Guild_Open, Playing);
	Register(PACKET_TELL, Guild_Tell, Playing, 0.5);
	Register(PACKET_REPORT, Guild_Report, Playing, 0.5);
	Register(PACKET_JUNK, Guild_Junk, Playing);
	Register(PACKET_KICK, Guild_Kick, Playing);
	Register(PACKET_RANK, Guild_Rank, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_GUILD)

}
