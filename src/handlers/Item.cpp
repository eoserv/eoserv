
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

CLIENT_F_FUNC(Item)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player using an item
		{
			if (this->state < EOClient::PlayingModal) return false;

			int id = reader.GetShort();

			if (this->player->character->HasItem(id))
			{
				EIF_Data *item = this->server->world->eif->Get(id);
				reply.SetID(PACKET_ITEM, PACKET_REPLY);
				reply.AddChar(item->type); // ?
				reply.AddShort(id);
				reply.AddInt(this->player->character->HasItem(id)-1);
				reply.AddChar(this->player->character->weight);
				reply.AddChar(this->player->character->maxweight);

				switch (item->type)
				{
					case EIF::Teleport:
					{
						if (this->state < EOClient::Playing)
						{
							break;
						}

						if (this->player->character->mapid == static_cast<int>(this->server->world->config["JailMap"]))
						{
							break;
						}

						if (item->scrollmap == 0)
						{
							this->player->character->Warp(this->player->character->spawnmap, this->player->character->spawnx, this->player->character->spawny, WARP_ANIMATION_SCROLL);
						}
						else
						{
							this->player->character->Warp(item->scrollmap, item->scrollx, item->scrolly, WARP_ANIMATION_SCROLL);
						}

						this->player->character->DelItem(id, 1);
					}
					break;

					case EIF::Heal:
					{
						int hpgain = std::min(int(item->hp), this->player->character->maxhp - this->player->character->hp);
						int tpgain = std::min(int(item->tp), this->player->character->maxtp - this->player->character->tp);
						this->player->character->hp += hpgain;
						this->player->character->tp += tpgain;

						reply.AddInt(hpgain);
						reply.AddShort(this->player->character->hp);
						reply.AddShort(this->player->character->tp);

						PacketBuilder builder(PACKET_RECOVER, PACKET_AGREE);
						builder.AddShort(this->player->id);
						builder.AddInt(hpgain);
						builder.AddChar(int(double(this->player->character->hp) / double(this->player->character->maxhp) * 100.0));

						UTIL_VECTOR_FOREACH_ALL(this->player->character->map->characters, Character *, character)
						{
							if (character != this->player->character && this->player->character->InRange(character))
							{
								character->player->client->SendBuilder(builder);
							}
						}

						this->player->character->DelItem(id, 1);

						if (this->player->character->party)
						{
							this->player->character->party->UpdateHP(this->player->character);
						}
					}
					break;

					default:
						return true;
				}

				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_DROP: // Drop an item on the ground
		{
			if (this->state < EOClient::PlayingModal) return false;

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
			int x = reader.GetByte(); // ?
			int y = reader.GetByte(); // ?

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

			int distance = std::abs(x + y - this->player->character->x - this->player->character->y);

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
					item->unprotecttime = Timer::GetTime() + static_cast<double>(this->server->world->config["ProctectPlayerDrop"]);
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

			int uid = reader.GetShort();

			UTIL_VECTOR_FOREACH_ALL(this->player->character->map->items, Map_Item, item)
			{
				if (item.uid == uid)
				{
					int distance = std::abs(item.x + item.y - this->player->character->x - this->player->character->y);

					if (distance > static_cast<int>(this->server->world->config["DropDistance"]))
					{
						break;
					}

					if (item.owner != this->player->id && item.unprotecttime > Timer::GetTime())
					{
						break;
					}

					this->player->character->AddItem(item.id, item.amount);
					this->player->character->map->DelItem(uid, this->player->character);

					reply.SetID(PACKET_ITEM, PACKET_GET);
					reply.AddShort(uid);
					reply.AddShort(item.id);
					reply.AddThree(item.amount);
					reply.AddChar(this->player->character->weight);
					reply.AddChar(this->player->character->maxweight);
					CLIENT_SEND(reply);
					break;
				}
			}

		}
		break;

		default:
			return false;
	}

	return true;
}
