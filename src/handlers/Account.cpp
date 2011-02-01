
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "console.hpp"

#include "eoserver.hpp"
#include "player.hpp"
#include "world.hpp"

CLIENT_F_FUNC(Account)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Check if a character exists
		{
			if (this->state != EOClient::Initialized) return false;
			std::string username = reader.GetEndString();

			username = util::lowercase(username);

			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			if (!Player::ValidName(username))
			{
				reply.AddShort(ACCOUNT_NOT_APPROVED);
				reply.AddString("NO");
			}
			else if (this->server()->world->PlayerExists(username))
			{
				reply.AddShort(ACCOUNT_EXISTS);
				reply.AddString("NO");
			}
			else
			{
				reply.AddShort(ACCOUNT_CONTINUE);
				reply.AddString("OK");
			}
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_CREATE: // Account creation
		{
			if (this->state != EOClient::Initialized) return false;

			reader.GetShort(); // Account creation "session ID"
			reader.GetByte(); // ?

			std::string username = reader.GetBreakString();
			std::string password = reader.GetBreakString();
			std::string fullname = reader.GetBreakString();
			std::string location = reader.GetBreakString();
			std::string email = reader.GetBreakString();
			std::string computer = reader.GetBreakString();

			int hdid;
			try
			{
				hdid = static_cast<int>(util::to_uint_raw(reader.GetBreakString()));
			}
			catch (std::invalid_argument)
			{
				return false;
			}

			username = util::lowercase(username);

			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			if (!Player::ValidName(username))
			{
				reply.AddShort(ACCOUNT_NOT_APPROVED);
				reply.AddString("NO");
			}
			else if (this->server()->world->PlayerExists(username))
			{
				reply.AddShort(ACCOUNT_EXISTS);
				reply.AddString("NO");
			}
			else
			{
				username = util::lowercase(username);

				this->server()->world->CreatePlayer(username, password, fullname, location, email, computer, util::to_string(hdid), static_cast<std::string>(this->GetRemoteAddr()));
				reply.AddShort(ACCOUNT_CREATED);
				reply.AddString("OK");
				Console::Out("New account: %s", username.c_str());
			}

			CLIENT_SEND(reply);
		}
		break;

		case PACKET_AGREE: // Change password
		{
			if (this->state != EOClient::LoggedIn) return false;

			std::string username = reader.GetBreakString();
			std::string oldpassword = reader.GetBreakString();
			std::string newpassword = reader.GetBreakString();

			if (!Player::ValidName(username))
			{
				reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
				reply.AddShort(ACCOUNT_NOT_APPROVED);
				reply.AddString("NO");
				CLIENT_SEND(reply);
				return true;
			}
			else if (!this->server()->world->PlayerExists(username))
			{
				return true;
			}

			Player *changepass = this->server()->world->Login(username, oldpassword);

			if (!changepass)
			{
				reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
				reply.AddShort(ACCOUNT_CHANGE_FAILED);
				reply.AddString("NO");
				CLIENT_SEND(reply);
				return true;
			}

			changepass->ChangePass(newpassword);

			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			reply.AddShort(ACCOUNT_CHANGED);
			reply.AddString("OK");
			CLIENT_SEND(reply);

			changepass->Release();
		}
		break;

		default:
			return false;
	}

	return true;
}
