
CLIENT_F_FUNC(Chair)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Player sitting on a chair
		{
			if (!this->player || !this->player->character) return false;

			int action = reader.GetChar();

			if (action == SIT_SITTING && this->player->character->sitting == SIT_STAND)
			{
				int x = reader.GetChar();
				int y = reader.GetChar();

				if ((x + y - this->player->character->x - this->player->character->y) > 1)
				{
					return false;
				}

				UTIL_LIST_FOREACH_ALL(this->player->character->map->characters, Character *, character)
				{
					if (character->x == x && character->y == y)
					{
						return false;
					}
				}

				switch (this->player->character->map->GetSpec(x, y))
				{
					case Map_Tile::ChairDown:
						if (this->player->character->y == y+1 && this->player->character->x == x)
						{
							this->player->character->direction = DIRECTION_DOWN;
						}
						else
						{
							return false;
						}
						break;

					case Map_Tile::ChairUp:
						if (this->player->character->y == y-1 && this->player->character->x == x)
						{
							this->player->character->direction = DIRECTION_UP;
						}
						else
						{
							return false;
						}
						break;

					case Map_Tile::ChairLeft:
						if (this->player->character->y == y && this->player->character->x == x-1)
						{
							this->player->character->direction = DIRECTION_LEFT;
						}
						else
						{
							return false;
						}
						break;

					case Map_Tile::ChairRight:
						if (this->player->character->y == y && this->player->character->x == x+1)
						{
							this->player->character->direction = DIRECTION_RIGHT;
						}
						else
						{
							return false;
						}
						break;

					case Map_Tile::ChairDownRight:
						if (this->player->character->y == y && this->player->character->x == x+1)
						{
							this->player->character->direction = DIRECTION_RIGHT;
						}
						else if (this->player->character->y == y+1 && this->player->character->x == x)
						{
							this->player->character->direction = DIRECTION_DOWN;
						}
						else
						{
							return false;
						}
						break;

					case Map_Tile::ChairUpLeft:
						if (this->player->character->y == y && this->player->character->x == x-1)
						{
							this->player->character->direction = DIRECTION_LEFT;
						}
						else if (this->player->character->y == y-1 && this->player->character->x == x)
						{
							this->player->character->direction = DIRECTION_UP;
						}
						else
						{
							return false;
						}
						break;

					case Map_Tile::ChairAll:
						if (this->player->character->y == y && this->player->character->x == x+1)
						{
							this->player->character->direction = DIRECTION_RIGHT;
						}
						else if (this->player->character->y == y && this->player->character->x == x-1)
						{
							this->player->character->direction = DIRECTION_LEFT;
						}
						else if (this->player->character->y == y-1 && this->player->character->x == x)
						{
							this->player->character->direction = DIRECTION_UP;
						}
						else if (this->player->character->y == y+1 && this->player->character->x == x)
						{
							this->player->character->direction = DIRECTION_DOWN;
						}
						else
						{
							return false;
						}
						break;

					default:
						return false;
				}

				this->player->character->x = x;
				this->player->character->y = y;

				reply.SetID(PACKET_CHAIR, PACKET_PLAYER);
				reply.AddShort(this->player->id);
				reply.AddChar(this->player->character->x);
				reply.AddChar(this->player->character->y);
				reply.AddChar(this->player->character->direction);
				reply.AddChar(0); // ?
				CLIENT_SEND(reply);
				this->player->character->Sit(SIT_CHAIR);
			}
			else if (this->player->character->sitting == SIT_CHAIR)
			{
				switch (this->player->character->direction)
				{
					case DIRECTION_UP:
						--this->player->character->y;
						break;
					case DIRECTION_RIGHT:
						++this->player->character->x;
						break;
					case DIRECTION_DOWN:
						++this->player->character->y;
						break;
					case DIRECTION_LEFT:
						--this->player->character->x;
						break;
				}

				reply.SetID(PACKET_CHAIR, PACKET_CLOSE);
				reply.AddShort(this->player->id);
				reply.AddChar(this->player->character->x);
				reply.AddChar(this->player->character->y);
				CLIENT_SEND(reply);
				this->player->character->Stand();
			}
		}

		break;

		default:
			return false;
	}

	return true;
}
