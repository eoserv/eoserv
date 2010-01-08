
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "eodata.hpp"
#include "guild.hpp"
#include "map.hpp"
#include "npc.hpp"

CLIENT_F_FUNC(Guild)
{
	PacketBuilder reply;

	double now = Timer::GetTime();

	if (this->player && this->player->character)
	{
		if (int(this->player->character->last_guild_action * 2) == int(now * 2))
		{
			CLIENT_FORCE_QUEUE_ACTION(0.5)
		}

		this->player->character->last_guild_action = now;
	}

	switch (action)
	{
		case PACKET_REQUEST: // Requested to create a guild
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			reader.GetByte();
			std::string tag = reader.GetBreakString();
			std::string name = reader.GetBreakString();

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (!this->player->character->guild)
				{
					Guild *guild = this->server->world->guildmanager->GetGuild(tag);

					if (!guild)
					{
						guild = this->server->world->guildmanager->GetGuildName(name);
					}

					if (!guild)
					{
						if (Guild::ValidTag(tag) && Guild::ValidName(name))
						{
							if (this->player->character->HasItem(1) >= static_cast<int>(this->server->world->config["GuildPrice"]))
							{
								Guild_Create *create = this->server->world->guildmanager->BeginCreate(tag, name, this->player->character);

								reply.SetID(PACKET_GUILD, PACKET_REPLY);
								reply.AddChar(GUILD_CREATE_BEGIN);
								CLIENT_SEND(reply);

								PacketBuilder builder(PACKET_GUILD, PACKET_REQUEST);
								builder.AddShort(create->leader->player->id);
								builder.AddString(util::ucfirst(util::lowercase(name)) + " (" + util::uppercase(tag) + ")");

								UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
								{
									if (*character != this->player->character && !character->guild)
									{
										character->guild_invite = tag;
										character->player->client->SendBuilder(builder);
									}
								}

								create->Release();
							}
						}
						else
						{
							reply.SetID(PACKET_GUILD, PACKET_REPLY);
							reply.AddChar(GUILD_NOT_APPROVED);
							CLIENT_SEND(reply);
						}
					}
					else
					{
						reply.SetID(PACKET_GUILD, PACKET_REPLY);
						reply.AddChar(GUILD_EXISTS);
						CLIENT_SEND(reply);
						guild->Release();
					}
				}
			}
		}
		break;

		case PACKET_ACCEPT: // Accept guild create invite
		{
			if (this->state < EOClient::Playing) return false;

			if (!this->player->character->guild_invite.empty())
			{
				Guild_Create *create = this->server->world->guildmanager->GetCreate(this->player->character->guild_invite);

				if (create)
				{
					create->AddMember(this->player->character->name);

					PacketBuilder builder(PACKET_GUILD, PACKET_REPLY);

					if (create->members.size() == static_cast<std::size_t>(static_cast<int>(this->server->world->config["GuildCreateMembers"])))
					{
						builder.AddShort(GUILD_CREATE_ADD_CONFIRM);
					}
					else
					{
						builder.AddShort(GUILD_CREATE_ADD);
					}

					builder.AddString(this->player->character->name);
					create->leader->player->client->SendBuilder(builder);

					create->Release();
				}

				this->player->character->guild_invite = "";
			}
		}
		break;

		case PACKET_REMOVE: // Left the guild
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = reader.GetInt();*/

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (this->player->character->guild)
				{
					this->player->character->guild->DelMember(this->player->character->name, 0, true);
				}
			}
		}
		break;

		case PACKET_AGREE: // Update guild rank list
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			GuildInfoType info = static_cast<GuildInfoType>(reader.GetShort());

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (this->player->character->guild)
				{
					switch (info)
					{
						case GUILD_INFO_DESCRIPTION:
						{
							if (this->player->character->guild_rank <= static_cast<int>(this->server->world->config["GuildEditRank"]))
							{
								this->player->character->guild->SetDescription(reader.GetEndString());

								reply.SetID(PACKET_GUILD, PACKET_REPLY);
								reply.AddChar(GUILD_UPDATED);
								CLIENT_SEND(reply);
							}
						}
						break;

						case GUILD_INFO_RANKS:
						{
							if (this->player->character->guild_rank <= static_cast<int>(this->server->world->config["GuildEditRank"]))
							{
								for (std::size_t i = 0; i < this->player->character->guild->ranks.size(); ++i)
								{
									this->player->character->guild->ranks[i] = reader.GetBreakString();
								}

								reply.SetID(PACKET_GUILD, PACKET_REPLY);
								reply.AddChar(GUILD_RANKS_UPDATED);
								CLIENT_SEND(reply);
							}
						}
						break;

						default: ;
					}
				}
			}
		}
		break;

		case PACKET_CREATE: // Creating a guild
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			reader.GetByte();
			std::string tag = reader.GetBreakString();
			std::string name = reader.GetBreakString();
			std::string description = reader.GetBreakString();

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (!this->player->character->guild
				 && this->player->character->HasItem(1) >= static_cast<int>(this->server->world->config["GuildPrice"]))
				{
					Guild_Create *create = this->server->world->guildmanager->GetCreate(tag);

					if (create && create->leader == this->player->character
					 && create->members.size() >= static_cast<std::size_t>(static_cast<int>(this->server->world->config["GuildCreateMembers"])))
					{
						Guild *guild = this->server->world->guildmanager->CreateGuild(create, description);

						this->player->character->DelItem(1, this->server->world->config["GuildPrice"]);

						reply.SetID(PACKET_GUILD, PACKET_CREATE);
						reply.AddShort(create->leader->player->id);
						reply.AddByte(255);
						reply.AddBreakString(guild->tag);
						reply.AddBreakString(guild->name);
						reply.AddBreakString(guild->GetRank(this->player->character->guild_rank));
						reply.AddInt(this->player->character->HasItem(1));
						CLIENT_SEND(reply);

						guild->Release();
						create->Release();
					}
				}
			}
		}
		break;

		case PACKET_PLAYER: // Requested to join a guild
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			reader.GetByte();
			std::string tag = util::uppercase(reader.GetBreakString());
			std::string recruiter_name = util::lowercase(reader.GetBreakString());

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (!this->player->character->guild)
				{
					Character *recruiter = this->server->world->GetCharacter(recruiter_name);

					if (recruiter)
					{
						if (recruiter->map == this->player->character->map)
						{
							if (recruiter->guild && recruiter->guild->tag == tag)
							{
								if (recruiter->guild_rank <= static_cast<int>(this->server->world->config["GuildRecruitRank"]))
								{
									this->player->character->guild_join = tag;

									PacketBuilder builder(PACKET_GUILD, PACKET_REPLY);
									builder.AddShort(GUILD_JOIN_REQUEST);
									builder.AddShort(this->player->id);
									builder.AddString(util::ucfirst(this->player->character->name));
									recruiter->player->client->SendBuilder(builder);
								}
								else
								{
									reply.SetID(PACKET_GUILD, PACKET_REPLY);
									reply.AddChar(GUILD_NOT_RECRUITER);
									CLIENT_SEND(reply);
								}
							}
							else
							{
								reply.SetID(PACKET_GUILD, PACKET_REPLY);
								reply.AddChar(GUILD_RECRUITER_WRONG_GUILD);
								CLIENT_SEND(reply);
							}
						}
						else
						{
							reply.SetID(PACKET_GUILD, PACKET_REPLY);
							reply.AddChar(GUILD_RECRUITER_NOT_HERE);
							CLIENT_SEND(reply);
						}
					}
					else
					{
						reply.SetID(PACKET_GUILD, PACKET_REPLY);
						reply.AddChar(GUILD_RECRUITER_OFFLINE);
						CLIENT_SEND(reply);
					}
				}
			}
		}
		break;

		case PACKET_TAKE: // Requested guild bank/rank list/description
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			GuildInfoType info = static_cast<GuildInfoType>(reader.GetShort());

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (this->player->character->guild)
				{
					switch (info)
					{
						case GUILD_INFO_DESCRIPTION:
						{
							if (this->player->character->guild_rank <= static_cast<int>(this->server->world->config["GuildEditRank"]))
							{
								reply.SetID(PACKET_GUILD, PACKET_TAKE);
								reply.AddString(this->player->character->guild->description);

								CLIENT_SEND(reply);
							}
						}
						break;

						case GUILD_INFO_RANKS:
						{
							if (this->player->character->guild_rank <= static_cast<int>(this->server->world->config["GuildEditRank"]))
							{
								reply.SetID(PACKET_GUILD, PACKET_RANK);

								for (std::size_t i = 0; i < this->player->character->guild->ranks.size(); ++i)
								{
									reply.AddBreakString(this->player->character->guild->ranks[i]);
								}

								CLIENT_SEND(reply);
							}
						}
						break;

						case GUILD_INFO_BANK:
						{
							reply.SetID(PACKET_GUILD, PACKET_SELL);
							reply.AddInt(this->player->character->guild->bank);

							CLIENT_SEND(reply);
						}
						break;
					}
				}
			}
		}
		break;

		case PACKET_USE: // Accepted a join request
		{
			if (this->state < EOClient::Playing) return false;

			unsigned short pid = reader.GetShort();

			Character *character = this->server->world->GetCharacterPID(pid);

			if (this->player->character->guild
			 && this->player->character->guild_rank <= static_cast<int>(this->server->world->config["GuildRecruitRank"])
			 && character && !character->guild
			 && this->player->character->map == character->map
			 && character->guild_join == this->player->character->guild->tag)
			{
				reply.SetID(PACKET_GUILD, PACKET_REPLY);

				if (this->player->character->guild->bank >= static_cast<int>(this->server->world->config["RecruitCost"]))
				{
					if (this->player->character->guild->members.size() < static_cast<std::size_t>(static_cast<int>(this->server->world->config["RecruitCost"])))
					{
						this->player->character->guild->AddMember(character, this->player->character, true);
						this->player->character->guild->DelBank(this->server->world->config["RecruitCost"]);
						reply.AddChar(GUILD_ACCEPTED);
					}
				}
				else
				{
					reply.AddChar(GUILD_ACCOUNT_LOW);
				}

				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_BUY: // Deposit gold to the guild bank
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			int gold = reader.GetInt();

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (this->player->character->guild)
				{
					if (gold >= static_cast<int>(this->server->world->config["GuildMinDeposit"]) && this->player->character->HasItem(1) >= gold)
					{
						this->player->character->DelItem(1, gold);
						this->player->character->guild->AddBank(gold);

						reply.SetID(PACKET_GUILD, PACKET_BUY);
						reply.AddInt(this->player->character->HasItem(1));
						CLIENT_SEND(reply);
					}
				}
			}
		}
		break;

		case PACKET_OPEN: // Talked to a guild NPC
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.0)

			short id = reader.GetShort();

			UTIL_PTR_VECTOR_FOREACH(this->player->character->map->npcs, NPC, npc)
			{
				if (npc->index == id && npc->Data()->type == ENF::Guild)
				{
					this->player->character->npc = *npc;
					this->player->character->npc_type = ENF::Guild;

					reply.SetID(PACKET_GUILD, PACKET_OPEN);
					reply.AddThree(0); // Session token

					CLIENT_SEND(reply);

					break;
				}
			}
		}
		break;

		case PACKET_TELL: // Requested member list of a guild
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			std::string tag = reader.GetEndString();

			if (this->player->character->npc_type == ENF::Guild)
			{
				Guild *guild;

				if (tag.length() == 2 || tag.length() == 3)
				{
					guild = this->server->world->guildmanager->GetGuild(tag);
				}
				else
				{
					guild = this->server->world->guildmanager->GetGuildName(tag);
				}

				if (!guild)
				{
					reply.SetID(PACKET_GUILD, PACKET_REPLY);
					reply.AddChar(GUILD_NOT_FOUND);
				}
				else
				{
					reply.SetID(PACKET_GUILD, PACKET_TELL);
					reply.AddShort(guild->members.size());
					reply.AddByte(255);

					UTIL_PTR_VECTOR_FOREACH(guild->members, Guild_Member, member)
					{
						reply.AddChar(member->rank);
						reply.AddByte(255);
						reply.AddBreakString(member->name);
						reply.AddBreakString(guild->GetRank(member->rank));
					}

					guild->Release();
				}

				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_REPORT: // Requested information on a guild
		{
			if (this->state < EOClient::Playing) return false;

			/*int session =*/ reader.GetInt();
			std::string tag = reader.GetEndString();

			if (this->player->character->npc_type == ENF::Guild)
			{
				Guild *guild;

				if (tag.length() == 2 || tag.length() == 3)
				{
					guild = this->server->world->guildmanager->GetGuild(tag);
				}
				else
				{
					guild = this->server->world->guildmanager->GetGuildName(tag);
				}

				if (!guild)
				{
					reply.SetID(PACKET_GUILD, PACKET_REPLY);
					reply.AddChar(GUILD_NOT_FOUND);
				}
				else
				{
					int leader_rank = std::max(static_cast<int>(this->server->world->config["GuildEditRank"]), static_cast<int>(this->server->world->config["GuildKickRank"]));
					int recruiter_rank = this->server->world->config["GuildRecruitRank"];

					PtrList<Guild_Member> leaders;
					PtrList<Guild_Member> recruiters;

					UTIL_PTR_VECTOR_FOREACH(guild->members, Guild_Member, member)
					{
						if (member->rank <= leader_rank)
						{
							leaders.push_back(*member);
						}
					}

					if (this->server->world->config["GuildShowRecruiters"])
					{
						UTIL_PTR_VECTOR_FOREACH(guild->members, Guild_Member, member)
						{
							if (member->rank > leader_rank && member->rank <= recruiter_rank)
							{
								recruiters.push_back(*member);
							}
						}
					}

					std::string create_date;
					create_date.resize(31);

					tm *local_time = localtime(&guild->created);
					create_date = create_date.substr(0, strftime(&create_date[0], 31, static_cast<std::string>(this->server->world->config["GuildDateFormat"]).c_str(), local_time));

					reply.SetID(PACKET_GUILD, PACKET_REPORT);
					reply.AddBreakString(guild->name);
					reply.AddBreakString(guild->tag);
					reply.AddBreakString(create_date);
					reply.AddBreakString(guild->description);
					reply.AddBreakString(util::to_string(guild->bank));

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

					UTIL_PTR_LIST_FOREACH(leaders, Guild_Member, member)
					{
						reply.AddChar(1);
						reply.AddByte(255);
						reply.AddBreakString(member->name + (member->rank == 0 ? " (founder)" : ""));
					}

					UTIL_PTR_LIST_FOREACH(recruiters, Guild_Member, member)
					{
						reply.AddChar(2);
						reply.AddByte(255);
						reply.AddBreakString(member->name);
					}

					guild->Release();
				}

				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_JUNK: // Disband guild
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = reader.GetInt();*/

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (this->player->character->guild)
				{
					// TODO: Guild disbanding
				}
			}
		}
		break;

		case PACKET_KICK: // Remove a member
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			std::string name = reader.GetEndString();

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (this->player->character->guild)
				{
					if (this->player->character->guild_rank <= static_cast<int>(this->server->world->config["GuildKickRank"]))
					{
						Guild_Member *target = this->player->character->guild->GetMember(name);

						if (target)
						{
							if (target->rank > this->player->character->guild_rank)
							{
								this->player->character->guild->DelMember(name, this->player->character, true);
								reply.SetID(PACKET_GUILD, PACKET_REPLY);
								reply.AddChar(GUILD_REMOVED);
								CLIENT_SEND(reply);
							}
							else
							{
								reply.SetID(PACKET_GUILD, PACKET_REPLY);
								reply.AddChar(GUILD_REMOVE_LEADER);
								CLIENT_SEND(reply);
							}
						}
						else
						{
							reply.SetID(PACKET_GUILD, PACKET_REPLY);
							reply.AddChar(GUILD_REMOVE_NOT_MEMBER);
							CLIENT_SEND(reply);
						}
					}
				}
			}
		}
		break;

		case PACKET_RANK: // Set a member's rank
		{
			if (this->state < EOClient::Playing) return false;

			/*int session = */reader.GetInt();
			int rank = reader.GetChar();
			std::string name = util::lowercase(reader.GetEndString());

			if (rank < 0 || rank > 10)
			{
				return false;
			}

			if (this->player->character->npc_type == ENF::Guild)
			{
				if (this->player->character->guild)
				{
					Guild_Member *target = this->player->character->guild->GetMember(name);

					if (target)
					{
						if (target->rank == 0)
						{
							reply.SetID(PACKET_GUILD, PACKET_REPLY);
							reply.AddChar(GUILD_RANKING_LEADER);
							CLIENT_SEND(reply);
						}
						else if (rank == 0)
						{
							if (this->player->character->guild_rank == 0 && this->server->world->config["GuildMultipleFounders"])
							{
								this->player->character->guild->SetMemberRank(target->name, 0);
								reply.SetID(PACKET_GUILD, PACKET_REPLY);
								reply.AddChar(GUILD_UPDATED);
								CLIENT_SEND(reply);
							}
						}
						else
						{
							bool has_perm = false;

							if (rank <= target->rank)
							{
								has_perm = has_perm || (this->player->character->guild_rank <= static_cast<int>(this->server->world->config["GuildPromoteRank"]));
							}

							if (rank >= target->rank)
							{
								has_perm = has_perm || (this->player->character->guild_rank <= static_cast<int>(this->server->world->config["GuildDemoteRank"]));
							}

							if (has_perm)
							{
								if (this->player->character->guild_rank == 0 || this->player->character->guild_rank < target->rank)
								{
									if (rank > this->player->character->guild_rank)
									{
										this->player->character->guild->SetMemberRank(target->name, rank);
										reply.SetID(PACKET_GUILD, PACKET_REPLY);
										reply.AddChar(GUILD_UPDATED);
										CLIENT_SEND(reply);
									}
								}
								else
								{
									reply.SetID(PACKET_GUILD, PACKET_REPLY);
									reply.AddChar(GUILD_RANKING_LEADER);
									CLIENT_SEND(reply);
								}
							}
						}
					}
					else
					{
						reply.SetID(PACKET_GUILD, PACKET_REPLY);
						reply.AddChar(GUILD_RANKING_NOT_MEMBER);
						CLIENT_SEND(reply);
					}
				}
			}
		}
		break;

		default:
			return false;
	}

	return true;
}

