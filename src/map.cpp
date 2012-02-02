
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "map.hpp"

#include <algorithm>
#include <set>

#include "console.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "util/rpn.hpp"

#include "arena.hpp"
#include "character.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "eoserver.hpp"
#include "npc.hpp"
#include "packet.hpp"
#include "party.hpp"
#include "player.hpp"
#include "quest.hpp"
#include "world.hpp"

static const char *safe_fail_filename;

static void safe_fail(int line)
{
	Console::Err("Invalid file / failed read/seek: %s -- %i", safe_fail_filename, line);
}

#define SAFE_SEEK(fh, offset, from) if (std::fseek(fh, offset, from) != 0) { std::fclose(fh); safe_fail(__LINE__); return false; }
#define SAFE_READ(buf, size, count, fh) if (std::fread(buf, size, count, fh) != static_cast<int>(count)) {  std::fclose(fh); safe_fail(__LINE__);return false; }

void map_spawn_chests(void *map_void)
{
	Map *map(static_cast<Map *>(map_void));

	double current_time = Timer::GetTime();
	UTIL_FOREACH(map->chests, chest)
	{
		bool needs_update = false;

		for (int slot = 1; slot <= chest->slots; ++slot)
		{
			std::list<Map_Chest_Spawn *> spawns;

			UTIL_FOREACH(chest->spawns, spawn)
			{
				if (spawn->last_taken + spawn->time*60.0 < current_time)
				{
					bool slot_used = false;

					UTIL_FOREACH(chest->items, item)
					{
						if (item->slot == spawn->slot)
						{
							slot_used = true;
						}
					}

					if (!slot_used)
					{
						spawns.push_back(spawn);
					}
				}
			}

			if (!spawns.empty())
			{
				std::list<Map_Chest_Spawn *>::iterator it = spawns.begin();
				int r = util::rand(0, spawns.size()-1);

				for (int i = 0; i < r; ++i)
				{
					++it;
				}

				Map_Chest_Spawn *spawn(*it);

				chest->AddItem(spawn->item->id, spawn->item->amount, spawn->slot);
				needs_update = true;

#ifdef DEBUG
				Console::Dbg("Spawning chest item %i (x%i) on map %i", spawn->item->id, spawn->item->amount, map->id);
#endif // DEBUG
			}
		}

		if (needs_update)
		{
			chest->Update(map, 0);
		}
	}
}

struct map_close_door_struct
{
	Map *map;
	unsigned char x, y;
};

void map_close_door(void *map_close_void)
{
	map_close_door_struct *close(static_cast<map_close_door_struct *>(map_close_void));

	close->map->CloseDoor(close->x, close->y);
}

struct map_evacuate_struct
{
	Map *map;
	int step;
};

void map_evacuate(void *map_evacuate_void)
{
	map_evacuate_struct *evac(static_cast<map_evacuate_struct *>(map_evacuate_void));

	int ticks_per_step = int(evac->map->world->config["EvacuateStep"]) / int(evac->map->world->config["EvacuateTick"]);

	if (evac->step > 0)
	{
		bool step = evac->step % ticks_per_step == 0;

		UTIL_FOREACH(evac->map->characters, character)
		{
			if (step)
				character->ServerMsg(character->world->i18n.Format("map_evacuate", (evac->step / ticks_per_step) * int(evac->map->world->config["EvacuateStep"])));

			character->PlaySound(int(evac->map->world->config["EvacuateSound"]));
		}

		--evac->step;
	}
	else
	{
		restart_loop:
		UTIL_FOREACH(evac->map->characters, character)
		{
			if (character->admin < ADMIN_GUIDE)
			{
				character->world->Jail(0, character, false);
				goto restart_loop;
			}
		}

		evac->map->evacuate_lock = false;
	}
}

int Map_Chest::HasItem(short item)
{
	UTIL_IFOREACH(this->items, it)
	{
		if ((*it)->id == item)
		{
			return (*it)->amount;
		}
	}

	return 0;
}

int Map_Chest::AddItem(short item, int amount, int slot)
{
	if (amount <= 0)
	{
		return 0;
	}

	if (slot == 0)
	{
		UTIL_FOREACH(this->items, it)
		{
			if (it->id == item)
			{
				if (it->amount + amount < 0 || it->amount + amount > this->maxchest)
				{
					return 0;
				}

				it->amount += amount;
				return amount;
			}
		}
	}

	if (this->items.size() >= static_cast<std::size_t>(this->chestslots) || amount > this->maxchest)
	{
		return 0;
	}

	if (slot == 0)
	{
		int user_items = 0;

		UTIL_FOREACH(this->items, item)
		{
			if (item->slot == 0)
			{
				++user_items;
			}
		}

		if (user_items + this->slots >= this->chestslots)
		{
			return 0;
		}
	}

	Map_Chest_Item *chestitem(new Map_Chest_Item);
	chestitem->id = item;
	chestitem->amount = amount;
	chestitem->slot = slot;

	if (slot == 0)
	{
		this->items.push_back(chestitem);
	}
	else
	{
		this->items.push_front(chestitem);
	}

	return amount;
}

int Map_Chest::DelItem(short item)
{
	UTIL_IFOREACH(this->items, it)
	{
		if ((*it)->id == item)
		{
			int amount = (*it)->amount;

			if ((*it)->slot)
			{
				double current_time = Timer::GetTime();

				UTIL_FOREACH(this->spawns, spawn)
				{
					if (spawn->slot == (*it)->slot)
					{
						spawn->last_taken = current_time;
					}
				}
			}

			this->items.erase(it);
			return amount;
		}
	}

	return 0;
}

void Map_Chest::Update(Map *map, Character *exclude)
{
	PacketBuilder builder(PACKET_CHEST, PACKET_AGREE, this->items.size() * 5);

	UTIL_FOREACH(this->items, item)
	{
		builder.AddShort(item->id);
		builder.AddThree(item->amount);
	}

	UTIL_FOREACH(map->characters, character)
	{
		if (character == exclude)
		{
			continue;
		}

		if (util::path_length(character->x, character->y, this->x, this->y) <= 1)
		{
			character->Send(builder);
		}
	}
}

Map::Map(int id, World *world)
{
	this->id = id;
	this->world = world;
	this->exists = false;
	this->jukebox_protect = 0.0;
	this->arena = 0;
	this->evacuate_lock = false;

	this->LoadArena();

	this->Load();

	if (!this->chests.empty())
	{
		TimeEvent *event = new TimeEvent(map_spawn_chests, this, 60.0, Timer::FOREVER);
		this->world->timer.Register(event);
	}
}

void Map::LoadArena()
{
	std::list<Character *> update_characters;

	if (this->arena)
	{
		UTIL_FOREACH(this->arena->map->characters, character)
		{
			if (character->arena == this->arena)
			{
				update_characters.push_back(character);
			}
		}

		delete this->arena;
	}

	if (world->arenas_config[util::to_string(id) + ".enabled"])
	{
		std::string spawns_str = world->arenas_config[util::to_string(id) + ".spawns"];

		std::vector<std::string> spawns = util::explode(',', spawns_str);

		if (spawns.size() % 4 != 0)
		{
			Console::Wrn("Invalid arena spawn data for map %i", id);
			this->arena = 0;
		}
		else
		{
			this->arena = new Arena(this, static_cast<int>(world->arenas_config[util::to_string(id) + ".time"]), static_cast<int>(world->arenas_config[util::to_string(id) + ".block"]));

			int i = 1;
			Arena_Spawn *s;
			UTIL_FOREACH(spawns, spawn)
			{
				util::trim(spawn);

				switch (i++ % 4)
				{
					case 1:
						s = new Arena_Spawn;
						s->sx = util::to_int(spawn);
						break;

					case 2:
						s->sy = util::to_int(spawn);
						break;

					case 3:
						s->dx = util::to_int(spawn);
						break;

					case 0:
						s->dy = util::to_int(spawn);
						this->arena->spawns.push_back(s);
						break;

				}
			}
		}
	}
	else
	{
		this->arena = 0;
	}

	UTIL_FOREACH(update_characters, character)
	{
		character->arena = this->arena;

		if (!this->arena)
		{
			character->Warp(character->map->id, character->map->relog_x, character->map->relog_y);
		}
	}
}

bool Map::Load()
{
	char namebuf[6];

	if (this->id < 0)
	{
		return false;
	}

	std::string filename = this->world->config["MapDir"];
	std::sprintf(namebuf, "%05i", this->id);
	filename.append(namebuf);
	filename.append(".emf");

	safe_fail_filename = filename.c_str();

	std::FILE *fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		return false;
	}

	SAFE_SEEK(fh, 0x03, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);

	char buf[12];
	int outersize;
	int innersize;

	SAFE_SEEK(fh, 0x1F, SEEK_SET);
	SAFE_READ(buf, sizeof(char), 1, fh);
	this->pk = PacketProcessor::Number(buf[0]) == 3;

	SAFE_SEEK(fh, 0x25, SEEK_SET);
	SAFE_READ(buf, sizeof(char), 2, fh);
	this->width = PacketProcessor::Number(buf[0]) + 1;
	this->height = PacketProcessor::Number(buf[1]) + 1;

	this->tiles.resize(height);
	for (int i = 0; i < height; ++i)
	{
		this->tiles[i].resize(width);
		for (int ii = 0; ii < width; ++ii)
		{
			this->tiles[i].at(ii) = new Map_Tile;
		}
	}

	SAFE_SEEK(fh, 0x2A, SEEK_SET);
	SAFE_READ(buf, sizeof(char), 3, fh);
	this->scroll = PacketProcessor::Number(buf[0]);
	this->relog_x = PacketProcessor::Number(buf[1]);
	this->relog_y = PacketProcessor::Number(buf[2]);

	SAFE_SEEK(fh, 0x2E, SEEK_SET);
	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = PacketProcessor::Number(buf[0]);
	if (outersize)
	{
		SAFE_SEEK(fh, 8 * outersize, SEEK_CUR);
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = PacketProcessor::Number(buf[0]);
	if (outersize)
	{
		SAFE_SEEK(fh, 4 * outersize, SEEK_CUR);
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = PacketProcessor::Number(buf[0]);
	if (outersize)
	{
		SAFE_SEEK(fh, 12 * outersize, SEEK_CUR);
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = PacketProcessor::Number(buf[0]);
	for (int i = 0; i < outersize; ++i)
	{
		SAFE_READ(buf, sizeof(char), 2, fh);
		unsigned char yloc = PacketProcessor::Number(buf[0]);
		innersize = PacketProcessor::Number(buf[1]);
		for (int ii = 0; ii < innersize; ++ii)
		{
			Map_Tile *newtile(new Map_Tile);
			SAFE_READ(buf, sizeof(char), 2, fh);
			unsigned char xloc = PacketProcessor::Number(buf[0]);
			unsigned char spec = PacketProcessor::Number(buf[1]);
			newtile->tilespec = static_cast<Map_Tile::TileSpec>(spec);

			if (spec == Map_Tile::Chest)
			{
				Map_Chest *chest = new Map_Chest;
				chest->maxchest = static_cast<int>(this->world->config["MaxChest"]);
				chest->chestslots = static_cast<int>(this->world->config["ChestSlots"]);
				chest->x = xloc;
				chest->y = yloc;
				chest->slots = 0;
				this->chests.push_back(chest);
			}

			try
			{
				this->tiles.at(yloc).at(xloc) = newtile;
			}
			catch (...)
			{
				std::fclose(fh);
				safe_fail(__LINE__);
				return false;
			}
		}
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = PacketProcessor::Number(buf[0]);
	for (int i = 0; i < outersize; ++i)
	{
		SAFE_READ(buf, sizeof(char), 2, fh);
		unsigned char yloc = PacketProcessor::Number(buf[0]);
		innersize = PacketProcessor::Number(buf[1]);
		for (int ii = 0; ii < innersize; ++ii)
		{
			Map_Warp *newwarp(new Map_Warp);
			SAFE_READ(buf, sizeof(char), 8, fh);
			unsigned char xloc = PacketProcessor::Number(buf[0]);
			newwarp->map = PacketProcessor::Number(buf[1], buf[2]);
			newwarp->x = PacketProcessor::Number(buf[3]);
			newwarp->y = PacketProcessor::Number(buf[4]);
			newwarp->levelreq = PacketProcessor::Number(buf[5]);
			newwarp->spec = static_cast<Map_Warp::WarpSpec>(PacketProcessor::Number(buf[6], buf[7]));

			try
			{
				this->tiles.at(yloc).at(xloc)->warp = newwarp;
			}
			catch (...)
			{
				std::fclose(fh);
				safe_fail(__LINE__);
				return false;
			}
		}
	}

	SAFE_SEEK(fh, 0x2E, SEEK_SET);
	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = PacketProcessor::Number(buf[0]);
	int index = 0;
	for (int i = 0; i < outersize; ++i)
	{
		SAFE_READ(buf, sizeof(char), 8, fh);
		unsigned char x = PacketProcessor::Number(buf[0]);
		unsigned char y = PacketProcessor::Number(buf[1]);
		short npc_id = PacketProcessor::Number(buf[2], buf[3]);
		unsigned char spawntype = PacketProcessor::Number(buf[4]);
		short spawntime = PacketProcessor::Number(buf[5], buf[6]);
		unsigned char amount = PacketProcessor::Number(buf[7]);

		if (npc_id != this->world->enf->Get(npc_id)->id)
		{
			Console::Wrn("An NPC spawn on map %i uses a non-existent NPC (#%i at %ix%i)", this->id, npc_id, x, y);
		}

		for (int ii = 0; ii < amount; ++ii)
		{
			if (x > this->width || y > this->height)
			{
				Console::Wrn("An NPC spawn on map %i is outside of map bounds (%s at %ix%i)", this->id, this->world->enf->Get(npc_id)->name.c_str(), x, y);
				continue;
			}

			NPC *newnpc = new NPC(this, npc_id, x, y, spawntype, spawntime, index++);
			this->npcs.push_back(newnpc);

			newnpc->Spawn();
		}
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = PacketProcessor::Number(buf[0]);
	if (outersize)
	{
		SAFE_SEEK(fh, 4 * outersize, SEEK_CUR);
	}

	SAFE_READ(buf, sizeof(char), 1, fh);
	outersize = PacketProcessor::Number(buf[0]);
	for (int i = 0; i < outersize; ++i)
	{
		SAFE_READ(buf, sizeof(char), 12, fh);
		unsigned char x = PacketProcessor::Number(buf[0]);
		unsigned char y = PacketProcessor::Number(buf[1]);
		short slot = PacketProcessor::Number(buf[4]);
		short itemid = PacketProcessor::Number(buf[5], buf[6]);
		short time = PacketProcessor::Number(buf[7], buf[8]);
		int amount = PacketProcessor::Number(buf[9], buf[10], buf[11]);

		if (itemid != this->world->eif->Get(itemid)->id)
		{
			Console::Wrn("A chest spawn on map %i uses a non-existent item (#%i at %ix%i)", this->id, itemid, x, y);
		}

		UTIL_FOREACH(this->chests, chest)
		{
			if (chest->x == x && chest->y == y)
			{
				Map_Chest_Item *item(new Map_Chest_Item);
				Map_Chest_Spawn *spawn(new Map_Chest_Spawn);

				item->id = itemid;
				item->amount = amount;

				spawn->slot = slot+1;
				spawn->time = time;
				spawn->last_taken = Timer::GetTime();
				spawn->item = item;

				chest->spawns.push_back(spawn);
				chest->slots = std::max(chest->slots, slot+1);
				goto skip_warning;
			}
		}
		Console::Wrn("A chest spawn on map %i points to a non-chest (%s x%i at %ix%i)", this->id, this->world->eif->Get(itemid)->name.c_str(), amount, x, y);
		skip_warning:
		;
	}

	SAFE_SEEK(fh, 0x00, SEEK_END);
	this->filesize = std::ftell(fh);

	std::fclose(fh);

	this->exists = true;

	return true;
}

void Map::Unload()
{
	this->exists = false;

	this->chests.clear();

	UTIL_FOREACH(this->npcs, npc)
	{
		UTIL_FOREACH(npc->damagelist, opponent)
		{
			opponent->attacker->unregister_npc.erase(
				std::remove(UTIL_RANGE(opponent->attacker->unregister_npc), npc),
				opponent->attacker->unregister_npc.end()
			);
		}
	}

	this->npcs.clear();
}

int Map::GenerateItemID()
{
	int lowest_free_id = 1;
	restart_loop:
	UTIL_FOREACH(this->items, item)
	{
		if (item->uid == lowest_free_id)
		{
			lowest_free_id = item->uid + 1;
			goto restart_loop;
		}
	}
	return lowest_free_id;
}

unsigned char Map::GenerateNPCIndex()
{
	unsigned char lowest_free_id = 1;
	restart_loop:
	UTIL_FOREACH(this->npcs, npc)
	{
		if (npc->index == lowest_free_id)
		{
			lowest_free_id = npc->index + 1;
			goto restart_loop;
		}
	}
	return lowest_free_id;
}

void Map::Enter(Character *character, WarpAnimation animation)
{
	this->characters.push_back(character);
	character->map = this;
	character->last_walk = Timer::GetTime();
	character->attacks = 0;
	character->CancelSpell();

	PacketBuilder builder(PACKET_PLAYERS, PACKET_AGREE, 63);

	builder.AddByte(255);
	builder.AddBreakString(character->name);
	builder.AddShort(character->player->id);
	builder.AddShort(character->mapid);
	builder.AddShort(character->x);
	builder.AddShort(character->y);
	builder.AddChar(character->direction);
	builder.AddChar(6); // ?
	builder.AddString(character->PaddedGuildTag());
	builder.AddChar(character->level);
	builder.AddChar(character->gender);
	builder.AddChar(character->hairstyle);
	builder.AddChar(character->haircolor);
	builder.AddChar(character->race);
	builder.AddShort(character->maxhp);
	builder.AddShort(character->hp);
	builder.AddShort(character->maxtp);
	builder.AddShort(character->tp);
	// equipment
	builder.AddShort(this->world->eif->Get(character->paperdoll[Character::Boots])->dollgraphic);
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(this->world->eif->Get(character->paperdoll[Character::Armor])->dollgraphic);
	builder.AddShort(0); // ??
	builder.AddShort(this->world->eif->Get(character->paperdoll[Character::Hat])->dollgraphic);

	EIF_Data* wep = this->world->eif->Get(character->paperdoll[Character::Weapon]);

	if (wep->subtype == EIF::TwoHanded && wep->dual_wield_dollgraphic)
		builder.AddShort(wep->dual_wield_dollgraphic);
	else
		builder.AddShort(this->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);

	builder.AddShort(wep->dollgraphic);

	builder.AddChar(character->sitting);
	builder.AddChar(character->hidden);
	builder.AddChar(animation);
	builder.AddByte(255);
	builder.AddChar(1); // 0 = NPC, 1 = player

	UTIL_FOREACH(this->characters, checkcharacter)
	{
		if (checkcharacter == character || !character->InRange(checkcharacter))
		{
			continue;
		}

		checkcharacter->Send(builder);
	}

	character->CheckQuestRules();
}

void Map::Leave(Character *character, WarpAnimation animation, bool silent)
{
	if (!silent)
	{
		PacketBuilder builder(PACKET_AVATAR, PACKET_REMOVE, 3);
		builder.AddShort(character->player->id);

		if (animation != WARP_ANIMATION_NONE)
		{
			builder.AddChar(animation);
		}

		UTIL_FOREACH(this->characters, checkcharacter)
		{
			if (checkcharacter == character || !character->InRange(checkcharacter))
			{
				continue;
			}

			checkcharacter->Send(builder);
		}
	}

	this->characters.erase(
		std::remove(UTIL_RANGE(this->characters), character),
		this->characters.end()
	);

	character->map = 0;
}

void Map::Msg(Character *from, std::string message, bool echo)
{
	message = util::text_cap(message, static_cast<int>(this->world->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from->name) + "  "));

	PacketBuilder builder(PACKET_TALK, PACKET_PLAYER, 2 + message.length());
	builder.AddShort(from->player->id);
	builder.AddString(message);

	UTIL_FOREACH(this->characters, character)
	{
		if ((!echo && character == from) || !from->InRange(character))
		{
			continue;
		}

		character->Send(builder);
	}
}

void Map::Msg(NPC *from, std::string message)
{
	message = util::text_cap(message, static_cast<int>(this->world->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from->Data()->name) + "  "));

	PacketBuilder builder(PACKET_NPC, PACKET_PLAYER, 4 + message.length());
	builder.AddByte(255);
	builder.AddByte(255);
	builder.AddShort(from->index);
	builder.AddChar(message.length());
	builder.AddString(message);

	UTIL_FOREACH(this->characters, character)
	{
		character->Send(builder);
	}
}

bool Map::Walk(Character *from, Direction direction, bool admin)
{
	int seedistance = this->world->config["SeeDistance"];

	unsigned char target_x = from->x;
	unsigned char target_y = from->y;

	switch (direction)
	{
		case DIRECTION_UP:
			target_y -= 1;

			if (target_y > from->y)
			{
				return false;
			}

			break;

		case DIRECTION_RIGHT:
			target_x += 1;

			if (target_x < from->x)
			{
				return false;
			}

			break;

		case DIRECTION_DOWN:
			target_y += 1;

			if (target_x < from->x)
			{
				return false;
			}

			break;

		case DIRECTION_LEFT:
			target_x -= 1;

			if (target_x > from->x)
			{
				return false;
			}

			break;
	}

	if (!admin && !this->Walkable(target_x, target_y))
	{
		return false;
	}

	Map_Warp *warp = this->GetWarp(target_x, target_y);

	if (!admin && warp)
	{
		if (from->level >= warp->levelreq && (warp->spec == Map_Warp::NoDoor || warp->open))
		{
			Map* map = this->world->GetMap(warp->map);
			if (from->admin < ADMIN_GUIDE && map->evacuate_lock && map->id != from->map->id)
			{
				from->StatusMsg(this->world->i18n.Format("map_evacuate_block"));
				from->Refresh();
			}
			else
			{
				from->Warp(warp->map, warp->x, warp->y);
			}
		}

		return false;
	}

    from->last_walk = Timer::GetTime();
    from->attacks = 0;
    from->CancelSpell();

	from->direction = direction;

	from->x = target_x;
	from->y = target_y;

	int newx;
	int newy;
	int oldx;
	int oldy;

	std::vector<std::pair<int, int>> newcoords;
	std::vector<std::pair<int, int>> oldcoords;

	std::vector<Character *> newchars;
	std::vector<Character *> oldchars;
	std::vector<NPC *> newnpcs;
	std::vector<NPC *> oldnpcs;
	std::vector<Map_Item *> newitems;

	switch (direction)
	{
		case DIRECTION_UP:
			for (int i = -seedistance; i <= seedistance; ++i)
			{
				newy = from->y - seedistance + std::abs(i);
				newx = from->x + i;
				oldy = from->y + seedistance + 1 - std::abs(i);
				oldx = from->x + i;

				newcoords.push_back(std::make_pair(newx, newy));
				oldcoords.push_back(std::make_pair(oldx, oldy));
			}
			break;

		case DIRECTION_RIGHT:
			for (int i = -seedistance; i <= seedistance; ++i)
			{
				newx = from->x + seedistance - std::abs(i);
				newy = from->y + i;
				oldx = from->x - seedistance - 1 + std::abs(i);
				oldy = from->y + i;

				newcoords.push_back(std::make_pair(newx, newy));
				oldcoords.push_back(std::make_pair(oldx, oldy));
			}
			break;

		case DIRECTION_DOWN:
			for (int i = -seedistance; i <= seedistance; ++i)
			{
				newy = from->y + seedistance - std::abs(i);
				newx = from->x + i;
				oldy = from->y - seedistance - 1 + std::abs(i);
				oldx = from->x + i;

				newcoords.push_back(std::make_pair(newx, newy));
				oldcoords.push_back(std::make_pair(oldx, oldy));
			}
			break;

		case DIRECTION_LEFT:
			for (int i = -seedistance; i <= seedistance; ++i)
			{
				newx = from->x - seedistance + std::abs(i);
				newy = from->y + i;
				oldx = from->x + seedistance + 1 - std::abs(i);
				oldy = from->y + i;

				newcoords.push_back(std::make_pair(newx, newy));
				oldcoords.push_back(std::make_pair(oldx, oldy));
			}
			break;

	}

	UTIL_FOREACH(this->characters, checkchar)
	{
		if (checkchar == from)
		{
			continue;
		}

		for (std::size_t i = 0; i < oldcoords.size(); ++i)
		{
			if (checkchar->x == oldcoords[i].first && checkchar->y == oldcoords[i].second)
			{
				oldchars.push_back(checkchar);
			}
			else if (checkchar->x == newcoords[i].first && checkchar->y == newcoords[i].second)
			{
				newchars.push_back(checkchar);
			}
		}
	}

	UTIL_FOREACH(this->npcs, checknpc)
	{
		if (!checknpc->alive)
		{
			continue;
		}

		for (std::size_t i = 0; i < oldcoords.size(); ++i)
		{
			if (checknpc->x == oldcoords[i].first && checknpc->y == oldcoords[i].second)
			{
				oldnpcs.push_back(checknpc);
			}
			else if (checknpc->x == newcoords[i].first && checknpc->y == newcoords[i].second)
			{
				newnpcs.push_back(checknpc);
			}
		}
	}

	UTIL_FOREACH(this->items, checkitem)
	{
		for (std::size_t i = 0; i < oldcoords.size(); ++i)
		{
			if (checkitem->x == newcoords[i].first && checkitem->y == newcoords[i].second)
			{
				newitems.push_back(checkitem);
			}
		}
	}

	PacketBuilder builder(PACKET_AVATAR, PACKET_REMOVE, 2);
	builder.AddShort(from->player->id);

	UTIL_FOREACH(oldchars, character)
	{
		PacketBuilder rbuilder(PACKET_AVATAR, PACKET_REMOVE, 2);
		rbuilder.AddShort(character->player->id);

		character->Send(builder);
		from->player->Send(rbuilder);
	}

	builder.Reset(62);
	builder.SetID(PACKET_PLAYERS, PACKET_AGREE);

	builder.AddByte(255);
	builder.AddBreakString(from->name);
	builder.AddShort(from->player->id);
	builder.AddShort(from->mapid);
	builder.AddShort(from->x);
	builder.AddShort(from->y);
	builder.AddChar(from->direction);
	builder.AddChar(6); // ?
	builder.AddString(from->PaddedGuildTag());
	builder.AddChar(from->level);
	builder.AddChar(from->gender);
	builder.AddChar(from->hairstyle);
	builder.AddChar(from->haircolor);
	builder.AddChar(from->race);
	builder.AddShort(from->maxhp);
	builder.AddShort(from->hp);
	builder.AddShort(from->maxtp);
	builder.AddShort(from->tp);
	// equipment
	builder.AddShort(this->world->eif->Get(from->paperdoll[Character::Boots])->dollgraphic);
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(this->world->eif->Get(from->paperdoll[Character::Armor])->dollgraphic);
	builder.AddShort(0); // ??
	builder.AddShort(this->world->eif->Get(from->paperdoll[Character::Hat])->dollgraphic);
	builder.AddShort(this->world->eif->Get(from->paperdoll[Character::Shield])->dollgraphic);
	builder.AddShort(this->world->eif->Get(from->paperdoll[Character::Weapon])->dollgraphic);
	builder.AddChar(from->sitting);
	builder.AddChar(from->hidden);
	builder.AddByte(255);
	builder.AddChar(1); // 0 = NPC, 1 = player

	UTIL_FOREACH(newchars, character)
	{
		PacketBuilder rbuilder(PACKET_PLAYERS, PACKET_AGREE, 62);
		rbuilder.AddByte(255);
		rbuilder.AddBreakString(character->name);
		rbuilder.AddShort(character->player->id);
		rbuilder.AddShort(character->mapid);
		rbuilder.AddShort(character->x);
		rbuilder.AddShort(character->y);
		rbuilder.AddChar(character->direction);
		rbuilder.AddChar(6); // ?
		rbuilder.AddString(character->PaddedGuildTag());
		rbuilder.AddChar(character->level);
		rbuilder.AddChar(character->gender);
		rbuilder.AddChar(character->hairstyle);
		rbuilder.AddChar(character->haircolor);
		rbuilder.AddChar(character->race);
		rbuilder.AddShort(character->maxhp);
		rbuilder.AddShort(character->hp);
		rbuilder.AddShort(character->maxtp);
		rbuilder.AddShort(character->tp);
		// equipment
		rbuilder.AddShort(this->world->eif->Get(character->paperdoll[Character::Boots])->dollgraphic);
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(this->world->eif->Get(character->paperdoll[Character::Armor])->dollgraphic);
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(this->world->eif->Get(character->paperdoll[Character::Hat])->dollgraphic);

		EIF_Data* wep = this->world->eif->Get(character->paperdoll[Character::Weapon]);

		if (wep->subtype == EIF::TwoHanded && wep->dual_wield_dollgraphic)
			rbuilder.AddShort(wep->dual_wield_dollgraphic);
		else
			rbuilder.AddShort(this->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);

		rbuilder.AddShort(wep->dollgraphic);

		rbuilder.AddChar(character->sitting);
		rbuilder.AddChar(character->hidden);
		rbuilder.AddByte(255);
		rbuilder.AddChar(1); // 0 = NPC, 1 = player

		character->Send(builder);
		from->player->Send(rbuilder);
	}

	builder.Reset(5);
	builder.SetID(PACKET_WALK, PACKET_PLAYER);

	builder.AddShort(from->player->id);
	builder.AddChar(direction);
	builder.AddChar(from->x);
	builder.AddChar(from->y);

	UTIL_FOREACH(this->characters, character)
	{
		if (character == from || !from->InRange(character))
		{
			continue;
		}

		character->Send(builder);
	}

	builder.Reset(2 + newitems.size() * 9);
	builder.SetID(PACKET_WALK, PACKET_REPLY);

	builder.AddByte(255);
	builder.AddByte(255);
	UTIL_FOREACH(newitems, item)
	{
		builder.AddShort(item->uid);
		builder.AddShort(item->id);
		builder.AddChar(item->x);
		builder.AddChar(item->y);
		builder.AddThree(item->amount);
	}
	from->player->Send(builder);

	builder.SetID(PACKET_APPEAR, PACKET_REPLY);
	UTIL_FOREACH(newnpcs, npc)
	{
		builder.Reset(8);
		builder.AddChar(0);
		builder.AddByte(255);
		builder.AddChar(npc->index);
		builder.AddShort(npc->id);
		builder.AddChar(npc->x);
		builder.AddChar(npc->y);
		builder.AddChar(npc->direction);

		from->player->Send(builder);
	}

	UTIL_FOREACH(oldnpcs, npc)
	{
		npc->RemoveFromView(from);
	}

	from->CheckQuestRules();

	return true;
}

bool Map::Walk(NPC *from, Direction direction)
{
	int seedistance = this->world->config["SeeDistance"];

	unsigned char target_x = from->x;
	unsigned char target_y = from->y;

	switch (direction)
	{
		case DIRECTION_UP:
			target_y -= 1;

			if (target_y > from->y)
			{
				return false;
			}

			break;

		case DIRECTION_RIGHT:
			target_x += 1;

			if (target_x < from->x)
			{
				return false;
			}

			break;

		case DIRECTION_DOWN:
			target_y += 1;

			if (target_x < from->x)
			{
				return false;
			}

			break;

		case DIRECTION_LEFT:
			target_x -= 1;

			if (target_x > from->x)
			{
				return false;
			}

			break;
	}

	if (!this->Walkable(target_x, target_y, true) || this->Occupied(target_x, target_y, Map::PlayerAndNPC))
	{
		return false;
	}

	from->x = target_x;
	from->y = target_y;

	int newx;
	int newy;
	int oldx;
	int oldy;

	std::vector<std::pair<int, int>> newcoords;
	std::vector<std::pair<int, int>> oldcoords;

	std::vector<Character *> newchars;
	std::vector<Character *> oldchars;

	switch (direction)
	{
		case DIRECTION_UP:
			for (int i = -seedistance; i <= seedistance; ++i)
			{
				newy = from->y - seedistance + std::abs(i);
				newx = from->x + i;
				oldy = from->y + seedistance + 1 - std::abs(i);
				oldx = from->x + i;

				newcoords.push_back(std::make_pair(newx, newy));
				oldcoords.push_back(std::make_pair(oldx, oldy));
			}
			break;

		case DIRECTION_RIGHT:
			for (int i = -seedistance; i <= seedistance; ++i)
			{
				newx = from->x + seedistance - std::abs(i);
				newy = from->y + i;
				oldx = from->x - seedistance - 1 + std::abs(i);
				oldy = from->y + i;

				newcoords.push_back(std::make_pair(newx, newy));
				oldcoords.push_back(std::make_pair(oldx, oldy));
			}
			break;

		case DIRECTION_DOWN:
			for (int i = -seedistance; i <= seedistance; ++i)
			{
				newy = from->y + seedistance - std::abs(i);
				newx = from->x + i;
				oldy = from->y - seedistance - 1 + std::abs(i);
				oldx = from->x + i;

				newcoords.push_back(std::make_pair(newx, newy));
				oldcoords.push_back(std::make_pair(oldx, oldy));
			}
			break;

		case DIRECTION_LEFT:
			for (int i = -seedistance; i <= seedistance; ++i)
			{
				newx = from->x - seedistance + std::abs(i);
				newy = from->y + i;
				oldx = from->x + seedistance + 1 - std::abs(i);
				oldy = from->y + i;

				newcoords.push_back(std::make_pair(newx, newy));
				oldcoords.push_back(std::make_pair(oldx, oldy));
			}
			break;

	}

	from->direction = direction;

	UTIL_FOREACH(this->characters, checkchar)
	{
		for (std::size_t i = 0; i < oldcoords.size(); ++i)
		{
			if (checkchar->x == oldcoords[i].first && checkchar->y == oldcoords[i].second)
			{
				oldchars.push_back(checkchar);
			}
			else if (checkchar->x == newcoords[i].first && checkchar->y == newcoords[i].second)
			{
				newchars.push_back(checkchar);
			}
		}
	}

	PacketBuilder builder(PACKET_APPEAR, PACKET_REPLY, 8);
	builder.AddChar(0);
	builder.AddByte(255);
	builder.AddChar(from->index);
	builder.AddShort(from->id);
	builder.AddChar(from->x);
	builder.AddChar(from->y);
	builder.AddChar(from->direction);

	UTIL_FOREACH(newchars, character)
	{
		character->Send(builder);
	}

	builder.Reset(7);
	builder.SetID(PACKET_NPC, PACKET_PLAYER);

	builder.AddChar(from->index);
	builder.AddChar(from->x);
	builder.AddChar(from->y);
	builder.AddChar(from->direction);
	builder.AddByte(255);
	builder.AddByte(255);
	builder.AddByte(255);

	UTIL_FOREACH(this->characters, character)
	{
		if (!character->InRange(from))
		{
			continue;
		}

		character->Send(builder);
	}

	UTIL_FOREACH(oldchars, character)
	{
		from->RemoveFromView(character);
	}

	return true;
}

void Map::Attack(Character *from, Direction direction)
{
	from->direction = direction;
	from->attacks += 1;

	from->CancelSpell();

	if (from->arena)
	{
		from->arena->Attack(from, direction);
	}

	if (this->pk || (this->world->config["GlobalPK"] && !this->world->PKExcept(this->id)))
	{
		if (this->AttackPK(from, direction))
		{
			return;
		}
	}

	PacketBuilder builder(PACKET_ATTACK, PACKET_PLAYER, 3);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);

	UTIL_FOREACH(this->characters, character)
	{
		if (character == from || !from->InRange(character))
		{
			continue;
		}

		character->Send(builder);
	}

	int target_x = from->x;
	int target_y = from->y;

	int range = 1;

	if (this->world->eif->Get(from->paperdoll[Character::Weapon])->subtype == EIF::Ranged)
	{
		range = static_cast<int>(this->world->config["RangedDistance"]);
	}

	for (int i = 0; i < range; ++i)
	{
		switch (from->direction)
		{
			case DIRECTION_UP:
				target_y -= 1;
				break;

			case DIRECTION_RIGHT:
				target_x += 1;
				break;

			case DIRECTION_DOWN:
				target_y += 1;
				break;

			case DIRECTION_LEFT:
				target_x -= 1;
				break;
		}

		UTIL_FOREACH(this->npcs, npc)
		{
			if ((npc->Data()->type == ENF::Passive || npc->Data()->type == ENF::Aggressive || from->admin > static_cast<int>(this->world->admin_config["killnpcs"]))
			 && npc->alive && npc->x == target_x && npc->y == target_y)
			{
				int amount = util::rand(from->mindam, from->maxdam);
				double rand = util::rand(0.0, 1.0);
				// Checks if target is facing you
				bool critical = std::abs(int(npc->direction) - from->direction) != 2 || rand < static_cast<double>(this->world->config["CriticalRate"]);

				std::unordered_map<std::string, double> formula_vars;

				from->FormulaVars(formula_vars);
				npc->FormulaVars(formula_vars, "target_");
				formula_vars["modifier"] = this->world->config["MobRate"];
				formula_vars["damage"] = amount;
				formula_vars["critical"] = critical;

				amount = rpn_eval(rpn_parse(this->world->formulas_config["damage"]), formula_vars);
				double hit_rate = rpn_eval(rpn_parse(this->world->formulas_config["hit_rate"]), formula_vars);

				if (rand > hit_rate)
				{
					amount = 0;
				}

				amount = std::max(amount, 0);

				int limitamount = std::min(amount, int(npc->hp));

				if (this->world->config["LimitDamage"])
				{
					amount = limitamount;
				}

				npc->Damage(from, amount);

				return;
			}
		}

		if (!this->Walkable(target_x, target_y, true))
		{
			return;
		}
	}
}

bool Map::AttackPK(Character *from, Direction direction)
{
	(void)direction;

	int target_x = from->x;
	int target_y = from->y;

	int range = 1;

	if (this->world->eif->Get(from->paperdoll[Character::Weapon])->subtype == EIF::Ranged)
	{
		range = static_cast<int>(this->world->config["RangedDistance"]);
	}

	for (int i = 0; i < range; ++i)
	{
		switch (from->direction)
		{
			case DIRECTION_UP:
				target_y -= 1;
				break;

			case DIRECTION_RIGHT:
				target_x += 1;
				break;

			case DIRECTION_DOWN:
				target_y += 1;
				break;

			case DIRECTION_LEFT:
				target_x -= 1;
				break;
		}

		UTIL_FOREACH(this->characters, character)
		{
			if (character->mapid == this->id && !character->nowhere && character->x == target_x && character->y == target_y)
			{
				int amount = util::rand(from->mindam, from->maxdam);
				double rand = util::rand(0.0, 1.0);
				// Checks if target is facing you
				bool critical = std::abs(int(character->direction) - from->direction) != 2 || rand < static_cast<double>(this->world->config["CriticalRate"]);

				std::unordered_map<std::string, double> formula_vars;

				from->FormulaVars(formula_vars);
				character->FormulaVars(formula_vars, "target_");
				formula_vars["modifier"] = this->world->config["PKRate"];
				formula_vars["damage"] = amount;
				formula_vars["critical"] = critical;

				amount = rpn_eval(rpn_parse(this->world->formulas_config["damage"]), formula_vars);
				double hit_rate = rpn_eval(rpn_parse(this->world->formulas_config["hit_rate"]), formula_vars);

				if (rand > hit_rate)
				{
					amount = 0;
				}

				amount = std::max(amount, 0);

				int limitamount = std::min(amount, int(character->hp));

				if (this->world->config["LimitDamage"])
				{
					amount = limitamount;
				}

				character->hp -= limitamount;

				PacketBuilder from_builder(PACKET_AVATAR, PACKET_REPLY, 10);
				from_builder.AddShort(0);
				from_builder.AddShort(character->player->id);
				from_builder.AddThree(amount);
				from_builder.AddChar(from->direction);
				from_builder.AddChar(int(double(character->hp) / double(character->maxhp) * 100.0));
				from_builder.AddChar(character->hp == 0);

				PacketBuilder builder(PACKET_AVATAR, PACKET_REPLY, 10);
				builder.AddShort(from->player->id);
				builder.AddShort(character->player->id);
				builder.AddThree(amount);
				builder.AddChar(from->direction);
				builder.AddChar(int(double(character->hp) / double(character->maxhp) * 100.0));
				builder.AddChar(character->hp == 0);

				from->Send(from_builder);

				UTIL_FOREACH(this->characters, checkchar)
				{
					if (from != checkchar && character->InRange(checkchar))
					{
						checkchar->Send(builder);
					}
				}

				if (character->hp == 0)
				{
					character->hp = int(character->maxhp * static_cast<double>(this->world->config["DeathRecover"]) / 100.0);

					if (this->world->config["Deadly"])
					{
						character->DropAll(from);
					}

					character->map->Leave(character, WARP_ANIMATION_NONE, true);
					character->nowhere = true;
					character->map = this->world->GetMap(character->SpawnMap());
					character->mapid = character->SpawnMap();
					character->x = character->SpawnX();
					character->y = character->SpawnY();

					character->player->client->queue.AddAction(PacketReader(std::array<char, 2>{
						{char(PACKET_INTERNAL_NULL), char(PACKET_INTERNAL)}
					}.data()), 1.5);

					character->player->client->queue.AddAction(PacketReader(std::array<char, 2>{
						{char(PACKET_INTERNAL_WARP), char(PACKET_INTERNAL)}
					}.data()), 0.0);

					UTIL_FOREACH(from->quests, q) { q.second->KilledPlayer(); }
				}

				builder.Reset(4);
				builder.SetID(PACKET_RECOVER, PACKET_PLAYER);

				builder.AddShort(character->hp);
				builder.AddShort(character->tp);
				character->Send(builder);

				return true;
			}
		}

		if (!this->Walkable(target_x, target_y, true))
		{
			return false;
		}
	}

	return false;
}

void Map::Face(Character *from, Direction direction)
{
	from->direction = direction;

	from->CancelSpell();

	PacketBuilder builder(PACKET_FACE, PACKET_PLAYER, 3);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);

	UTIL_FOREACH(this->characters, character)
	{
		if (character == from || !from->InRange(character))
		{
			continue;
		}

		character->Send(builder);
	}
}

void Map::Sit(Character *from, SitState sit_type)
{
	from->sitting = sit_type;

	from->CancelSpell();

	PacketBuilder builder((sit_type == SIT_CHAIR) ? PACKET_CHAIR : PACKET_SIT, PACKET_PLAYER, 6);
	builder.AddShort(from->player->id);
	builder.AddChar(from->x);
	builder.AddChar(from->y);
	builder.AddChar(from->direction);
	builder.AddChar(0); // ?

	UTIL_FOREACH(this->characters, character)
	{
		if (character == from || !from->InRange(character))
		{
			continue;
		}

		character->Send(builder);
	}
}

void Map::Stand(Character *from)
{
	from->sitting = SIT_STAND;

	from->CancelSpell();

	PacketBuilder builder(PACKET_SIT, PACKET_REMOVE, 4);
	builder.AddShort(from->player->id);
	builder.AddChar(from->x);
	builder.AddChar(from->y);

	UTIL_FOREACH(this->characters, character)
	{
		if (character == from || !from->InRange(character))
		{
			continue;
		}

		character->Send(builder);
	}
}

void Map::Emote(Character *from, enum Emote emote, bool echo)
{
	PacketBuilder builder(PACKET_EMOTE, PACKET_PLAYER, 3);
	builder.AddShort(from->player->id);
	builder.AddChar(emote);

	UTIL_FOREACH(this->characters, character)
	{
		if (!echo && (character == from || !from->InRange(character)))
		{
			continue;
		}

		character->Send(builder);
	}
}

bool Map::Occupied(unsigned char x, unsigned char y, Map::OccupiedTarget target)
{
	if (x >= this->width || y >= this->height)
	{
		return false;
	}

	if (target != Map::NPCOnly)
	{
		UTIL_FOREACH(this->characters, character)
		{
			if (character->x == x && character->y == y)
			{
				return true;
			}
		}
	}

	if (target != Map::PlayerOnly)
	{
		UTIL_FOREACH(this->npcs, npc)
		{
			if (npc->alive && npc->x == x && npc->y == y)
			{
				return true;
			}
		}
	}

	return false;
}

Map::~Map()
{
	this->Unload();
}

bool Map::OpenDoor(Character *from, unsigned char x, unsigned char y)
{
	if (from && !from->InRange(x, y))
	{
		return false;
	}

	if (Map_Warp *warp = this->GetWarp(x, y))
	{
		if (warp->spec == Map_Warp::NoDoor || warp->open)
		{
			return false;
		}

		if (from && warp->spec > Map_Warp::Door)
		{
			if (!from->HasItem(this->world->eif->GetKey(warp->spec - static_cast<int>(Map_Warp::Door) + 1)))
			{
				return false;
			}
		}

		PacketBuilder builder(PACKET_DOOR, PACKET_OPEN, 3);
		builder.AddChar(x);
		builder.AddShort(y);

		UTIL_FOREACH(this->characters, character)
		{
			if (character->InRange(x, y))
			{
				character->Send(builder);
			}
		}

		warp->open = true;

		map_close_door_struct *close = new map_close_door_struct;
		close->map = this;
		close->x = x;
		close->y = y;

		TimeEvent *event = new TimeEvent(map_close_door, close, this->world->config["DoorTimer"], 1);
		this->world->timer.Register(event);

		return true;
	}

	return false;
}

void Map::CloseDoor(unsigned char x, unsigned char y)
{
	if (Map_Warp *warp = this->GetWarp(x, y))
	{
		if (warp->spec == Map_Warp::NoDoor || !warp->open)
		{
			return;
		}

		warp->open = false;
	}
}

ESF_Data *map_spell_common(Character *from, unsigned short spell_id)
{
	ESF_Data *spell = from->world->esf->Get(spell_id);

	if (spell->id == 0)
		return 0;

	if (from->tp < spell->tp)
		return 0;

	return spell;
}

void Map::SpellSelf(Character *from, unsigned short spell_id)
{
	ESF_Data *spell = map_spell_common(from, spell_id);

	if (!spell || spell->type != ESF::Heal)
		return;

	from->tp -= spell->tp;

	int hpgain = spell->hp;

	if (this->world->config["LimitDamage"])
		hpgain = std::min(hpgain, from->maxhp - from->hp);

	hpgain = std::max(hpgain, 0);

	from->hp += hpgain;

	if (this->world->config["LimitDamage"])
		from->hp = std::min(from->hp, from->maxhp);

	PacketBuilder builder(PACKET_SPELL, PACKET_TARGET_SELF, 15);
	builder.AddShort(from->player->id);
	builder.AddShort(spell_id);
	builder.AddInt(spell->hp);
	builder.AddChar(int(double(from->hp) / double(from->maxhp) * 100.0));

	UTIL_FOREACH(this->characters, character)
	{
		if (character != from && from->InRange(character))
			character->Send(builder);
	}

	builder.AddShort(from->hp);
	builder.AddShort(from->tp);
	builder.AddShort(1);

	from->Send(builder);
}

void Map::SpellAttack(Character *from, NPC *npc, unsigned short spell_id)
{
	ESF_Data *spell = map_spell_common(from, spell_id);

	if (!spell || spell->type != ESF::Damage)
		return;

	if ((npc->Data()->type == ENF::Passive || npc->Data()->type == ENF::Aggressive) && npc->alive)
	{
		int amount = util::rand(from->mindam, from->maxdam);
		double rand = util::rand(0.0, 1.0);

		bool critical = rand < static_cast<double>(this->world->config["CriticalRate"]);

		std::unordered_map<std::string, double> formula_vars;

		from->FormulaVars(formula_vars);
		npc->FormulaVars(formula_vars, "target_");
		formula_vars["modifier"] = this->world->config["MobRate"];
		formula_vars["damage"] = amount;
		formula_vars["critical"] = critical;

		amount = rpn_eval(rpn_parse(this->world->formulas_config["damage"]), formula_vars);
		double hit_rate = rpn_eval(rpn_parse(this->world->formulas_config["hit_rate"]), formula_vars);

		if (rand > hit_rate)
		{
			amount = 0;
		}

		amount = std::max(amount, 0);

		int limitamount = std::min(amount, int(npc->hp));

		if (this->world->config["LimitDamage"])
		{
			amount = limitamount;
		}

		npc->Damage(from, amount, spell_id);
	}
}

void Map::SpellAttackPK(Character *from, Character *victim, unsigned short spell_id)
{
	ESF_Data *spell = map_spell_common(from, spell_id);

	if (!spell || (spell->type != ESF::Heal && spell->type != ESF::Damage))
		return;

	if (spell->type == ESF::Damage && from->map->pk)
	{
		from->tp -= spell->tp;

		int amount = util::rand(from->mindam, from->maxdam);
		double rand = util::rand(0.0, 1.0);

		bool critical = rand < static_cast<double>(this->world->config["CriticalRate"]);

		std::unordered_map<std::string, double> formula_vars;

		from->FormulaVars(formula_vars);
		victim->FormulaVars(formula_vars, "target_");
		formula_vars["modifier"] = this->world->config["PKRate"];
		formula_vars["damage"] = amount;
		formula_vars["critical"] = critical;

		amount = rpn_eval(rpn_parse(this->world->formulas_config["damage"]), formula_vars);
		double hit_rate = rpn_eval(rpn_parse(this->world->formulas_config["hit_rate"]), formula_vars);

		if (rand > hit_rate)
		{
			amount = 0;
		}

		amount = std::max(amount, 0);

		int limitamount = std::min(amount, int(victim->hp));

		if (this->world->config["LimitDamage"])
		{
			amount = limitamount;
		}

		victim->hp -= limitamount;

		PacketBuilder builder(PACKET_AVATAR, PACKET_ADMIN, 12);
		builder.AddShort(from->player->id);
		builder.AddShort(victim->player->id);
		builder.AddThree(amount);
		builder.AddChar(from->direction);
		builder.AddChar(int(double(victim->hp) / double(victim->maxhp) * 100.0));
		builder.AddChar(victim->hp == 0);
		builder.AddShort(spell_id);

		UTIL_FOREACH(this->characters, character)
		{
			if (victim->InRange(character))
				character->Send(builder);
		}

		if (victim->hp == 0)
		{
			victim->hp = int(victim->maxhp * static_cast<double>(this->world->config["DeathRecover"]) / 100.0);

			if (this->world->config["Deadly"])
			{
				victim->DropAll(from);
			}

			victim->map->Leave(victim, WARP_ANIMATION_NONE, true);
			victim->nowhere = true;
			victim->map = this->world->GetMap(victim->SpawnMap());
			victim->mapid = victim->SpawnMap();
			victim->x = victim->SpawnX();
			victim->y = victim->SpawnY();

			victim->player->client->queue.AddAction(PacketReader(std::array<char, 2>{
				{char(PACKET_INTERNAL_NULL), char(PACKET_INTERNAL)}
			}.data()), 1.5);

			victim->player->client->queue.AddAction(PacketReader(std::array<char, 2>{
				{char(PACKET_INTERNAL_WARP), char(PACKET_INTERNAL)}
			}.data()), 0.0);
		}

		builder.Reset(4);
		builder.SetID(PACKET_RECOVER, PACKET_PLAYER);

		builder.AddShort(victim->hp);
		builder.AddShort(victim->tp);
		victim->Send(builder);

		if (victim->party)
		{
			victim->party->UpdateHP(victim);
		}
	}
	else if (spell->type == ESF::Heal)
	{
		from->tp -= spell->tp;

		int hpgain = spell->hp;

		if (this->world->config["LimitDamage"])
			hpgain = std::min(hpgain, victim->maxhp - victim->hp);

		hpgain = std::max(hpgain, 0);

		victim->hp += hpgain;

		if (!this->world->config["LimitDamage"])
			victim->hp = std::min(victim->hp, victim->maxhp);

		PacketBuilder builder(PACKET_SPELL, PACKET_TARGET_OTHER, 18);
		builder.AddShort(victim->player->id);
		builder.AddShort(from->player->id);
		builder.AddChar(from->direction);
		builder.AddShort(spell_id);
		builder.AddInt(spell->hp);
		builder.AddChar(int(double(victim->hp) / double(victim->maxhp) * 100.0));

		UTIL_FOREACH(this->characters, character)
		{
			if (character != victim && victim->InRange(character))
				character->Send(builder);
		}

		builder.AddShort(victim->hp);

		victim->Send(builder);

		if (victim->party)
		{
			victim->party->UpdateHP(victim);
		}
	}

	PacketBuilder builder(PACKET_RECOVER, PACKET_PLAYER, 4);
	builder.AddShort(from->hp);
	builder.AddShort(from->tp);
	from->Send(builder);
}

void Map::SpellGroup(Character *from, unsigned short spell_id)
{
	ESF_Data *spell = map_spell_common(from, spell_id);

	if (!spell || spell->type != ESF::Heal || !from->party)
		return;

	from->tp -= spell->tp;

	int hpgain = spell->hp;

	if (this->world->config["LimitDamage"])
		hpgain = std::min(hpgain, from->maxhp - from->hp);

	hpgain = std::max(hpgain, 0);

	from->hp += hpgain;

	if (this->world->config["LimitDamage"])
		from->hp = std::min(from->hp, from->maxhp);

	std::set<Character *> in_range;

	PacketBuilder builder(PACKET_SPELL, PACKET_TARGET_GROUP, 8 + from->party->members.size() * 10);
	builder.AddShort(spell_id);
	builder.AddShort(from->player->id);
	builder.AddShort(from->tp);
	builder.AddShort(spell->hp);

	UTIL_FOREACH(from->party->members, member)
	{
		if (member->map != from->map)
			continue;

		int hpgain = spell->hp;

		if (this->world->config["LimitDamage"])
			hpgain = std::min(hpgain, member->maxhp - member->hp);

		hpgain = std::max(hpgain, 0);

		member->hp += hpgain;

		if (this->world->config["LimitDamage"])
			member->hp = std::min(member->hp, member->maxhp);

		// wat?
		builder.AddByte(255);
		builder.AddByte(255);
		builder.AddByte(255);
		builder.AddByte(255);
		builder.AddByte(255);

		builder.AddShort(member->player->id);
		builder.AddChar(int(double(member->hp) / double(member->maxhp) * 100.0));
		builder.AddShort(member->hp);

		UTIL_FOREACH(this->characters, character)
		{
			if (member->InRange(character))
				in_range.insert(character);
		}
	}

	UTIL_FOREACH(in_range, character)
	{
		character->Send(builder);
	}
}

Map_Item *Map::AddItem(short id, int amount, unsigned char x, unsigned char y, Character *from)
{
	Map_Item *newitem(new Map_Item(GenerateItemID(), id, amount, x, y, 0, 0));

	PacketBuilder builder(PACKET_ITEM, PACKET_ADD, 9);
	builder.AddShort(id);
	builder.AddShort(newitem->uid);
	builder.AddThree(amount);
	builder.AddChar(x);
	builder.AddChar(y);

	if (from || (from && from->admin <= ADMIN_GM))
	{
		int ontile = 0;
		int onmap = 0;

		UTIL_FOREACH(this->items, item)
		{
			++onmap;
			if (item->x == x && item->y == y)
			{
				++ontile;
			}
		}

		if (ontile >= static_cast<int>(this->world->config["MaxTile"]) || onmap >= static_cast<int>(this->world->config["MaxMap"]))
		{
			return 0;
		}
	}

	UTIL_FOREACH(this->characters, character)
	{
		if ((from && character == from) || !character->InRange(newitem))
		{
			continue;
		}
		character->Send(builder);
	}

	this->items.push_back(newitem);
	return this->items.back();
}

Map_Item *Map::GetItem(short uid)
{
	UTIL_FOREACH(this->items, item)
	{
		if (item->uid == uid)
		{
			return item;
		}
	}

	return 0;
}

void Map::DelItem(short uid, Character *from)
{
	UTIL_IFOREACH(this->items, it)
	{
		if ((*it)->uid == uid)
		{
			PacketBuilder builder(PACKET_ITEM, PACKET_REMOVE, 2);
			builder.AddShort(uid);
			UTIL_FOREACH(this->characters, character)
			{
				if ((from && character == from) || !character->InRange(*it))
				{
					continue;
				}
				character->Send(builder);
			}
			this->items.erase(it);
			break;
		}
	}
}

void Map::DelItem(Map_Item *item, Character *from)
{
	UTIL_IFOREACH(this->items, it)
	{
		if (item == *it)
		{
			PacketBuilder builder(PACKET_ITEM, PACKET_REMOVE, 2);
			builder.AddShort((*it)->uid);
			UTIL_FOREACH(this->characters, character)
			{
				if ((from && character == from) || !character->InRange(*it))
				{
					continue;
				}
				character->Send(builder);
			}
			this->items.erase(it);
			break;
		}
	}
}

bool Map::InBounds(unsigned char x, unsigned char y)
{
	return !(x >= this->width || y >= this->height);
}

bool Map::Walkable(unsigned char x, unsigned char y, bool npc)
{
	return (InBounds(x, y) && this->tiles[y].at(x)->Walkable(npc));
}

Map_Tile::TileSpec Map::GetSpec(unsigned char x, unsigned char y)
{
	if (x >= this->width || y >= this->height)
	{
		return Map_Tile::None;
	}

	return this->tiles[y].at(x)->tilespec;
}

Map_Warp *Map::GetWarp(unsigned char x, unsigned char y)
{
	if (x >= this->width || y >= this->height)
	{
		return 0;
	}

	return this->tiles[y].at(x)->warp;
}

std::vector<Character *> Map::CharactersInRange(unsigned char x, unsigned char y, unsigned char range)
{
	std::vector<Character *> characters;

	UTIL_FOREACH(this->characters, character)
	{
		if (util::path_length(character->x, character->y, x, y) <= range)
			characters.push_back(character);
	}

	return characters;
}

std::vector<NPC *> Map::NPCsInRange(unsigned char x, unsigned char y, unsigned char range)
{
	std::vector<NPC *> npcs;

	UTIL_FOREACH(this->npcs, npc)
	{
		if (util::path_length(npc->x, npc->y, x, y) <= range)
			npcs.push_back(npc);
	}

	return npcs;
}

void Map::Effect(MapEffect effect, unsigned char param)
{
	PacketBuilder builder(PACKET_EFFECT, PACKET_USE, 2);
	builder.AddChar(effect);
	builder.AddChar(param);

	UTIL_FOREACH(this->characters, character)
	{
		character->Send(builder);
	}
}

bool Map::Evacuate()
{
	if (!this->evacuate_lock)
	{
		this->evacuate_lock = true;

		map_evacuate_struct *evac = new map_evacuate_struct;
		evac->map = this;
		evac->step = int(evac->map->world->config["EvacuateLength"]) / int(evac->map->world->config["EvacuateTick"]);

		TimeEvent *event = new TimeEvent(map_evacuate, evac, this->world->config["EvacuateTick"], evac->step);
		this->world->timer.Register(event);

		map_evacuate(evac);
		return true;
	}
	else
	{
		return false;
	}
}

bool Map::Reload()
{
	char namebuf[6];
	char checkrid[4];

	std::string filename = this->world->config["MapDir"];
	std::sprintf(namebuf, "%05i", this->id);
	filename.append(namebuf);
	filename.append(".emf");

	std::FILE *fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		return false;
	}

	SAFE_SEEK(fh, 0x03, SEEK_SET);
	SAFE_READ(checkrid, sizeof(char), 4, fh);

	if (this->rid[0] == checkrid[0] && this->rid[1] == checkrid[1]
	 && this->rid[2] == checkrid[2] && this->rid[3] == checkrid[3])
	{
		return true;
	}

	std::list<Character *> temp = this->characters;

	this->Unload();

	if (!this->Load())
	{
		return false;
	}

	this->characters = temp;

	std::fclose(fh);

	UTIL_FOREACH(temp, character)
	{
		character->player->client->Upload(FILE_MAP, character->mapid, INIT_MAP_MUTATION);
		character->Refresh(); // TODO: Find a better way to reload NPCs
	}

	this->exists = true;

	return true;
}

Character *Map::GetCharacter(std::string name)
{
	name = util::lowercase(name);

	UTIL_FOREACH(this->characters, character)
	{
		if (character->name.compare(name) == 0)
		{
			return character;
		}
	}

	return 0;
}

Character *Map::GetCharacterPID(unsigned int id)
{
	UTIL_FOREACH(this->characters, character)
	{
		if (character->player->id == id)
		{
			return character;
		}
	}

	return 0;
}

Character *Map::GetCharacterCID(unsigned int id)
{
	UTIL_FOREACH(this->characters, character)
	{
		if (character->id == id)
		{
			return character;
		}
	}

	return 0;
}

NPC *Map::GetNPCIndex(unsigned char index)
{
	UTIL_FOREACH(this->npcs, npc)
	{
		if (npc->index == index)
		{
			return npc;
		}
	}

	return 0;
}
