
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "map.hpp"

#include "arena.hpp"
#include "character.hpp"
#include "console.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "eoserver.hpp"
#include "npc.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "timer.hpp"
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
	UTIL_PTR_VECTOR_FOREACH(map->chests, Map_Chest, chest)
	{
		bool needs_update = false;

		for (int slot = 1; slot <= chest->slots; ++slot)
		{
			PtrList<Map_Chest_Spawn> spawns;

			UTIL_PTR_LIST_FOREACH(chest->spawns, Map_Chest_Spawn, spawn)
			{
				if (spawn->last_taken + spawn->time*60.0 < current_time)
				{
					bool slot_used = false;

					UTIL_PTR_LIST_FOREACH(chest->items, Map_Chest_Item, item)
					{
						if (item->slot == spawn->slot)
						{
							slot_used = true;
						}
					}

					if (!slot_used)
					{
						spawns.push_back(*spawn);
					}
				}
			}

			if (!spawns.empty())
			{
				PtrList<Map_Chest_Spawn>::Iterator it(spawns);
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

struct map_close_door_struct : public Shared
{
	Map *map;
	unsigned char x, y;
};

void map_close_door(void *map_close_door_struct_void)
{
	map_close_door_struct *close(static_cast<map_close_door_struct *>(map_close_door_struct_void));

	close->map->CloseDoor(close->x, close->y);

	close->Release();
}

int Map_Chest::AddItem(short item, int amount, int slot)
{
	if (amount <= 0)
	{
		return 0;
	}

	if (slot == 0)
	{
		UTIL_PTR_LIST_FOREACH(this->items, Map_Chest_Item, it)
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

		UTIL_PTR_LIST_FOREACH(this->items, Map_Chest_Item, item)
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
	UTIL_PTR_LIST_FOREACH(this->items, Map_Chest_Item, it)
	{
		if (it->id == item)
		{
			int amount = it->amount;

			if (it->slot)
			{
				double current_time = Timer::GetTime();

				UTIL_PTR_LIST_FOREACH(this->spawns, Map_Chest_Spawn, spawn)
				{
					if (spawn->slot == it->slot)
					{
						spawn->last_taken = current_time;
					}
				}
			}

			this->items.erase(it);
			return amount;
		}
	}

	return false;
}

void Map_Chest::Update(Map *map, Character *exclude)
{
	PacketBuilder builder(PACKET_CHEST, PACKET_AGREE);

	UTIL_PTR_LIST_FOREACH(this->items, Map_Chest_Item, item)
	{
		builder.AddShort(item->id);
		builder.AddThree(item->amount);
	}

	UTIL_PTR_LIST_FOREACH(map->characters, Character, character)
	{
		if (*character == exclude)
		{
			continue;
		}

		if (util::path_length(character->x, character->y, this->x, this->y) <= 1)
		{
			character->player->client->SendBuilder(builder);
		}
	}
}

Map::Map(int id, World *world)
{
	this->id = id;
	this->world = world;
	this->exists = false;
	this->jukebox_protect = 0.0;

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
			UTIL_VECTOR_FOREACH_ALL(spawns, std::string, spawn)
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
						s->Release();
						break;

				}
			}
		}
	}
	else
	{
		this->arena = 0;
	}

	this->Load();

	if (!this->chests.empty())
	{
		TimeEvent *event = new TimeEvent(map_spawn_chests, this, 60.0, Timer::FOREVER);
		this->world->timer.Register(event);
		event->Release();
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
	this->filename = filename;
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
		this->tiles[i] = new PtrVector<Map_Tile>;
		this->tiles[i]->resize(width);
		for (int ii = 0; ii < width; ++ii)
		{
			this->tiles[i]->at(ii) = new Map_Tile;
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
				this->chests.push_back(chest); chest->Release();
			}

			try
			{
				this->tiles.at(yloc)->at(xloc) = newtile;
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
				this->tiles.at(yloc)->at(xloc)->warp = newwarp;
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

		UTIL_PTR_VECTOR_FOREACH(this->chests, Map_Chest, chest)
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
				spawn->Release();
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

	UTIL_PTR_VECTOR_FOREACH(this->npcs, NPC, npc)
	{
		UTIL_PTR_LIST_FOREACH(npc->damagelist, NPC_Opponent, opponent)
		{
			erase_first(opponent->attacker->unregister_npc, *npc);
		}
	}

	this->npcs.clear();
}

int Map::GenerateItemID()
{
	int lowest_free_id = 1;
	restart_loop:
	UTIL_PTR_LIST_FOREACH(this->items, Map_Item, item)
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
	UTIL_PTR_VECTOR_FOREACH(this->npcs, NPC, npc)
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
	PacketBuilder builder;
	this->characters.push_back(character);
	character->map = this;

	builder.SetID(PACKET_PLAYERS, PACKET_AGREE);

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
	builder.AddShort(this->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);
	builder.AddShort(this->world->eif->Get(character->paperdoll[Character::Weapon])->dollgraphic);
	builder.AddChar(character->sitting);
	builder.AddChar(character->hidden);
	builder.AddChar(animation);
	builder.AddByte(255);
	builder.AddChar(1); // 0 = NPC, 1 = player

	UTIL_PTR_LIST_FOREACH(this->characters, Character, checkcharacter)
	{
		if (*checkcharacter == character || !character->InRange(*checkcharacter))
		{
			continue;
		}

		checkcharacter->player->client->SendBuilder(builder);
	}
}

void Map::Leave(Character *character, WarpAnimation animation, bool silent)
{
	if (!silent)
	{
		PacketBuilder builder;

		builder.SetID(PACKET_CLOTHES, PACKET_REMOVE);
		builder.AddShort(character->player->id);
		if (animation != WARP_ANIMATION_NONE)
		{
			builder.AddChar(animation);
		}

		UTIL_PTR_LIST_FOREACH(this->characters, Character, checkcharacter)
		{
			if (*checkcharacter == character || !character->InRange(*checkcharacter))
			{
				continue;
			}

			checkcharacter->player->client->SendBuilder(builder);
		}
	}

	erase_first(this->characters, character);
	character->map = 0;
}

void Map::Msg(Character *from, std::string message, bool echo)
{
	message = util::text_cap(message, static_cast<int>(this->world->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from->name) + "  "));

	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddString(message);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if ((!echo && *character == from) || !from->InRange(*character))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

bool Map::Walk(Character *from, Direction direction, bool admin)
{
	PacketBuilder builder;
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

	Map_Warp *warp;
	if (!admin && (warp = this->GetWarp(target_x, target_y)))
	{
		if (from->level >= warp->levelreq && (warp->spec == Map_Warp::NoDoor || warp->open))
		{
			from->Warp(warp->map, warp->x, warp->y);
		}

		return false;
	}

	from->direction = direction;

	HOOK_BEGIN(CharacterEvent, this->world->hookmanager, "character/walk")
		e->character = from;
		e->target_map = this;
		e->target_mapid = this->id;
		e->target_x = target_x;
		e->target_y = target_y;
		if (HOOK_CALL()) return false;
	HOOK_END()

	from->x = target_x;
	from->y = target_y;

	int newx;
	int newy;
	int oldx;
	int oldy;

	std::vector<std::pair<int, int> > newcoords;
	std::vector<std::pair<int, int> > oldcoords;

	PtrVector<Character> newchars;
	PtrVector<Character> oldchars;
	PtrVector<NPC> newnpcs;
	PtrVector<NPC> oldnpcs;
	PtrVector<Map_Item> newitems;

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

	UTIL_PTR_LIST_FOREACH(this->characters, Character, checkchar)
	{
		if (*checkchar == from)
		{
			continue;
		}

		for (std::size_t i = 0; i < oldcoords.size(); ++i)
		{
			if (checkchar->x == oldcoords[i].first && checkchar->y == oldcoords[i].second)
			{
				oldchars.push_back(*checkchar);
			}
			else if (checkchar->x == newcoords[i].first && checkchar->y == newcoords[i].second)
			{
				newchars.push_back(*checkchar);
			}
		}
	}

	UTIL_PTR_VECTOR_FOREACH(this->npcs, NPC, checknpc)
	{
		if (!checknpc->alive)
		{
			continue;
		}

		for (std::size_t i = 0; i < oldcoords.size(); ++i)
		{
			if (checknpc->x == oldcoords[i].first && checknpc->y == oldcoords[i].second)
			{
				oldnpcs.push_back(*checknpc);
			}
			else if (checknpc->x == newcoords[i].first && checknpc->y == newcoords[i].second)
			{
				newnpcs.push_back(*checknpc);
			}
		}
	}

	UTIL_PTR_LIST_FOREACH(this->items, Map_Item, checkitem)
	{
		for (std::size_t i = 0; i < oldcoords.size(); ++i)
		{
			if (checkitem->x == newcoords[i].first && checkitem->y == newcoords[i].second)
			{
				newitems.push_back(*checkitem);
			}
		}
	}

	builder.SetID(PACKET_CLOTHES, PACKET_REMOVE);
	builder.AddShort(from->player->id);

	UTIL_PTR_VECTOR_FOREACH(oldchars, Character, character)
	{
		PacketBuilder rbuilder;
		rbuilder.SetID(PACKET_CLOTHES, PACKET_REMOVE);
		rbuilder.AddShort(character->player->id);

		character->player->client->SendBuilder(builder);
		from->player->client->SendBuilder(rbuilder);
	}

	builder.Reset();

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

	UTIL_PTR_VECTOR_FOREACH(newchars, Character, character)
	{
		PacketBuilder rbuilder;
		rbuilder.SetID(PACKET_PLAYERS, PACKET_AGREE);
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
		rbuilder.AddShort(this->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);
		rbuilder.AddShort(this->world->eif->Get(character->paperdoll[Character::Weapon])->dollgraphic);
		rbuilder.AddChar(character->sitting);
		rbuilder.AddChar(character->hidden);
		rbuilder.AddByte(255);
		rbuilder.AddChar(1); // 0 = NPC, 1 = player

		character->player->client->SendBuilder(builder);
		from->player->client->SendBuilder(rbuilder);
	}

	builder.Reset();

	builder.SetID(PACKET_WALK, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);
	builder.AddChar(from->x);
	builder.AddChar(from->y);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (*character == from || !from->InRange(*character))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}

	builder.Reset();

	builder.SetID(PACKET_WALK, PACKET_REPLY);
	builder.AddByte(255);
	builder.AddByte(255);
	UTIL_PTR_VECTOR_FOREACH(newitems, Map_Item, item)
	{
		builder.AddShort(item->uid);
		builder.AddShort(item->id);
		builder.AddChar(item->x);
		builder.AddChar(item->y);
		builder.AddThree(item->amount);
	}
	from->player->client->SendBuilder(builder);

	builder.SetID(PACKET_APPEAR, PACKET_REPLY);
	UTIL_PTR_VECTOR_FOREACH(newnpcs, NPC, npc)
	{
		builder.Reset();
		builder.AddChar(0);
		builder.AddByte(255);
		builder.AddChar(npc->index);
		builder.AddShort(npc->id);
		builder.AddChar(npc->x);
		builder.AddChar(npc->y);
		builder.AddChar(npc->direction);

		from->player->client->SendBuilder(builder);
	}

	UTIL_PTR_VECTOR_FOREACH(oldnpcs, NPC, npc)
	{
		npc->RemoveFromView(from);
	}

	return true;
}

bool Map::Walk(NPC *from, Direction direction)
{
	PacketBuilder builder;
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

	std::vector<std::pair<int, int> > newcoords;
	std::vector<std::pair<int, int> > oldcoords;

	PtrVector<Character> newchars;
	PtrVector<Character> oldchars;

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

	UTIL_PTR_LIST_FOREACH(this->characters, Character, checkchar)
	{
		for (std::size_t i = 0; i < oldcoords.size(); ++i)
		{
			if (checkchar->x == oldcoords[i].first && checkchar->y == oldcoords[i].second)
			{
				oldchars.push_back(*checkchar);
			}
			else if (checkchar->x == newcoords[i].first && checkchar->y == newcoords[i].second)
			{
				newchars.push_back(*checkchar);
			}
		}
	}

	builder.SetID(PACKET_APPEAR, PACKET_REPLY);
	builder.AddChar(0);
	builder.AddByte(255);
	builder.AddChar(from->index);
	builder.AddShort(from->id);
	builder.AddChar(from->x);
	builder.AddChar(from->y);
	builder.AddChar(from->direction);

	UTIL_PTR_VECTOR_FOREACH(newchars, Character, character)
	{
		character->player->client->SendBuilder(builder);
	}

	builder.Reset();

	builder.SetID(PACKET_NPC, PACKET_PLAYER);
	builder.AddChar(from->index);
	builder.AddChar(from->x);
	builder.AddChar(from->y);
	builder.AddChar(from->direction);
	builder.AddByte(255);
	builder.AddByte(255);
	builder.AddByte(255);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (!character->InRange(from))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}

	UTIL_PTR_VECTOR_FOREACH(oldchars, Character, character)
	{
		from->RemoveFromView(*character);
	}

	return true;
}

void Map::Attack(Character *from, Direction direction)
{
	PacketBuilder builder;

	from->direction = direction;

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

	builder.SetID(PACKET_ATTACK, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (*character == from || !from->InRange(*character))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
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

		if (!this->Walkable(target_x, target_y, true))
		{
			return;
		}

		UTIL_PTR_VECTOR_FOREACH(this->npcs, NPC, npc)
		{
			if ((npc->Data()->type == ENF::Passive || npc->Data()->type == ENF::Aggressive || from->admin > static_cast<int>(this->world->admin_config["killnpcs"]))
			 && npc->alive && npc->x == target_x && npc->y == target_y)
			{
				int amount = util::rand(from->mindam, from->maxdam);
				double rand = util::rand(0.0, 1.0);
				// Checks if target is facing you
				bool critical = std::abs(int(npc->direction) - from->direction) != 2 || rand < static_cast<double>(this->world->config["CriticalRate"]);

				std::map<std::string, double> formula_vars;

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
	}
}

bool Map::AttackPK(Character *from, Direction direction)
{
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

		if (!this->Walkable(target_x, target_y, true))
		{
			return false;
		}

		UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
		{
			if (character->mapid == this->id && !character->nowhere && character->x == target_x && character->y == target_y)
			{
				Character *character_ptr = *character;
				character_ptr->AddRef();

				int amount = util::rand(from->mindam, from->maxdam);
				double rand = util::rand(0.0, 1.0);
				// Checks if target is facing you
				bool critical = std::abs(int(character_ptr->direction) - from->direction) != 2 || rand < static_cast<double>(this->world->config["CriticalRate"]);

				std::map<std::string, double> formula_vars;

				from->FormulaVars(formula_vars);
				character_ptr->FormulaVars(formula_vars, "target_");
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

				int limitamount = std::min(amount, int(character_ptr->hp));

				if (this->world->config["LimitDamage"])
				{
					amount = limitamount;
				}

				character_ptr->hp -= limitamount;

				PacketBuilder builder(PACKET_CLOTHES, PACKET_REPLY);
				builder.AddShort(from->player->id);
				builder.AddShort(character_ptr->player->id);
				builder.AddThree(amount);
				builder.AddChar(from->direction);
				builder.AddChar(int(double(character_ptr->hp) / double(character_ptr->maxhp) * 100.0));
				builder.AddChar(character_ptr->hp == 0);

				UTIL_PTR_LIST_FOREACH(this->characters, Character, checkchar)
				{
					if (character_ptr->InRange(*checkchar))
					{
						checkchar->player->client->SendBuilder(builder);
					}
				}

				if (character_ptr->hp == 0)
				{
					character_ptr->hp = int(character_ptr->maxhp * static_cast<double>(this->world->config["DeathRecover"]) / 100.0);

					if (this->world->config["Deadly"])
					{
						character_ptr->DropAll(from);
					}

					character_ptr->map->Leave(character_ptr, WARP_ANIMATION_NONE, true);

					character_ptr->nowhere = true;
					character_ptr->map = this->world->GetMap(character_ptr->spawnmap);
					character_ptr->mapid = character_ptr->spawnmap;
					character_ptr->x = character_ptr->spawnx;
					character_ptr->y = character_ptr->spawny;

					PacketReader reader("");

					character_ptr->player->client->queue.AddAction(PACKET_INTERNAL, PACKET_INTERNAL_NULL, reader, 1.5);
					character_ptr->player->client->queue.AddAction(PACKET_INTERNAL, PACKET_INTERNAL_WARP, reader, 0.0);
				}

				builder.Reset();
				builder.SetID(PACKET_RECOVER, PACKET_PLAYER);
				builder.AddShort(character_ptr->hp);
				builder.AddShort(character_ptr->tp);
				character_ptr->player->client->SendBuilder(builder);

				character_ptr->Release();

				return true;
			}
		}
	}

	return false;
}

void Map::Face(Character *from, Direction direction)
{
	PacketBuilder builder;

	from->direction = direction;

	builder.SetID(PACKET_FACE, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (*character == from || !from->InRange(*character))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void Map::Sit(Character *from, SitAction sit_type)
{
	PacketBuilder builder;

	from->sitting = sit_type;

	builder.SetID((sit_type == SIT_CHAIR) ? PACKET_CHAIR : PACKET_SIT, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(from->x);
	builder.AddChar(from->y);
	builder.AddChar(from->direction);
	builder.AddChar(0); // ?

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (*character == from || !from->InRange(*character))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void Map::Stand(Character *from)
{
	PacketBuilder builder;

	from->sitting = SIT_STAND;

	builder.SetID(PACKET_SIT, PACKET_REMOVE);
	builder.AddShort(from->player->id);
	builder.AddChar(from->x);
	builder.AddChar(from->y);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (*character == from || !from->InRange(*character))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void Map::Emote(Character *from, enum Emote emote, bool echo)
{
	PacketBuilder builder;

	builder.SetID(PACKET_EMOTE, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(emote);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (!echo && (*character == from || !from->InRange(*character)))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
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
		UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
		{
			if (character->x == x && character->y == y)
			{
				return true;
			}
		}
	}

	if (target != Map::PlayerOnly)
	{
		UTIL_PTR_VECTOR_FOREACH(this->npcs, NPC, npc)
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
			if (!from->HasItem(this->world->eif->GetKey(warp->spec - static_cast<int>(Map_Warp::Door))))
			{
				return false;
			}
		}

		PacketBuilder builder;
		builder.SetID(PACKET_DOOR, PACKET_OPEN);
		builder.AddChar(x);
		builder.AddShort(y);

		UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
		{
			if (character->InRange(x, y))
			{
				character->player->client->SendBuilder(builder);
			}
		}

		warp->open = true;

		map_close_door_struct *close = new map_close_door_struct;
		close->map = this;
		close->x = x;
		close->y = y;

		TimeEvent *event = new TimeEvent(map_close_door, close, this->world->config["DoorTimer"], 1);
		this->world->timer.Register(event);
		event->Release();

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

Map_Item *Map::AddItem(short id, int amount, unsigned char x, unsigned char y, Character *from)
{
	Map_Item *newitem(new Map_Item(GenerateItemID(), id, amount, x, y, 0, 0));

	PacketBuilder builder;
	builder.SetID(PACKET_ITEM, PACKET_ADD);
	builder.AddShort(id);
	builder.AddShort(newitem->uid);
	builder.AddThree(amount);
	builder.AddChar(x);
	builder.AddChar(y);

	if (from || (from && from->admin <= ADMIN_GM))
	{
		int ontile = 0;
		int onmap = 0;

		UTIL_PTR_LIST_FOREACH(this->items, Map_Item, item)
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

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if ((from && *character == from) || !character->InRange(newitem))
		{
			continue;
		}
		character->player->client->SendBuilder(builder);
	}

	this->items.push_back(newitem);
	newitem->Release();
	return this->items.back();
}

Map_Item *Map::GetItem(short uid)
{
	UTIL_PTR_LIST_FOREACH(this->items, Map_Item, item)
	{
		if (item->uid == uid)
		{
			return *item;
		}
	}

	return 0;
}

void Map::DelItem(short uid, Character *from)
{
	UTIL_PTR_LIST_FOREACH(this->items, Map_Item, item)
	{
		if (item->uid == uid)
		{
			PacketBuilder builder;
			builder.SetID(PACKET_ITEM, PACKET_REMOVE);
			builder.AddShort(uid);
			UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
			{
				if ((from && *character == from) || !character->InRange(*item))
				{
					continue;
				}
				character->player->client->SendBuilder(builder);
			}
			this->items.erase(item);
			break;
		}
	}
}

void Map::DelItem(Map_Item *item_, Character *from)
{
	UTIL_PTR_LIST_FOREACH(this->items, Map_Item, item)
	{
		if (item_ == *item)
		{
			PacketBuilder builder;
			builder.SetID(PACKET_ITEM, PACKET_REMOVE);
			builder.AddShort(item->uid);
			UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
			{
				if ((from && *character == from) || !character->InRange(*item))
				{
					continue;
				}
				character->player->client->SendBuilder(builder);
			}
			this->items.erase(item);
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
	return (InBounds(x, y) && this->tiles[y]->at(x)->Walkable(npc));
}

Map_Tile::TileSpec Map::GetSpec(unsigned char x, unsigned char y)
{
	if (x >= this->width || y >= this->height)
	{
		return Map_Tile::None;
	}

	return this->tiles[y]->at(x)->tilespec;
}

Map_Warp *Map::GetWarp(unsigned char x, unsigned char y)
{
	if (x >= this->width || y >= this->height)
	{
		return 0;
	}

	return this->tiles[y]->at(x)->warp;
}

void Map::Effect(int effect, int param)
{
	PacketBuilder builder;
	builder.SetID(PACKET_EFFECT, PACKET_USE);
	builder.AddChar(effect);
	builder.AddChar(param);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		character->player->client->SendBuilder(builder);
	}
}

bool Map::Reload()
{
	char namebuf[6];
	char checkrid[4];

	std::string filename = this->world->config["MapDir"];
	std::sprintf(namebuf, "%05i", this->id);
	this->filename = filename;
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

	PtrList<Character> temp = this->characters;

	this->Unload();

	if (!this->Load())
	{
		return false;
	}

	this->characters = temp;

	PacketBuilder builder(0);
	builder.AddChar(INIT_MAP_MUTATION);

	std::string content;
	std::fseek(fh, 0, SEEK_SET);
	do {
		char buf[4096];
		int len = std::fread(buf, sizeof(char), 4096, fh);
		content.append(buf, len);
	} while (!std::feof(fh));
	std::fclose(fh);

	builder.AddString(content);

	PacketBuilder protect_builder(0);
	protect_builder.AddChar(INIT_BANNED);

	UTIL_PTR_LIST_FOREACH(temp, Character, character)
	{
		character->player->client->Send(builder.Get());
		character->Refresh(); // TODO: Find a better way to reload NPCs

		if (this->world->config["ProtectMaps"])
		{
			character->player->client->Send(protect_builder.Get());
		}
	}

	this->exists = true;

	return true;
}

Character *Map::GetCharacter(std::string name)
{
	name = util::lowercase(name);

	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (character->name.compare(name) == 0)
		{
			return *character;
		}
	}

	return 0;
}

Character *Map::GetCharacterPID(unsigned int id)
{
	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (character->player->id == id)
		{
			return *character;
		}
	}

	return 0;
}

Character *Map::GetCharacterCID(unsigned int id)
{
	UTIL_PTR_LIST_FOREACH(this->characters, Character, character)
	{
		if (character->id == id)
		{
			return *character;
		}
	}

	return 0;
}
