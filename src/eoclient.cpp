
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eoclient.hpp"

#include "console.hpp"
#include "eoclient.hpp"
#include "eoserver.hpp"
#include "nanohttp.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "timer.hpp"
#include "socket.hpp"
#include "world.hpp"
#include "handlers/handlers.hpp"

#define CLIENT_F_HANDLE(ID,FUNC) \
case ID: \
	result = this->Handle_##FUNC(family, action, reader, false);\
	break

void ActionQueue::AddAction(PacketReader reader, double time, bool auto_queue)
{
	this->queue.push(new ActionQueue_Action(reader, time, auto_queue));
}

ActionQueue::~ActionQueue()
{
	while (!this->queue.empty())
	{
		delete this->queue.front();
		this->queue.pop();
	}
}

void EOClient::Initialize()
{
	this->id = this->server()->world->GeneratePlayerID();
	this->length = 0;
	this->packet_state = EOClient::ReadLen1;
	this->state = EOClient::Uninitialized;
	this->player = 0;
	this->version = 0;
	this->needpong = false;
}

void EOClient::Tick()
{
	std::string data;
	int done = false;
	int oldlength;

	data = this->Recv((this->packet_state == EOClient::ReadData) ? this->length : 1);

	while (data.length() > 0 && !done)
	{
		switch (this->packet_state)
		{
			case EOClient::ReadLen1:
				this->raw_length[0] = data[0];
				data.erase(0, 1);
				this->packet_state = EOClient::ReadLen2;

				if (data.length() == 0)
				{
					break;
				}

			case EOClient::ReadLen2:
				this->raw_length[1] = data[0];
				data.erase(0, 1);
				this->length = PacketProcessor::Number(this->raw_length[0], this->raw_length[1]);
				this->packet_state = EOClient::ReadData;

				if (data.length() == 0)
				{
					break;
				}

			case EOClient::ReadData:
				oldlength = this->data.length();
				this->data += data.substr(0, this->length);
				data.erase(0, this->length);
				this->length -= this->data.length() - oldlength;

				if (this->length == 0)
				{
					try
					{
						this->Execute(this->data);
					}
					catch (Socket_Exception &e)
					{
						Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(this->GetRemoteAddr()).c_str());
						Console::Err("%s: %s", e.what(), e.error());
						this->Close();
					}
					catch (Database_Exception &e)
					{
						Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(this->GetRemoteAddr()).c_str());
						Console::Err("%s: %s", e.what(), e.error());
						this->Close();
					}
					catch (std::runtime_error &e)
					{
						Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(this->GetRemoteAddr()).c_str());
						Console::Err("Runtime Error: %s", e.what());
						this->Close();
					}
					catch (std::logic_error &e)
					{
						Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(this->GetRemoteAddr()).c_str());
						Console::Err("Logic Error: %s", e.what());
						this->Close();
					}
					catch (std::exception &e)
					{
						Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(this->GetRemoteAddr()).c_str());
						Console::Err("Uncaught Exception: %s", e.what());
						this->Close();
					}
					catch (...)
					{
						Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(this->GetRemoteAddr()).c_str());
						this->Close();
					}

					this->data.erase();
					this->packet_state = EOClient::ReadLen1;

					done = true;
				}
				break;

			default:
				// If the code ever gets here, something is broken, so we just reset the client's state.
				data.erase();
				this->data.erase();
				this->packet_state = EOClient::ReadLen1;
		}
	}
}

void EOClient::Execute(std::string data)
{
	if (data.length() < 2)
		return;

	PacketReader reader(processor.Decode(data));

	if (reader.Family() == PACKET_INTERNAL)
	{
		Console::Wrn("Closing client connection sending a reserved packet ID.");
		this->Close();
		return;
	}

	if (reader.Family() != PACKET_F_INIT)
	{
		reader.GetChar(); // Ordering Byte
	}

	queue.AddAction(reader, 0.02, true);
}

void EOClient::Send(const PacketBuilder &builder)
{
	std::string packet(builder);

	std::array<unsigned char, 2> id = PacketProcessor::EPID(builder.GetID());

	if (id[1] != PACKET_F_INIT)
		Client::Send(this->processor.Encode(packet));
	else
		Client::Send(packet);
}

EOClient::~EOClient()
{
	if (this->player)
	{
		this->player->Logout();
	}
}
