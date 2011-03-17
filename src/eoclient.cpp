
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eoclient.hpp"

#include <algorithm>
#include <cstdio>
#include <stdexcept>

#include "character.hpp"
#include "console.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
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
	this->upload_fh = 0;
	this->id = this->server()->world->GeneratePlayerID();
	this->length = 0;
	this->packet_state = EOClient::ReadLen1;
	this->state = EOClient::Uninitialized;
	this->player = 0;
	this->version = 0;
	this->needpong = false;
}

bool EOClient::NeedTick()
{
	return this->upload_fh;
}

void EOClient::Tick()
{
	std::string data;
	int done = false;
	int oldlength;

	if (this->upload_fh)
	{
		// Send more of the file instead of doing other tasks
		std::size_t upload_available = std::min(this->upload_size - this->upload_pos, Client::SendBufferRemaining());

		if (upload_available != 0)
		{
			std::string buf;
			buf.resize(upload_available);
			upload_available = std::fread(&buf[0], 1, upload_available, this->upload_fh);
			buf.resize(upload_available);

			// Dynamically rewrite the bytes of the map to enable PK
			if (this->upload_type == FILE_MAP && this->server()->world->config["GlobalPK"] && !this->server()->world->PKExcept(player->character->mapid))
			{
				if (this->upload_pos <= 0x03 && this->upload_pos + upload_available > 0x03) buf[0x03 - this->upload_pos] = 0xFF;
				if (this->upload_pos <= 0x03 && this->upload_pos + upload_available > 0x04) buf[0x04 - this->upload_pos] = 0x01;
				if (this->upload_pos <= 0x1F && this->upload_pos + upload_available > 0x1F) buf[0x1F - this->upload_pos] = 0x04;
			}

			Client::Send(buf);
			this->upload_pos += upload_available;
		}
		else if (this->upload_pos == this->upload_size && this->SendBufferRemaining() == this->send_buffer.length())
		{
			using std::swap;

			std::fclose(this->upload_fh);
			this->upload_fh = 0;
			this->upload_pos = 0;
			this->upload_size = 0;

			// Place our temporary buffer back as the real one
			swap(this->send_buffer, this->send_buffer2);
			swap(this->send_buffer_gpos, this->send_buffer2_gpos);
			swap(this->send_buffer_ppos, this->send_buffer2_ppos);
			swap(this->send_buffer_used, this->send_buffer2_used);

			// We're not using this anymore...
			std::string empty;
			swap(this->send_buffer2, empty);
		}
	}
	else
	{
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
}

void EOClient::Execute(const std::string &data)
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

bool EOClient::Upload(FileType type, int id, InitReply init_reply)
{
	char mapbuf[7];
	std::sprintf(mapbuf, "%05i", int(std::abs(id)));

	switch (type)
	{
		case FILE_MAP: return EOClient::Upload(type, std::string(server()->world->config["MapDir"]) + mapbuf + ".emf", init_reply);
		case FILE_ITEM: return EOClient::Upload(type, std::string(this->server()->world->config["EIF"]), init_reply);
		case FILE_NPC: return EOClient::Upload(type, std::string(this->server()->world->config["ENF"]),init_reply);
		case FILE_SPELL: return EOClient::Upload(type, std::string(this->server()->world->config["ESF"]), init_reply);
		case FILE_CLASS: return EOClient::Upload(type, std::string(this->server()->world->config["ECF"]), init_reply);
		default: return false;
	}
}

bool EOClient::Upload(FileType type, const std::string &filename, InitReply init_reply)
{
	using std::swap;

	if (this->upload_fh)
		throw std::runtime_error("Already uploading file");

	this->upload_fh = std::fopen(filename.c_str(), "rb");

	if (!this->upload_fh)
		return false;

	if (std::fseek(this->upload_fh, 0, SEEK_END) != 0)
	{
		std::fclose(this->upload_fh);
		return false;
	}

	this->upload_type = type;
	this->upload_pos = 0;
	this->upload_size = std::ftell(this->upload_fh);

	std::fseek(this->upload_fh, 0, SEEK_SET);

	this->send_buffer2.resize(this->send_buffer.size());
	this->send_buffer2_gpos = 0;
	this->send_buffer2_ppos = 0;
	this->send_buffer2_used = 0;

	swap(this->send_buffer, this->send_buffer2);
	swap(this->send_buffer_gpos, this->send_buffer2_gpos);
	swap(this->send_buffer_ppos, this->send_buffer2_ppos);
	swap(this->send_buffer_used, this->send_buffer2_used);

	// Build the file upload header packet
	PacketBuilder builder(PACKET_F_INIT, PACKET_A_INIT, 2);
	builder.AddChar(init_reply);

	if (type != FILE_MAP)
		builder.AddChar(1);

	builder.AddSize(this->upload_size);

	Client::Send(builder);

	return true;
}

void EOClient::Send(const PacketBuilder &builder)
{
	std::string data(builder);

	std::array<unsigned char, 2> id = PacketProcessor::EPID(builder.GetID());

	if (id[1] != PACKET_F_INIT)
		data = this->processor.Encode(data);

	if (this->upload_fh)
	{
		// Stick any incoming data in to our temporary buffer
		if (data.length() > this->send_buffer2.length() - this->send_buffer_used)
		{
			this->Close(true);
			return;
		}

		const std::size_t mask = this->send_buffer2.length() - 1;

		for (std::size_t i = 0; i < data.length(); ++i)
		{
			this->send_buffer2_ppos = (this->send_buffer2_ppos + 1) & mask;
			this->send_buffer2[this->send_buffer2_ppos] = data[i];
		}

		this->send_buffer2_used += data.length();
	}
	else
	{
		Client::Send(data);
	}
}

EOClient::~EOClient()
{
	if (this->upload_fh)
	{
		std::fclose(this->upload_fh);
	}

	if (this->player)
	{
		this->player->Logout();
	}
}
