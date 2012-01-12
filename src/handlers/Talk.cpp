
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <csignal>
#include <ctime>

#include "arena.hpp"
#include "character.hpp"
#include "console.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "player.hpp"
#include "world.hpp"

extern volatile std::sig_atomic_t eoserv_sig_abort;

static void limit_message(std::string &message, std::size_t chatlength)
{
	if (message.length() > chatlength)
	{
		message = message.substr(0, chatlength - 6) + " [...]";
	}
}

namespace Handlers
{

// Guild chat message
void Talk_Request(Character *character, PacketReader &reader)
{
	if (!character->guild) return;
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->guild->Msg(character, message, false);
}

// Party chat messagea
void Talk_Open(Character *character, PacketReader &reader)
{
	if (!character->party) return;
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->party->Msg(character, message, false);
}

// Global chat message
void Talk_Msg(Character *character, PacketReader &reader)
{
	if (character->muted_until > time(0)) return;

	if (character->mapid == static_cast<int>(character->world->config["JailMap"]))
	{
		return;
	}

	std::string message = reader.GetEndString();
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->world->Msg(character, message, false);
}

// Private chat message
void Talk_Tell(Character *character, PacketReader &reader)
{
	if (character->muted_until > time(0)) return;

	std::string name = reader.GetBreakString();
	std::string message = reader.GetEndString();
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));
	Character *to = character->world->GetCharacter(name);

	if (to && !to->hidden)
	{
		if (to->whispers)
		{
			to->Msg(character, message);
		}
		else
		{
			character->Msg(to, character->world->i18n.Format("whisper_blocked", to->name));
		}
	}
	else
	{
		PacketBuilder reply(PACKET_TALK, PACKET_REPLY, 2 + name.length());
		reply.AddShort(TALK_NOTFOUND);
		reply.AddString(name);
		character->Send(reply);
	}
}

// Public chat message
void Talk_Report(Character *character, PacketReader &reader)
{
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString();
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	if (message.empty())
	{
		return;
	}

	if (character->admin && message[0] == '$')
	{
		std::string command;
		std::vector<std::string> arguments = util::explode(' ', message);
		command = arguments.front().substr(1);
		arguments.erase(arguments.begin());

		if ((command.length() >= 1 && command.compare(0,1,"k") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["kick"]))
		 || (command.length() >= 2 && command.compare(0,2,"sk") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["skick"])))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				if (victim->admin < character->admin)
				{
					character->world->Kick(character, victim, command[0] != 's');
				}
				else
				{
					character->ServerMsg(character->world->i18n.Format("command_access_denied"));
				}
			}
		}
		else if (command.length() >= 2 && command.compare(0,2,"bo") == 0 && character->admin >= static_cast<int>(character->world->admin_config["board"]))
		{
			short boardid = ((arguments.size() >= 1) ? util::to_int(arguments[0]) : static_cast<int>(character->world->config["AdminBoard"])) - 1;

			if (boardid != static_cast<int>(character->world->config["AdminBoard"]) - 1
			 || character->admin >= static_cast<int>(character->world->admin_config["reports"]))
			{
				if (static_cast<std::size_t>(boardid) < character->world->boards.size())
				{
					character->board = character->world->boards[boardid];
					character->ShowBoard();
				}
			}
		}
		else if ((command.length() >= 1 && command.compare(0,1,"b") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["ban"]))
		 || (command.length() >= 2 && command.compare(0,2,"sb") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["sban"])))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);
			int duration;
			if (arguments.size() >= 2)
			{
				arguments[1] = util::lowercase(arguments[1]);
				if (arguments[1] == "forever")
				{
					duration = -1;
				}
				else
				{
					duration = int(util::tdparse(arguments[1]));
				}
			}
			else
			{
				duration = int(util::tdparse(character->world->config["DefaultBanLength"]));
			}

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				if (victim->admin < character->admin)
				{
					character->world->Ban(character, victim, duration, command[0] != 's');
				}
				else
				{
					character->ServerMsg(character->world->i18n.Format("command_access_denied"));
				}
			}
		}
		else if ((command.length() >= 1 && command.compare(0,1,"j") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["jail"]))
		 || (command.length() >= 2 && command.compare(0,2,"sj") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["sjail"])))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				if (victim->admin < character->admin)
				{
					character->world->Jail(character, victim, command[0] != 's');
				}
				else
				{
					character->ServerMsg(character->world->i18n.Format("command_access_denied"));
				}
			}
		}
		else if ((command.length() >= 1 && command.compare(0,1,"m") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["mute"]))
		 || (command.length() >= 2 && command.compare(0,2,"sm") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["smute"])))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				if (victim->admin < character->admin)
				{
					character->world->Mute(character, victim);
				}
				else
				{
					character->ServerMsg(character->world->i18n.Format("command_access_denied"));
				}
			}
		}
		else if (command.length() >= 2 && command.compare(0,2,"si") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["sitem"]))
		{
			int id = util::to_int(arguments[0]);
			int amount = (arguments.size() >= 2)?util::to_int(arguments[1]):1;
			if (character->AddItem(id, amount))
			{
				PacketBuilder reply(PACKET_ITEM, PACKET_GET, 9);
				reply.AddShort(0); // UID
				reply.AddShort(id);
				reply.AddThree(amount);
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);
				character->Send(reply);
			}
		}
		else if (command.length() >= 2 && command.compare(0,2,"di") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["ditem"]))
		{
			if (character->trading) return;

			int id = util::to_int(arguments[0]);
			int amount = (arguments.size() >= 2)?util::to_int(arguments[1]):1;
			int x = (arguments.size() >= 3)?util::to_int(arguments[2]):character->x;
			int y = (arguments.size() >= 4)?util::to_int(arguments[3]):character->y;
			if (amount > 0 && character->HasItem(id) >= amount)
			{
				Map_Item *item = character->map->AddItem(id, amount, x, y, character);
				if (item)
				{
					item->owner = character->player->id;
					item->unprotecttime = Timer::GetTime() + static_cast<double>(character->world->config["ProtectPlayerDrop"]);
					character->DelItem(id, amount);

					PacketBuilder reply(PACKET_ITEM, PACKET_DROP, 15);
					reply.AddShort(id);
					reply.AddThree(amount);
					reply.AddInt(character->HasItem(id));
					reply.AddShort(item->uid);
					reply.AddChar(x);
					reply.AddChar(y);
					reply.AddChar(character->weight);
					reply.AddChar(character->maxweight);
					character->Send(reply);
				}
			}
		}
		else if (command.length() >= 2 && command.compare(0,2,"sn") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["snpc"]))
		{
			int id = util::to_int(arguments[0]);
			int amount = (arguments.size() >= 2)?util::to_int(arguments[1]):1;

			if (id < 0 || static_cast<std::size_t>(id) > character->world->enf->data.size())
			{
				return;
			}

			for (int i = 0; i < amount; ++i)
			{
				unsigned char index = character->map->GenerateNPCIndex();

				if (index > 250)
				{
					break;
				}

				NPC *npc = new NPC(character->map, id, character->x, character->y, 1, 1, index, true);
				character->map->npcs.push_back(npc);
				npc->Spawn();
			}
		}
		else if (command.length() >= 5 && command.compare(0,5,"warpm") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["warpmeto"]))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				character->Warp(victim->mapid, victim->x, victim->y, character->world->config["WarpBubbles"] ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
			}
		}
		else if (command.length() >= 5 && command.compare(0,5,"warpt") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["warptome"]))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				victim->Warp(character->mapid, character->x, character->y, character->world->config["WarpBubbles"] ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
			}
		}
		else if (command.length() >= 1 && command.compare(0,1,"w") == 0 && arguments.size() >= 3 && character->admin >= static_cast<int>(character->world->admin_config["warp"]))
		{
			int map = util::to_int(arguments[0]);
			int x = util::to_int(arguments[1]);
			int y = util::to_int(arguments[2]);

			if (map < 0 || map >= static_cast<int>(character->world->maps.size()))
			{
				return;
			}

			character->Warp(map, x, y, character->world->config["WarpBubbles"] ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
		}
		else if (command.length() >= 3 && command.compare(0,3,"rem") == 0 && character->admin >= static_cast<int>(character->world->admin_config["remap"]))
		{
			if (!(character->map->Reload()))
			{
				while (!character->map->characters.empty())
				{
					character->map->characters.back()->player->client->Close();
					character->map->characters.pop_back();
				}
			}
		}
		else if (command.length() >= 3 && command.compare(0,3,"rep") == 0 && character->admin >= static_cast<int>(character->world->admin_config["repub"]))
		{
			character->world->ReloadPub();
		}
		else if (command.length() >= 1 && command.compare(0,1,"r") == 0 && character->admin >= static_cast<int>(character->world->admin_config["rehash"]))
		{
			Console::Out("Config reloaded by %s", character->name.c_str());
			character->world->Rehash();
		}
		else if (command.length() >= 1 && command.compare(0,1,"a") == 0 && character->admin >= static_cast<int>(character->world->admin_config["arena"]))
		{
			if (character->map->arena)
			{
				character->map->arena->Spawn(true);
			}
		}
		else if (command.length() >= 1 && command.compare(0,1,"l") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["learn"]))
		{
			short skill_id = util::to_int(arguments[0]);
			short level = -1;

			if (arguments.size() >= 2)
			{
				level = util::to_int(arguments[1]);

				level = std::max(0, std::min<int>(character->world->config["MaxSkillLevel"], level));
			}

			if (character->AddSpell(skill_id))
			{
				PacketBuilder builder(PACKET_STATSKILL, PACKET_TAKE, 6);
				builder.AddShort(skill_id);
				builder.AddInt(character->HasItem(1));
				character->Send(builder);
			}

			if (level >= 0)
			{
				auto it = std::find_if(UTIL_RANGE(character->spells), [&](Character_Spell spell) { return spell.id == skill_id; });

				if (it != character->spells.end())
				{
					it->level = level;

					PacketBuilder builder(PACKET_STATSKILL, PACKET_ACCEPT, 6);
					builder.AddShort(character->skillpoints);
					builder.AddShort(skill_id);
					builder.AddShort(it->level);
					character->Send(builder);
				}
			}
		}
		else if (command.length() >= 3 && command.compare(0,3,"inv") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["inventory"]))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				std::string name = util::ucfirst(victim->name);

				PacketBuilder reply(PACKET_ADMININTERACT, PACKET_LIST, 32 + name.length() + victim->inventory.size() * 6 + victim->bank.size() * 7);

				switch (victim->admin)
				{
					case ADMIN_HGM: reply.AddString(character->world->i18n.Format("high_game_master_title", name)); break;
					case ADMIN_GM: reply.AddString(character->world->i18n.Format("game_master_title", name)); break;
					case ADMIN_GUARDIAN: reply.AddString(character->world->i18n.Format("guardian_title", name)); break;
					case ADMIN_GUIDE: reply.AddString(character->world->i18n.Format("light_guide_title", name)); break;
					default: ;
				}

				reply.AddString(" ");
				reply.AddBreakString(util::trim(victim->PaddedGuildTag()));
				reply.AddInt(victim->Usage());
				reply.AddByte(255);
				reply.AddInt(victim->goldbank);
				reply.AddByte(255);

				UTIL_CFOREACH(victim->inventory, item)
				{
					reply.AddShort(item.id);
					reply.AddInt(item.amount);
				}
				reply.AddByte(255);

				UTIL_CFOREACH(victim->bank, item)
				{
					reply.AddShort(item.id);
					reply.AddThree(item.amount);
				}

				character->Send(reply);
			}
		}
		else if (command.length() >= 2 && command.compare(0,2,"in") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["info"]))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				std::string name = util::ucfirst(victim->name);

				PacketBuilder reply(PACKET_ADMININTERACT, PACKET_TELL, 85 + name.length());

				switch (victim->admin)
				{
					case ADMIN_HGM: reply.AddString(character->world->i18n.Format("high_game_master_title", name)); break;
					case ADMIN_GM: reply.AddString(character->world->i18n.Format("game_master_title", name)); break;
					case ADMIN_GUARDIAN: reply.AddString(character->world->i18n.Format("guardian_title", name)); break;
					case ADMIN_GUIDE: reply.AddString(character->world->i18n.Format("light_guide_title", name)); break;
					default: ;
				}

				reply.AddString(" ");
				reply.AddBreakString(util::trim(victim->PaddedGuildTag()));
				reply.AddInt(victim->Usage());
				reply.AddByte(255);
				reply.AddByte(255);
				reply.AddInt(victim->exp);
				reply.AddChar(victim->level);
				reply.AddShort(victim->mapid);
				reply.AddShort(victim->x);
				reply.AddShort(victim->y);
				reply.AddShort(victim->hp);
				reply.AddShort(victim->maxhp);
				reply.AddShort(victim->tp);
				reply.AddShort(victim->maxtp);
				reply.AddShort(victim->display_str);
				reply.AddShort(victim->display_intl);
				reply.AddShort(victim->display_wis);
				reply.AddShort(victim->display_agi);
				reply.AddShort(victim->display_con);
				reply.AddShort(victim->display_cha);
				reply.AddShort(victim->maxdam);
				reply.AddShort(victim->mindam);
				reply.AddShort(victim->accuracy);
				reply.AddShort(victim->evade);
				reply.AddShort(victim->armor);
				reply.AddShort(0); // light
				reply.AddShort(0); // dark
				reply.AddShort(0); // fire
				reply.AddShort(0); // water
				reply.AddShort(0); // earth
				reply.AddShort(0); // wind
				reply.AddChar(victim->weight);
				reply.AddChar(victim->maxweight);

				character->Send(reply);
			}
		}
		else if (command.length() == 8 && command.compare(0,8,"shutdown") == 0 && character->admin >= static_cast<int>(character->world->admin_config["shutdown"]))
		{
			Console::Wrn("Server shut down by %s", character->name.c_str());
			eoserv_sig_abort = true;
		}
		else if (command.length() >= 2 && command.compare(0,2,"up") == 0 && character->admin >= static_cast<int>(character->world->admin_config["uptime"]))
		{
			std::string buffer = "Server started ";
			buffer += util::timeago(character->world->server->start, Timer::GetTime());
			character->ServerMsg(buffer);
		}
		else if (command.length() >= 1 && command.compare(0,1,"q") == 0 && character->admin >= static_cast<int>(character->world->admin_config["quake"]))
		{
			int strength = (arguments.size() >= 1)?std::min(8,std::max(1,util::to_int(arguments[0]))):5;
			character->map->Effect(MAP_EFFECT_QUAKE, strength);
		}
		else if (command.length() >= 1 && command.compare(0,1,"h") == 0 && character->admin >= static_cast<int>(character->world->admin_config["hide"]))
		{
			if (character->hidden)
			{
				character->Unhide();
			}
			else
			{
				character->Hide();
			}
		}
		else if ((command == "e" || (command.length() >= 2 && command.compare(0,2,"ev") == 0)) && character->admin >= static_cast<int>(character->world->admin_config["evacuate"]))
		{
			character->map->Evacuate();
		}
		else if (command.length() >= 2 && command.compare(0,2,"st") == 0 && character->admin >= static_cast<int>(character->world->admin_config["strip"]))
		{
			Character *victim = ((arguments.size() >= 1) ? character->world->GetCharacter(arguments[0]) : character);

			if (!victim)
			{
				character->ServerMsg(character->world->i18n.Format("character_not_found"));
			}
			else
			{
				if (victim->admin < character->admin || victim == character)
				{
					for (std::size_t i = 0; i < victim->paperdoll.size(); ++i)
					{
						if (victim->paperdoll[i] != 0)
						{
							int itemid = victim->paperdoll[i];
							int subloc = ((i == Character::Ring2 || i == Character::Armlet2 || i == Character::Bracer2) ? 1 : 0);

							if (victim->Unequip(victim->paperdoll[i], subloc))
							{
								PacketBuilder builder(PACKET_PAPERDOLL, PACKET_REMOVE, 43);
								builder.AddShort(victim->player->id);
								builder.AddChar(SLOT_CLOTHES);
								builder.AddChar(0); // ?
								builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Boots])->dollgraphic);
								builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Armor])->dollgraphic);
								builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Hat])->dollgraphic);
								builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Weapon])->dollgraphic);
								builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Shield])->dollgraphic);
								builder.AddShort(itemid);
								builder.AddChar(subloc);
								builder.AddShort(victim->maxhp);
								builder.AddShort(victim->maxtp);
								builder.AddShort(victim->display_str);
								builder.AddShort(victim->display_intl);
								builder.AddShort(victim->display_wis);
								builder.AddShort(victim->display_agi);
								builder.AddShort(victim->display_con);
								builder.AddShort(victim->display_cha);
								builder.AddShort(victim->mindam);
								builder.AddShort(victim->maxdam);
								builder.AddShort(victim->accuracy);
								builder.AddShort(victim->evade);
								builder.AddShort(victim->armor);
								victim->Send(builder);
							}
						}
					}

					PacketBuilder builder(PACKET_AVATAR, PACKET_AGREE, 14);
					builder.AddShort(victim->player->id);
					builder.AddChar(SLOT_CLOTHES);
					builder.AddChar(0);
					builder.AddShort(0);
					builder.AddShort(0);
					builder.AddShort(0);
					builder.AddShort(0);
					builder.AddShort(0);

					UTIL_FOREACH(victim->map->characters, updatecharacter)
					{
						if (updatecharacter == victim || !victim->InRange(updatecharacter))
							continue;

						updatecharacter->Send(builder);
					}
				}
				else
				{
					character->ServerMsg(character->world->i18n.Format("command_access_denied"));
				}
			}
		}
		else if (command.length() >= 3 && command.compare(0,3,"set") == 0 && arguments.size() >= 1)
		{
			std::string set = command.substr(3);

			auto aconfig_it = character->world->admin_config.find("set" + set);

			if (aconfig_it == character->world->admin_config.end()
				|| (arguments.size() < 2 && (set != "title" || set != "fiance" || set != "partner" || set != "home")))
			{
				character->ServerMsg(character->world->i18n.Format("unknown_command"));
			}
			else
			{
				Character *victim = character->world->GetCharacter(arguments[0]);

				if (!victim)
				{
					character->ServerMsg(character->world->i18n.Format("character_not_found"));
				}
				else
				{
					bool appearance = false;
					bool failure = false;
					bool level = false;
					bool stats = false;
					bool karma = false;

					bool statpoints = false;
					bool skillpoints = false;

						 if (set == "level") (level = true, victim->level) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxLevel"]));
					else if (set == "exp") (level = true, victim->exp) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxEXP"]));
					else if (set == "str") (stats = true, victim->str) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxStat"]));
					else if (set == "int") (stats = true, victim->intl) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxStat"]));
					else if (set == "wis") (stats = true, victim->wis) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxStat"]));
					else if (set == "agi") (stats = true, victim->agi) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxStat"]));
					else if (set == "con") (stats = true, victim->con) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxStat"]));
					else if (set == "cha") (stats = true, victim->cha) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxStat"]));
					else if (set == "statpoints") (statpoints = true, victim->statpoints) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxLevel"]) * int(character->world->config["StatPerLevel"]));
					else if (set == "skillpoints") (skillpoints = true, victim->skillpoints) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxLevel"]) * int(character->world->config["SkillPerLevel"]));
					else if (set == "admin")
					{
						if (victim->admin < character->admin || character->admin == ADMIN_HGM)
						{
							AdminLevel level = std::min(std::max(AdminLevel(util::to_int(arguments[1])), ADMIN_PLAYER), ADMIN_HGM);

							if (level == ADMIN_PLAYER && victim->admin != ADMIN_PLAYER)
								victim->world->DecAdminCount();
							else if (level != ADMIN_PLAYER && victim->admin == ADMIN_PLAYER)
								victim->world->IncAdminCount();

							victim->admin = level;
						}
						else
						{
							character->ServerMsg(character->world->i18n.Format("command_access_denied"));
						}
					}
					else if (set == "title") victim->title = (arguments.size() > 1) ? message.substr(11 + victim->name.length()) : "";
					else if (set == "fiance") victim->fiance = (arguments.size() > 1) ? arguments[1] : "";
					else if (set == "partner") victim->partner = (arguments.size() > 1) ? arguments[1] : "";
					else if (set == "home") victim->home = (arguments.size() > 1) ? arguments[1] : "";
					else if (set == "gender") (appearance = true, victim->gender) = Gender(std::min(std::max(util::to_int(arguments[1]), 0), 1));
					else if (set == "hairstyle") (appearance = true, victim->hairstyle) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxHairStyle"]));
					else if (set == "haircolor") (appearance = true, victim->haircolor) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxHairColor"]));
					else if (set == "race") (appearance = true, victim->race) = Skin(std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->config["MaxSkin"])));
					else if (set == "guildrank") victim->guild_rank = std::min(std::max(util::to_int(arguments[1]), 0), 9);
					else if (set == "karma") (karma = true, victim->karma) = std::min(std::max(util::to_int(arguments[1]), 0), 2000);
					else if (set == "class") (stats = true, victim->clas) = std::min(std::max(util::to_int(arguments[1]), 0), int(character->world->ecf->data.size() - 1));
					else failure = true;

					if (failure)
					{
						character->ServerMsg(character->world->i18n.Format("invalid_setx"));
					}
					else
					{
						// Easiest way to get the character to update on everyone nearby's screen
						if (appearance)
							victim->Warp(victim->map->id, victim->x, victim->y);

						// TODO: Good way of updating skillpoints
						(void)skillpoints;

						if (stats || statpoints)
						{
							victim->CalculateStats();

							PacketBuilder builder(PACKET_RECOVER, PACKET_LIST, 32);

							if (statpoints)
							{
								builder.SetID(PACKET_STATSKILL, PACKET_PLAYER);
								builder.AddShort(character->statpoints);
							}
							else
							{
								builder.AddShort(victim->clas);
							}

							builder.AddShort(victim->display_str);
							builder.AddShort(victim->display_intl);
							builder.AddShort(victim->display_wis);
							builder.AddShort(victim->display_agi);
							builder.AddShort(victim->display_con);
							builder.AddShort(victim->display_cha);
							builder.AddShort(victim->maxhp);
							builder.AddShort(victim->maxtp);
							builder.AddShort(victim->maxsp);
							builder.AddShort(victim->maxweight);
							builder.AddShort(victim->mindam);
							builder.AddShort(victim->maxdam);
							builder.AddShort(victim->accuracy);
							builder.AddShort(victim->evade);
							builder.AddShort(victim->armor);
							victim->Send(builder);
						}

						if (karma || level)
						{
							PacketBuilder builder(PACKET_RECOVER, PACKET_REPLY, 7);
							builder.AddInt(victim->exp);
							builder.AddShort(victim->karma);
							builder.AddChar(level ? victim->level : 0);
							victim->Send(builder);
						}
					}
				}
			}
		}
		else
		{
			character->ServerMsg(character->world->i18n.Format("unknown_command"));
		}
	}
	else
	{
		character->map->Msg(character, message, false);
	}
}

// Admin chat message
void Talk_Admin(Character *character, PacketReader &reader)
{
	if (character->admin < ADMIN_GUARDIAN) return;
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->world->AdminMsg(character, message, ADMIN_GUARDIAN, false);
}

// Announcement message
void Talk_Announce(Character *character, PacketReader &reader)
{
	if (character->admin < ADMIN_GUARDIAN) return;
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->world->AnnounceMsg(character, message, false);
}

PACKET_HANDLER_REGISTER(PACKET_TALK)
	Register(PACKET_REQUEST, Talk_Request, Playing);
	Register(PACKET_OPEN, Talk_Open, Playing);
	Register(PACKET_MSG, Talk_Msg, Playing);
	Register(PACKET_TELL, Talk_Tell, Playing);
	Register(PACKET_REPORT, Talk_Report, Playing);
	Register(PACKET_ADMIN, Talk_Admin, Playing);
	Register(PACKET_ANNOUNCE, Talk_Announce, Playing);
PACKET_HANDLER_REGISTER_END()

}
