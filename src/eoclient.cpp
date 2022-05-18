/* eoclient.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eoclient.hpp"

#include "character.hpp"
#include "config.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "eoserver.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "timer.hpp"
#include "world.hpp"

#include "console.hpp"
#include "socket.hpp"
#include "util.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <utility>

void ActionQueue::AddAction(const PacketReader& reader, double time, bool auto_queue)
{
	this->queue.emplace(new ActionQueue_Action(reader, time, auto_queue));
}

ActionQueue::~ActionQueue()
{
	while (!this->queue.empty())
	{
		this->queue.pop();
	}
}

void EOClient::Initialize()
{
	this->upload_fh = 0;
	this->seq_start = 0;
	this->upcoming_seq_start = -1;
	this->seq = 0;
	this->id = this->server()->world->GeneratePlayerID();
	this->length = 0;
	this->packet_state = EOClient::ReadLen1;
	this->state = EOClient::Uninitialized;
	this->player = 0;
	this->version = 0;
	this->needpong = false;
	this->login_attempts = 0;
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
			upload_available = std::fread(&this->send_buffer[this->send_buffer_ppos], 1, upload_available, this->upload_fh);

			// Dynamically rewrite the bytes of the map to enable PK
			if (this->upload_type == FILE_MAP && this->server()->world->config["GlobalPK"] && !this->server()->world->PKExcept(player->character->mapid))
			{
				if (this->upload_pos <= 0x03 && this->upload_pos + upload_available > 0x03)
					this->send_buffer[this->send_buffer_ppos + 0x03 - this->upload_pos] = 0xFF;

				if (this->upload_pos <= 0x03 && this->upload_pos + upload_available > 0x04)
					this->send_buffer[this->send_buffer_ppos + 0x04 - this->upload_pos] = 0x01;

				if (this->upload_pos <= 0x1F && this->upload_pos + upload_available > 0x1F)
					this->send_buffer[this->send_buffer_ppos + 0x1F - this->upload_pos] = 0x04;
			}

			this->upload_pos += upload_available;
			this->send_buffer_ppos += upload_available;
			this->send_buffer_used += upload_available;
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
					data[0] = '\0';
					data.erase(0, 1);
					this->packet_state = EOClient::ReadLen2;

					if (data.length() == 0)
					{
						break;
					}

				case EOClient::ReadLen2:
					this->raw_length[1] = data[0];
					data[0] = '\0';
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
					std::fill(data.begin(), data.begin() + std::min<std::size_t>(data.length(), this->length), '\0');
					data.erase(0, this->length);
					this->length -= this->data.length() - oldlength;

					if (this->length == 0)
					{
						this->Execute(this->data);

						std::fill(UTIL_RANGE(this->data), '\0');
						this->data.erase();
						this->packet_state = EOClient::ReadLen1;

						done = true;
					}
					break;

				default:
					// If the code ever gets here, something is broken, so we just reset the client's state.
					std::fill(UTIL_RANGE(data), '\0');
					std::fill(UTIL_RANGE(this->data), '\0');
					data.erase();
					this->data.erase();
					this->packet_state = EOClient::ReadLen1;
			}
		}
	}
}

void EOClient::InitNewSequence()
{
	this->seq_start = util::rand(0, 1757);
}

void EOClient::PingNewSequence()
{
	this->upcoming_seq_start = util::rand(0, 1757);
}

void EOClient::PongNewSequence()
{
	this->seq_start = this->upcoming_seq_start;
}

void EOClient::AccountReplyNewSequence()
{
	this->seq_start = util::rand(0, 240);
}

int EOClient::GetSeqStart()
{
	return this->seq_start;
}

std::pair<unsigned char, unsigned char> EOClient::GetSeqInitBytes()
{
	int s1_max = (this->seq_start + 13) / 7;
	int s1_min = std::max(0, (this->seq_start - 252 + 13 + 6) / 7);

	unsigned char s1 = util::rand(s1_min, s1_max);
	unsigned char s2 = this->seq_start - s1 * 7 + 13;

	return {s1, s2};
}

std::pair<unsigned short, unsigned char> EOClient::GetSeqUpdateBytes()
{
	int s1_max = this->upcoming_seq_start + 252;
	int s1_min = this->upcoming_seq_start;

	unsigned short s1 = util::rand(s1_min, s1_max);
	unsigned char s2 = s1 - this->upcoming_seq_start;

	return {s1, s2};
}

int EOClient::GenSequence()
{
	int result = std::uint32_t(this->seq_start + this->seq);

	this->seq = (this->seq + 1) % 10;

	return result;
}

int EOClient::GenUpcomingSequence()
{
	int result = std::uint32_t(this->upcoming_seq_start + this->seq);

	this->seq = (this->seq + 1) % 10;

	return result;
}

void EOClient::Execute(const std::string &data)
{
	if (data.length() < 2)
		return;

	if (!this->Connected())
		return;

	PacketReader reader(processor.Decode(data));

	if (reader.Family() == PACKET_INTERNAL)
	{
		Console::Wrn("Closing client connection sending a reserved packet ID: %s", static_cast<std::string>(this->GetRemoteAddr()).c_str());
		this->Close();
		return;
	}

	if (reader.Family() != PACKET_F_INIT)
	{
		bool ping_reply = (reader.Family() == PACKET_CONNECTION && reader.Action() == PACKET_PING);

		if (ping_reply)
			this->PongNewSequence();

		int client_seq;
		int server_seq = this->GenSequence();

		if (server_seq >= 253)
			client_seq = reader.GetShort();
		else
			client_seq = reader.GetChar();

		if (this->server()->world->config["EnforceSequence"])
		{
			if (client_seq != server_seq)
			{
				Console::Wrn("Closing client connection sending invalid sequence: %s, Got %i, expected %i.", static_cast<std::string>(this->GetRemoteAddr()).c_str(), client_seq, server_seq);
				this->Close();
				return;
			}
		}
	}
	else
	{
		this->GenSequence();
	}

	queue.AddAction(reader, 0.02, true);
}

bool EOClient::Upload(FileType type, int id, InitReply init_reply)
{
	this->upload_file_id = id;

	std::string filename;
	std::size_t file_start = 0;
	std::size_t file_length = 0;

	std::vector<std::size_t>* file_splits = nullptr;

	auto load_file_splits = [&](auto pub)
	{
		if (pub.files.empty())
			return false;

		Pub_File& first_file = pub.files[0];

		if (pub.files.size() == 1 && first_file.splits.size() > 2)
		{
			filename = first_file.filename;
			file_splits = &first_file.splits;
		}
		else
		{
			if (id - 1 < 0 || id - 1 > pub.files.size())
				return false;

			filename = pub.files[id - 1].filename;
		}

		return true;
	};

	if (type == FILE_MAP)
	{
		char mapbuf[7];
		std::sprintf(mapbuf, "%05i", int(std::abs(id)));
		filename = std::string(server()->world->config["MapDir"]) + mapbuf + ".emf";
	}
	else if (type == FILE_ITEM)
	{
		if (!load_file_splits(*this->server()->world->eif))
			return false;
	}
	else if (type == FILE_NPC)
	{
		if (!load_file_splits(*this->server()->world->enf))
			return false;
	}
	else if (type == FILE_SPELL)
	{
		if (!load_file_splits(*this->server()->world->esf))
			return false;
	}
	else if (type == FILE_CLASS)
	{
		if (!load_file_splits(*this->server()->world->ecf))
			return false;
	}

	if (file_splits)
	{
		if (id < 1 || id >= file_splits->size())
			return false;

		file_start = (*file_splits)[id - 1];
		file_length = (*file_splits)[id] - file_start;
	}

	return EOClient::Upload(type, filename, file_start, file_length, init_reply);
}

bool EOClient::Upload(FileType type, const std::string &filename, std::size_t file_start, std::size_t file_length, InitReply init_reply)
{
	using std::swap;

	if (this->upload_fh)
		throw std::runtime_error("Already uploading file");

	if (file_length > 63992)
		throw std::runtime_error("File is too large to send");

	this->upload_fh = std::fopen(filename.c_str(), "rb");

	if (!this->upload_fh)
		return false;

	// The size of all pub file headers is 10 bytes
	std::array<char, 10> pub_header_bytes;

	if (file_length == 0)
	{
		if (std::fseek(this->upload_fh, 0, SEEK_END) != 0)
		{
			std::fclose(this->upload_fh);
			return false;
		}

		file_length = std::ftell(this->upload_fh);
		std::fseek(this->upload_fh, 0, SEEK_SET);
	}
	else
	{
		file_length += pub_header_bytes.size();
	}

	this->upload_type = type;
	this->upload_pos = 0;
	this->upload_size = file_length;

	std::size_t temp_buffer_size = this->send_buffer.size();

	// Allocate a power-of-two buffer size large enough to hold the file
	while (temp_buffer_size < this->upload_size + 6)
		temp_buffer_size *= 2;

	this->send_buffer2.resize(temp_buffer_size);
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
		builder.AddChar(this->upload_file_id);

	builder.AddSize(this->upload_size);

	Client::Send(builder);

	// Copy the header from dat001 in to higher numbered files
	if (file_start != 0)
	{
		int bytes_read = std::fread(&pub_header_bytes[0], 1, pub_header_bytes.size(), this->upload_fh);

		if (bytes_read < sizeof pub_header_bytes)
			return false;

		std::copy(pub_header_bytes.begin(), pub_header_bytes.end(), this->send_buffer.begin() + this->send_buffer_ppos);

		this->upload_pos += bytes_read;
		this->send_buffer_ppos += bytes_read;
		this->send_buffer_used += bytes_read;

		if (file_start != pub_header_bytes.size())
			std::fseek(this->upload_fh, file_start, SEEK_SET);
	}

	return true;
}

void EOClient::Send(const PacketBuilder &builder)
{
	std::string data= this->processor.Encode(builder);

	if (this->upload_fh)
	{
		// Stick any incoming data in to our temporary buffer
		if (data.length() > this->send_buffer2.length() - this->send_buffer2_used)
		{
			this->Close(true);
			return;
		}

		const std::size_t mask = this->send_buffer2.length() - 1;

		for (std::size_t i = 0; i < data.length(); ++i)
		{
			this->send_buffer2[this->send_buffer2_ppos] = data[i];
			this->send_buffer2_ppos = (this->send_buffer2_ppos + 1) & mask;
		}

		this->send_buffer2_used += data.length();
	}
	else
	{
		Client::Send(this->processor.Encode(builder));
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
		delete this->player;
	}
}
