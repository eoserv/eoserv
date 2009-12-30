
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "eodata.hpp"
#include "map.hpp"
#include "party.hpp"

CLIENT_F_FUNC(Item)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player using an item
		{
			if (this->state < EOClient::PlayingModal) return false;
			CLIENT_QUEUE_ACTION(0.0)

			int id = reader.GetShort();

			if (this->player->character->HasItem(id))
			{
				EIF_Data *item = this->server->world->eif->Get(id);
				reply.SetID(PACKET_ITEM, PACKET_REPLY);
				reply.AddChar(item->type);
				reply.AddShort(id);

				switch (item->type)
				{
					case EIF::Teleport:
					{
						if (this->state < EOClient::Playing)
						{
							break;
						}

						if (!this->player->character->map->scroll)
						{
							break;
						}

						this->player->character->DelItem(id, 1);
						reply.AddInt(this->player->character->HasItem(id));
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);

						if (item->scrollmap == 0)
						{
							this->player->character->Warp(this->player->character->spawnmap, this->player->character->spawnx, this->player->character->spawny, WARP_ANIMATION_SCROLL);
						}
						else
						{
							this->player->character->Warp(item->scrollmap, item->scrollx, item->scrolly, WARP_ANIMATION_SCROLL);
						}

						CLIENT_SEND(reply);
					}
					break;

					case EIF::Heal:
					{
						int hpgain = item->hp;
						int tpgain = item->tp;

						if (this->server->world->config["LimitDamage"])
						{
							hpgain = std::min(hpgain, this->player->character->maxhp - this->player->character->hp);
							tpgain = std::min(tpgain, this->player->character->maxtp - this->player->character->tp);
						}

						this->player->character->hp += hpgain;
						this->player->character->tp += tpgain;

						if (!this->server->world->config["LimitDamage"])
						{
							this->player->character->hp = std::min(this->player->character->hp, this->player->character->maxhp);
							this->player->character->tp = std::min(this->player->character->tp, this->player->character->maxtp);
						}

						this->player->character->DelItem(id, 1);
						reply.AddInt(this->player->character->HasItem(id));
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);

						reply.AddInt(hpgain);
						reply.AddShort(this->player->character->hp);
						reply.AddShort(this->player->character->tp);

						PacketBuilder builder(PACKET_RECOVER, PACKET_AGREE);
						builder.AddShort(this->player->id);
						builder.AddInt(hpgain);
						builder.AddChar(int(double(this->player->character->hp) / double(this->player->character->maxhp) * 100.0));

						UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
						{
							if (*character != this->player->character && this->player->character->InRange(*character))
							{
								character->player->client->SendBuilder(builder);
							}
						}

						if (this->player->character->party)
						{
							this->player->character->party->UpdateHP(this->player->character);
						}

						CLIENT_SEND(reply);
					}
					break;

					case EIF::HairDye:
					{
						this->player->character->haircolor = item->haircolor;

						this->player->character->DelItem(id, 1);
						reply.AddInt(this->player->character->HasItem(id));
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);

						reply.AddChar(item->haircolor);

						PacketBuilder builder(PACKET_CLOTHES, PACKET_AGREE);
						builder.AddShort(this->player->id);
						builder.AddChar(SLOT_HAIRCOLOR);
						builder.AddChar(0); // subloc
						builder.AddChar(item->haircolor);

						UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
						{
							if (*character != this->player->character && this->player->character->InRange(*character))
							{
								character->player->client->SendBuilder(builder);
							}
						}

						CLIENT_SEND(reply);
					}
					break;

					case EIF::Beer:
					{
						this->player->character->DelItem(id, 1);
						reply.AddInt(this->player->character->HasItem(id));
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);

						PacketBuilder builder(PACKET_CLOTHES, PACKET_AGREE);
						builder.AddShort(this->player->id);
						builder.AddChar(SLOT_HAIRCOLOR);
						builder.AddChar(0); // subloc
						builder.AddChar(item->haircolor);

						UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
						{
							if (*character != this->player->character && this->player->character->InRange(*character))
							{
								character->player->client->SendBuilder(builder);
							}
						}
					}
					break;

					case EIF::EffectPotion:
					{
						this->player->character->DelItem(id, 1);
						reply.AddInt(this->player->character->HasItem(id));
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);
						reply.AddShort(item->effect);

						this->player->character->Effect(item->effect, false);

						CLIENT_SEND(reply);
					}
					break;

					case EIF::CureCurse:
					{
						for (std::size_t i = 0; i < this->player->character->paperdoll.size(); ++i)
						{
							if (this->server->world->eif->Get(this->player->character->paperdoll[i])->special == EIF::Cursed)
							{
								this->player->character->paperdoll[i] = 0;
							}
						}

						this->player->character->CalculateStats();

						this->player->character->DelItem(id, 1);
						reply.AddInt(this->player->character->HasItem(id));
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);

						reply.AddShort(this->player->character->maxhp);
						reply.AddShort(this->player->character->maxtp);
						reply.AddShort(this->player->character->str);
						reply.AddShort(this->player->character->intl);
						reply.AddShort(this->player->character->wis);
						reply.AddShort(this->player->character->agi);
						reply.AddShort(this->player->character->con);
						reply.AddShort(this->player->character->cha);
						reply.AddShort(this->player->character->mindam);
						reply.AddShort(this->player->character->maxdam);
						reply.AddShort(this->player->character->accuracy);
						reply.AddShort(this->player->character->evade);
						reply.AddShort(this->player->character->armor);

						PacketBuilder builder;
						builder.SetID(PACKET_CLOTHES, PACKET_AGREE);
						builder.AddShort(this->player->id);
						builder.AddChar(SLOT_CLOTHES);
						builder.AddChar(0);
						builder.AddShort(this->server->world->eif->Get(this->player->character->paperdoll[Character::Boots])->dollgraphic);
						builder.AddShort(this->server->world->eif->Get(this->player->character->paperdoll[Character::Armor])->dollgraphic);
						builder.AddShort(this->server->world->eif->Get(this->player->character->paperdoll[Character::Hat])->dollgraphic);
						builder.AddShort(this->server->world->eif->Get(this->player->character->paperdoll[Character::Weapon])->dollgraphic);
						builder.AddShort(this->server->world->eif->Get(this->player->character->paperdoll[Character::Shield])->dollgraphic);

						UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
						{
							if (*character != this->player->character && this->player->character->InRange(*character))
							{
								character->player->client->SendBuilder(builder);
							}
						}

						CLIENT_SEND(reply);
					}
					break;

					case EIF::EXPReward:
					{
						bool level_up = false;

						this->player->character->exp += item->expreward;

						this->player->character->exp = std::min(this->player->character->exp, static_cast<int>(this->player->character->map->world->config["MaxExp"]));

						while (this->player->character->level < static_cast<int>(this->player->character->map->world->config["MaxLevel"])
						 && this->player->character->exp >= this->player->character->map->world->exp_table[this->player->character->level+1])
						{
							level_up = true;
							++this->player->character->level;
							this->player->character->statpoints += static_cast<int>(this->player->character->map->world->config["StatPerLevel"]);
							this->player->character->skillpoints += static_cast<int>(this->player->character->map->world->config["SkillPerLevel"]);
							this->player->character->CalculateStats();
						}

						this->player->character->DelItem(id, 1);
						reply.AddInt(this->player->character->HasItem(id));
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);

						reply.AddInt(this->player->character->exp);

						reply.AddChar(level_up ? this->player->character->level : 0);
						reply.AddShort(this->player->character->statpoints);
						reply.AddShort(this->player->character->skillpoints);
						reply.AddShort(this->player->character->maxhp);
						reply.AddShort(this->player->character->maxtp);
						reply.AddShort(this->player->character->maxsp);

						if (level_up)
						{
							// TODO: Something better than this
							this->player->character->Emote(EMOTE_LEVELUP, false);
						}

						CLIENT_SEND(reply);
					}
					break;

					default:
						return true;
				}
			}
		}
		break;

		case PACKET_DROP: // Drop an item on the ground
		{
			if (this->state < EOClient::PlayingModal) return false;
			CLIENT_QUEUE_ACTION(0.0)

			int id = reader.GetShort();
			int amount;

			if (this->server->world->eif->Get(id)->special == EIF::Lore)
			{
				return true;
			}

			if (reader.Length() == 8)
			{
				amount = reader.GetThree();
			}
			else
			{
				amount = reader.GetInt();
			}
			unsigned char x = reader.GetByte(); // ?
			unsigned char y = reader.GetByte(); // ?

			amount = std::min<int>(amount, this->server->world->config["MaxDrop"]);

			if (amount == 0)
			{
				return true;
			}

			if (x == 255 && y == 255)
			{
				x = this->player->character->x;
				y = this->player->character->y;
			}
			else
			{
				if (this->state < EOClient::Playing) return false;
				x = PacketProcessor::Number(x);
				y = PacketProcessor::Number(y);
			}

			int distance = util::path_length(x, y, this->player->character->x, this->player->character->y);

			if (distance > static_cast<int>(this->server->world->config["DropDistance"]))
			{
				return true;
			}

			if (!this->player->character->map->Walkable(x, y))
			{
				return true;
			}

			if (this->player->character->HasItem(id) >= amount && this->player->character->mapid != static_cast<int>(this->server->world->config["JailMap"]))
			{
				Map_Item *item = this->player->character->map->AddItem(id, amount, x, y, this->player->character);
				if (item)
				{
					item->owner = this->player->id;
					item->unprotecttime = Timer::GetTime() + static_cast<double>(this->server->world->config["ProtectPlayerDrop"]);
					this->player->character->DelItem(id, amount);

					reply.SetID(PACKET_ITEM, PACKET_DROP);
					reply.AddShort(id);
					reply.AddThree(amount);
					reply.AddInt(this->player->character->HasItem(id));
					reply.AddShort(item->uid);
					reply.AddChar(x);
					reply.AddChar(y);
					reply.AddChar(this->player->character->weight);
					reply.AddChar(this->player->character->maxweight);
					CLIENT_SEND(reply);
				}
			}
		}
		break;

		case PACKET_JUNK: // Destroying an item
		{
			if (this->state < EOClient::PlayingModal) return false;

			int id = reader.GetShort();
			int amount = reader.GetInt();

			if (this->player->character->HasItem(id) >= amount)
			{
				this->player->character->DelItem(id, amount);

				reply.SetID(PACKET_ITEM, PACKET_JUNK);
				reply.AddShort(id);
				reply.AddThree(amount); // Overflows, does it matter?
				reply.AddInt(this->player->character->HasItem(id));
				reply.AddChar(this->player->character->weight);
				reply.AddChar(this->player->character->maxweight);
				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_GET: // Retrieve an item from the ground
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.0)

			int uid = reader.GetShort();

			Map_Item *item = this->player->character->map->GetItem(uid);
			if (item)
			{
				int distance = util::path_length(item->x, item->y, this->player->character->x, this->player->character->y);

				if (distance > static_cast<int>(this->server->world->config["DropDistance"]))
				{
					break;
				}

				if (item->owner != this->player->id && item->unprotecttime > Timer::GetTime())
				{
					break;
				}

				this->player->character->AddItem(item->id, item->amount);

				reply.SetID(PACKET_ITEM, PACKET_GET);
				reply.AddShort(uid);
				reply.AddShort(item->id);
				reply.AddThree(item->amount);
				reply.AddChar(this->player->character->weight);
				reply.AddChar(this->player->character->maxweight);
				CLIENT_SEND(reply);

				this->player->character->map->DelItem(item, this->player->character);

				break;
			}

		}
		break;

		default:
			return false;
	}

	return true;
}
