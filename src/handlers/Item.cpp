
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../party.hpp"
#include "../player.hpp"
#include "../quest.hpp"
#include "../world.hpp"

namespace Handlers
{

// Player using an item
void Item_Use(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	int id = reader.GetShort();

	if (character->HasItem(id))
	{
		const EIF_Data& item = character->world->eif->Get(id);
		PacketBuilder reply(PACKET_ITEM, PACKET_REPLY, 3);
		reply.AddChar(item.type);
		reply.AddShort(id);

		switch (item.type)
		{
			case EIF::Teleport:
			{
				if (!character->map->scroll)
					break;

				if (item.scrollmap == 0)
				{
					if (character->mapid == character->SpawnMap() && character->x == character->SpawnX() && character->y == character->SpawnY())
						break;
				}
				else
				{
					if (character->mapid == item.scrollmap && character->x == item.scrollx && character->y == item.scrolly)
						break;
				}

				character->DelItem(id, 1);

				reply.ReserveMore(6);
				reply.AddInt(character->HasItem(id));
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);

				if (item.scrollmap == 0)
				{
					character->Warp(character->SpawnMap(), character->SpawnX(), character->SpawnY(), WARP_ANIMATION_SCROLL);
				}
				else
				{
					character->Warp(item.scrollmap, item.scrollx, item.scrolly, WARP_ANIMATION_SCROLL);
				}

				character->Send(reply);

				UTIL_FOREACH(character->quests, q) { q.second->UsedItem(id); }
			}
			break;

			case EIF::Heal:
			{
				int hpgain = item.hp;
				int tpgain = item.tp;

				if (character->world->config["LimitDamage"])
				{
					hpgain = std::min(hpgain, character->maxhp - character->hp);
					tpgain = std::min(tpgain, character->maxtp - character->tp);
				}

				hpgain = std::max(hpgain, 0);
				tpgain = std::max(tpgain, 0);

				if (hpgain == 0 && tpgain == 0)
					break;

				character->hp += hpgain;
				character->tp += tpgain;

				if (!character->world->config["LimitDamage"])
				{
					character->hp = std::min(character->hp, character->maxhp);
					character->tp = std::min(character->tp, character->maxtp);
				}

				character->DelItem(id, 1);

				reply.ReserveMore(14);
				reply.AddInt(character->HasItem(id));
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);

				reply.AddInt(hpgain);
				reply.AddShort(character->hp);
				reply.AddShort(character->tp);

				PacketBuilder builder(PACKET_RECOVER, PACKET_AGREE, 7);
				builder.AddShort(character->player->id);
				builder.AddInt(hpgain);
				builder.AddChar(util::clamp<int>(double(character->hp) / double(character->maxhp) * 100.0, 0, 100));

				UTIL_FOREACH(character->map->characters, updatecharacter)
				{
					if (updatecharacter != character && character->InRange(updatecharacter))
					{
						updatecharacter->Send(builder);
					}
				}

				if (character->party)
				{
					character->party->UpdateHP(character);
				}

				character->Send(reply);

				UTIL_FOREACH(character->quests, q) { q.second->UsedItem(id); }
			}
			break;

			case EIF::HairDye:
			{
				if (character->haircolor == item.haircolor)
					break;

				character->haircolor = item.haircolor;

				character->DelItem(id, 1);

				reply.ReserveMore(7);
				reply.AddInt(character->HasItem(id));
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);

				reply.AddChar(item.haircolor);

				PacketBuilder builder(PACKET_AVATAR, PACKET_AGREE, 5);
				builder.AddShort(character->player->id);
				builder.AddChar(SLOT_HAIRCOLOR);
				builder.AddChar(0); // subloc
				builder.AddChar(item.haircolor);

				UTIL_FOREACH(character->map->characters, updatecharacter)
				{
					if (updatecharacter != character && character->InRange(updatecharacter))
					{
						updatecharacter->Send(builder);
					}
				}

				character->Send(reply);

				UTIL_FOREACH(character->quests, q) { q.second->UsedItem(id); }
			}
			break;

			case EIF::Beer:
			{
				character->DelItem(id, 1);

				reply.ReserveMore(6);
				reply.AddInt(character->HasItem(id));
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);

				character->Send(reply);

				UTIL_FOREACH(character->quests, q) { q.second->UsedItem(id); }
			}
			break;

			case EIF::EffectPotion:
			{
				character->DelItem(id, 1);

				reply.ReserveMore(8);
				reply.AddInt(character->HasItem(id));
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);
				reply.AddShort(item.effect);

				character->Effect(item.effect, false);

				character->Send(reply);

				UTIL_FOREACH(character->quests, q) { q.second->UsedItem(id); }
			}
			break;

			case EIF::CureCurse:
			{
				bool found = false;

				for (std::size_t i = 0; i < character->paperdoll.size(); ++i)
				{
					if (character->world->eif->Get(character->paperdoll[i]).special == EIF::Cursed)
					{
						character->paperdoll[i] = 0;
						found = true;
					}
				}

				if (!found)
					break;

				character->CalculateStats();

				character->DelItem(id, 1);

				reply.ReserveMore(32);
				reply.AddInt(character->HasItem(id));
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);

				reply.AddShort(character->maxhp);
				reply.AddShort(character->maxtp);

				reply.AddShort(character->display_str);
				reply.AddShort(character->display_intl);
				reply.AddShort(character->display_wis);
				reply.AddShort(character->display_agi);
				reply.AddShort(character->display_con);
				reply.AddShort(character->display_cha);

				reply.AddShort(character->mindam);
				reply.AddShort(character->maxdam);
				reply.AddShort(character->accuracy);
				reply.AddShort(character->evade);
				reply.AddShort(character->armor);

				PacketBuilder builder(PACKET_AVATAR, PACKET_AGREE, 14);
				builder.AddShort(character->player->id);
				builder.AddChar(SLOT_CLOTHES);
				builder.AddChar(0); // sound
				builder.AddShort(character->world->eif->Get(character->paperdoll[Character::Boots]).dollgraphic);
				builder.AddShort(character->world->eif->Get(character->paperdoll[Character::Armor]).dollgraphic);
				builder.AddShort(character->world->eif->Get(character->paperdoll[Character::Hat]).dollgraphic);

				const EIF_Data& wep = character->world->eif->Get(character->paperdoll[Character::Weapon]);

				builder.AddShort(wep.dollgraphic);

				if (wep.subtype == EIF::TwoHanded && wep.dual_wield_dollgraphic)
					builder.AddShort(wep.dual_wield_dollgraphic);
				else
					builder.AddShort(character->world->eif->Get(character->paperdoll[Character::Shield]).dollgraphic);

				UTIL_FOREACH(character->map->characters, updatecharacter)
				{
					if (updatecharacter != character && character->InRange(updatecharacter))
					{
						updatecharacter->Send(builder);
					}
				}

				character->Send(reply);

				UTIL_FOREACH(character->quests, q) { q.second->UsedItem(id); }
			}
			break;

			case EIF::EXPReward:
			{
				bool level_up = false;

				character->exp += item.expreward;

				character->exp = std::min(character->exp, static_cast<int>(character->map->world->config["MaxExp"]));

				while (character->level < static_cast<int>(character->map->world->config["MaxLevel"])
				 && character->exp >= character->map->world->exp_table[character->level+1])
				{
					level_up = true;
					++character->level;
					character->statpoints += static_cast<int>(character->map->world->config["StatPerLevel"]);
					character->skillpoints += static_cast<int>(character->map->world->config["SkillPerLevel"]);
					character->CalculateStats();
				}

				character->DelItem(id, 1);

				reply.ReserveMore(21);
				reply.AddInt(character->HasItem(id));
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);

				reply.AddInt(character->exp);

				reply.AddChar(level_up ? character->level : 0);
				reply.AddShort(character->statpoints);
				reply.AddShort(character->skillpoints);
				reply.AddShort(character->maxhp);
				reply.AddShort(character->maxtp);
				reply.AddShort(character->maxsp);

				if (level_up)
				{
					PacketBuilder builder(PACKET_RECOVER, PACKET_REPLY, 9);
					builder.AddInt(character->exp);
					builder.AddShort(character->karma);
					builder.AddChar(character->level);
					builder.AddShort(character->statpoints);
					builder.AddShort(character->skillpoints);

					UTIL_FOREACH(character->map->characters, check)
					{
						if (character != check && character->InRange(check))
						{
							PacketBuilder builder(PACKET_ITEM, PACKET_ACCEPT, 2);
							builder.AddShort(character->player->id);
							character->Send(builder);
						}
					}
				}

				character->Send(reply);

				UTIL_FOREACH(character->quests, q) { q.second->UsedItem(id); }
			}
			break;

			default:
				return;
		}
	}
}

// Drop an item on the ground
void Item_Drop(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	int id = reader.GetShort();
	int amount;

	if (character->world->eif->Get(id).special == EIF::Lore)
	{
		return;
	}

	if (reader.Length() == 10)
	{
		amount = reader.GetThree();
	}
	else
	{
		amount = reader.GetInt();
	}
	unsigned char x = reader.GetByte(); // ?
	unsigned char y = reader.GetByte(); // ?

	amount = std::min<int>(amount, character->world->config["MaxDrop"]);

	if (amount == 0)
	{
		return;
	}

	if (x == 255 && y == 255)
	{
		x = character->x;
		y = character->y;
	}
	else
	{
		x = PacketProcessor::Number(x);
		y = PacketProcessor::Number(y);
	}

	int distance = util::path_length(x, y, character->x, character->y);

	if (distance > static_cast<int>(character->world->config["DropDistance"]))
	{
		return;
	}

	if (!character->map->Walkable(x, y))
	{
		return;
	}

	if (character->HasItem(id) >= amount && character->mapid != static_cast<int>(character->world->config["JailMap"]))
	{
		std::shared_ptr<Map_Item> item = character->map->AddItem(id, amount, x, y, character);

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

// Destroying an item
void Item_Junk(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	int id = reader.GetShort();
	int amount = reader.GetInt();

	if (character->HasItem(id) >= amount)
	{
		character->DelItem(id, amount);

		PacketBuilder reply(PACKET_ITEM, PACKET_JUNK, 11);
		reply.AddShort(id);
		reply.AddThree(amount); // Overflows, does it matter?
		reply.AddInt(character->HasItem(id));
		reply.AddChar(character->weight);
		reply.AddChar(character->maxweight);
		character->Send(reply);
	}
}

// Retrieve an item from the ground
void Item_Get(Character *character, PacketReader &reader)
{
	int uid = reader.GetShort();

	std::shared_ptr<Map_Item> item = character->map->GetItem(uid);

	if (item)
	{
		int distance = util::path_length(item->x, item->y, character->x, character->y);

		if (distance > static_cast<int>(character->world->config["DropDistance"]))
		{
			return;
		}

		if (item->owner != character->player->id && item->unprotecttime > Timer::GetTime())
		{
			return;
		}

		int taken = character->CanHoldItem(item->id, item->amount);

		character->AddItem(item->id, taken);

		PacketBuilder reply(PACKET_ITEM, PACKET_GET, 9);
		reply.AddShort(uid);
		reply.AddShort(item->id);
		reply.AddThree(taken);
		reply.AddChar(character->weight);
		reply.AddChar(character->maxweight);
		character->Send(reply);

		character->map->DelSomeItem(item->uid, taken, character);
	}
}

PACKET_HANDLER_REGISTER(PACKET_ITEM)
	Register(PACKET_USE, Item_Use, Playing);
	Register(PACKET_DROP, Item_Drop, Playing);
	Register(PACKET_JUNK, Item_Junk, Playing);
	Register(PACKET_GET, Item_Get, Playing);
PACKET_HANDLER_REGISTER_END()

}
