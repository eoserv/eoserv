
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <csignal>

#include "arena.hpp"
#include "character.hpp"
#include "console.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "player.hpp"

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

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->guild->Msg(character, message, false);
}

// Party chat messagea
void Talk_Open(Character *character, PacketReader &reader)
{
	if (!character->party) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->party->Msg(character, message, false);
}

// Global chat message
void Talk_Msg(Character *character, PacketReader &reader)
{
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
	std::string name = reader.GetBreakString();
	std::string message = reader.GetEndString();
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));
	Character *to = character->world->GetCharacter(name);

	if (to && !to->hidden)
	{
		to->Msg(character, message);
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

			if (victim)
			{
				if (victim->admin < character->admin)
				{
					character->world->Kick(character, victim, command[0] != 's');
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

			if (victim)
			{
				if (victim->admin < character->admin)
				{
					character->world->Ban(character, victim, duration, command[0] != 's');
				}
			}
		}
		else if ((command.length() >= 1 && command.compare(0,1,"j") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["jail"]))
		 || (command.length() >= 2 && command.compare(0,2,"sj") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["sjail"])))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);
			if (victim)
			{
				if (victim->admin < character->admin)
				{
					character->world->Jail(character, victim, command[0] != 's');
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
			if (victim)
			{
				character->Warp(victim->mapid, victim->x, victim->y, WARP_ANIMATION_ADMIN);
			}
		}
		else if (command.length() >= 5 && command.compare(0,5,"warpt") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["warptome"]))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);
			if (victim)
			{
				victim->Warp(character->mapid, character->x, character->y, WARP_ANIMATION_ADMIN);
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

			character->Warp(map, x, y, WARP_ANIMATION_ADMIN);
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
		else if (command.length() >= 2 && command.compare(0,2,"in") == 0 && arguments.size() >= 1 && character->admin >= static_cast<int>(character->world->admin_config["info"]))
		{
			Character *victim = character->world->GetCharacter(arguments[0]);
			if (victim)
			{
				std::string name = util::ucfirst(victim->name);
				PacketBuilder reply(PACKET_ADMININTERACT, PACKET_TELL, 85 + name.length());
				if (victim->admin >= 4)
				{
					reply.AddString("High Game Master ");
				}
				else if (victim->admin >= 3)
				{
					reply.AddString("Game Master ");
				}
				else if (victim->admin >= 2)
				{
					reply.AddString("Guardian ");
				}
				else if (victim->admin >= 1)
				{
					reply.AddString("Light Guide ");
				}
				reply.AddString(name);
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
				reply.AddShort(victim->str);
				reply.AddShort(victim->intl);
				reply.AddShort(victim->wis);
				reply.AddShort(victim->agi);
				reply.AddShort(victim->con);
				reply.AddShort(victim->cha);
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

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->world->AdminMsg(character, message, ADMIN_GUARDIAN, false);
}

// Announcement message
void Talk_Announce(Character *character, PacketReader &reader)
{
	if (character->admin < ADMIN_GUARDIAN) return;

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
