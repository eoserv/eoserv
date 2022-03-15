/* eodata.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eodata.hpp"

#include "console.hpp"
#include "packet.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>

static const char *eodata_safe_fail_filename;

static void eodata_safe_fail(int line)
{
	Console::Err("Invalid file / failed read/seek: %s -- %i", eodata_safe_fail_filename, line);
	std::exit(1);
}

#define SAFE_SEEK(fh, offset, from) if (std::fseek(fh, offset, from) != 0) { std::fclose(fh); eodata_safe_fail(__LINE__); }
#define SAFE_READ(buf, size, count, fh) if (std::fread(buf, size, count, fh) != static_cast<std::size_t>(count)) { std::fclose(fh); eodata_safe_fail(__LINE__); }

void EIF::Read(const std::string& filename)
{
	this->data.clear();

	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	eodata_safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid.data(), sizeof(char), 4, fh);
	SAFE_READ(this->len.data(), sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	this->file_splits.reserve(1 + (numobj / EIF::FILE_MAX_ENTRIES));

	unsigned char namesize;
	std::string name;
	char buf[EIF::DATA_SIZE] = {0};

	this->data.resize(numobj+1);

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		EIF_Data& newdata = this->data[i];

		if ((i - 1) % EIF::FILE_MAX_ENTRIES == 0)
			this->file_splits.push_back(std::size_t(std::ftell(fh)) - 1);

		namesize = PacketProcessor::Number(namesize);
		name.resize(namesize);
		SAFE_READ(&name[0], sizeof(char), namesize, fh);
		SAFE_READ(buf, sizeof(char), EIF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.graphic = PacketProcessor::Number(buf[0], buf[1]);
		newdata.type = static_cast<EIF::Type>(PacketProcessor::Number(buf[2]));
		newdata.subtype = static_cast<EIF::SubType>(PacketProcessor::Number(buf[3]));
		// Ranged gun hack
		if (newdata.id == 365 && newdata.name == "Gun")
		{
			newdata.subtype = EIF::Ranged;
		}
		// / Ranged gun hack
		newdata.special = static_cast<EIF::Special>(PacketProcessor::Number(buf[4]));
		newdata.hp = PacketProcessor::Number(buf[5], buf[6]);
		newdata.tp = PacketProcessor::Number(buf[7], buf[8]);
		newdata.mindam = PacketProcessor::Number(buf[9], buf[10]);
		newdata.maxdam = PacketProcessor::Number(buf[11], buf[12]);
		newdata.accuracy = PacketProcessor::Number(buf[13], buf[14]);
		newdata.evade = PacketProcessor::Number(buf[15], buf[16]);
		newdata.armor = PacketProcessor::Number(buf[17], buf[18]);

		newdata.str = PacketProcessor::Number(buf[20]);
		newdata.intl = PacketProcessor::Number(buf[21]);
		newdata.wis = PacketProcessor::Number(buf[22]);
		newdata.agi = PacketProcessor::Number(buf[23]);
		newdata.con = PacketProcessor::Number(buf[24]);
		newdata.cha = PacketProcessor::Number(buf[25]);

		newdata.light = PacketProcessor::Number(buf[26]);
		newdata.dark = PacketProcessor::Number(buf[27]);
		newdata.earth = PacketProcessor::Number(buf[28]);
		newdata.air = PacketProcessor::Number(buf[29]);
		newdata.water = PacketProcessor::Number(buf[30]);
		newdata.fire = PacketProcessor::Number(buf[31]);

		newdata.scrollmap = PacketProcessor::Number(buf[32], buf[33], buf[34]);
		newdata.scrollx = PacketProcessor::Number(buf[35]);
		newdata.scrolly = PacketProcessor::Number(buf[36]);

		newdata.levelreq = PacketProcessor::Number(buf[37], buf[38]);
		newdata.classreq = PacketProcessor::Number(buf[39], buf[40]);

		newdata.strreq = PacketProcessor::Number(buf[41], buf[42]);
		newdata.intreq = PacketProcessor::Number(buf[43], buf[44]);
		newdata.wisreq = PacketProcessor::Number(buf[45], buf[46]);
		newdata.agireq = PacketProcessor::Number(buf[47], buf[48]);
		newdata.conreq = PacketProcessor::Number(buf[49], buf[50]);
		newdata.chareq = PacketProcessor::Number(buf[51], buf[52]);

		newdata.weight = PacketProcessor::Number(buf[55]);

		newdata.size = static_cast<EIF::Size>(PacketProcessor::Number(buf[57]));

		if (i < numobj && std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data.back().name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	this->file_splits.push_back(std::size_t(std::ftell(fh)));

	Console::Out("%i items loaded.", this->data.size()-1);

	std::fclose(fh);
}

EIF_Data& EIF::Get(unsigned int id)
{
	if (id < this->data.size())
		return this->data[id];
	else
		return this->data[0];
}

const EIF_Data& EIF::Get(unsigned int id) const
{
	if (id < this->data.size())
		return this->data[id];
	else
		return this->data[0];
}

unsigned int EIF::GetKey(int keynum) const
{
	for (std::size_t i = 0; i < this->data.size(); ++i)
	{
		if (this->Get(i).type == EIF::Key && this->Get(i).key == keynum)
			return i;
	}

	return 0;
}

void ENF::Read(const std::string& filename)
{
	this->data.clear();

	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	eodata_safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid.data(), sizeof(char), 4, fh);
	SAFE_READ(this->len.data(), sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	this->file_splits.reserve(1 + (numobj / ENF::FILE_MAX_ENTRIES));

	unsigned char namesize;
	std::string name;
	char buf[ENF::DATA_SIZE] = {0};

	this->data.resize(numobj+1);

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		ENF_Data& newdata = this->data[i];

		if ((i - 1) % ENF::FILE_MAX_ENTRIES == 0)
			this->file_splits.push_back(std::size_t(std::ftell(fh)) - 1);

		namesize = PacketProcessor::Number(namesize);
		name.resize(namesize);
		SAFE_READ(&name[0], sizeof(char), namesize, fh);
		SAFE_READ(buf, sizeof(char), ENF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.graphic = PacketProcessor::Number(buf[0], buf[1]);

		newdata.boss = PacketProcessor::Number(buf[3], buf[4]);
		newdata.child = PacketProcessor::Number(buf[5], buf[6]);
		newdata.type = static_cast<ENF::Type>(PacketProcessor::Number(buf[7], buf[8]));
		newdata.vendor_id = PacketProcessor::Number(buf[9], buf[10]);
		newdata.hp = PacketProcessor::Number(buf[11], buf[12], buf[13]);

		newdata.mindam = PacketProcessor::Number(buf[16], buf[17]);
		newdata.maxdam = PacketProcessor::Number(buf[18], buf[19]);

		newdata.accuracy = PacketProcessor::Number(buf[20], buf[21]);
		newdata.evade = PacketProcessor::Number(buf[22], buf[23]);
		newdata.armor = PacketProcessor::Number(buf[24], buf[25]);

		newdata.exp = PacketProcessor::Number(buf[36], buf[37]);

		if (i < numobj && std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data.back().name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	this->file_splits.push_back(std::size_t(std::ftell(fh)));

	Console::Out("%i npc types loaded.", this->data.size()-1);

	std::fclose(fh);
}

ENF_Data& ENF::Get(unsigned int id)
{
	if (id < this->data.size())
		return this->data[id];
	else
		return this->data[0];
}

const ENF_Data& ENF::Get(unsigned int id) const
{
	if (id < this->data.size())
		return this->data[id];
	else
		return this->data[0];
}

void ESF::Read(const std::string& filename)
{
	this->data.clear();

	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	eodata_safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid.data(), sizeof(char), 4, fh);
	SAFE_READ(this->len.data(), sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	this->file_splits.reserve(1 + (numobj / ESF::FILE_MAX_ENTRIES));

	unsigned char namesize, shoutsize;
	std::string name, shout;
	char buf[ESF::DATA_SIZE] = {0};

	this->data.resize(numobj+1);

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	SAFE_READ(static_cast<void *>(&shoutsize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		ESF_Data& newdata = this->data[i];

		if ((i - 1) % ESF::FILE_MAX_ENTRIES == 0)
			this->file_splits.push_back(std::size_t(std::ftell(fh)) - 2);

		namesize = PacketProcessor::Number(namesize);
		name.resize(namesize);
		if (namesize > 0)
		{
			SAFE_READ(&name[0], sizeof(char), namesize, fh);
		}

		shoutsize = PacketProcessor::Number(shoutsize);
		shout.resize(shoutsize);
		if (shoutsize > 0)
		{
			SAFE_READ(&shout[0], sizeof(char), shoutsize, fh);
		}

		SAFE_READ(buf, sizeof(char), ESF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;
		newdata.shout = shout;

		newdata.icon = PacketProcessor::Number(buf[0], buf[1]);
		newdata.graphic = PacketProcessor::Number(buf[2], buf[3]);
		newdata.tp = PacketProcessor::Number(buf[4], buf[5]);
		newdata.sp = PacketProcessor::Number(buf[6], buf[7]);
		newdata.cast_time = PacketProcessor::Number(buf[8]);

		newdata.type = static_cast<ESF::Type>(PacketProcessor::Number(buf[11]));
		newdata.target_restrict = static_cast<ESF::TargetRestrict>(PacketProcessor::Number(buf[17]));
		newdata.target = static_cast<ESF::Target>(PacketProcessor::Number(buf[18]));

		newdata.mindam = PacketProcessor::Number(buf[23], buf[24]);
		newdata.maxdam = PacketProcessor::Number(buf[25], buf[26]);
		newdata.accuracy = PacketProcessor::Number(buf[27], buf[28]);
		newdata.hp = PacketProcessor::Number(buf[34], buf[35]);

		if (i < numobj && std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}

		if (i < numobj && std::fread(static_cast<void *>(&shoutsize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data.back().name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	this->file_splits.push_back(std::size_t(std::ftell(fh)));

	Console::Out("%i spells loaded.", this->data.size()-1);

	std::fclose(fh);
}

ESF_Data& ESF::Get(unsigned int id)
{
	if (id < this->data.size())
		return this->data[id];
	else
		return this->data[0];
}

const ESF_Data& ESF::Get(unsigned int id) const
{
	if (id < this->data.size())
		return this->data[id];
	else
		return this->data[0];
}

void ECF::Read(const std::string& filename)
{
	this->data.clear();

	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	eodata_safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid.data(), sizeof(char), 4, fh);
	SAFE_READ(this->len.data(), sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	this->file_splits.reserve(1 + (numobj / ECF::FILE_MAX_ENTRIES));

	unsigned char namesize;
	std::string name;
	char buf[ECF::DATA_SIZE] = {0};

	this->data.resize(numobj+1);

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		ECF_Data& newdata = this->data[i];

		if ((i - 1) % ECF::FILE_MAX_ENTRIES == 0)
			this->file_splits.push_back(std::size_t(std::ftell(fh)) - 1);

		namesize = PacketProcessor::Number(namesize);
		name.resize(namesize);
		SAFE_READ(&name[0], sizeof(char), namesize, fh);

		SAFE_READ(buf, sizeof(char), ECF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.base = PacketProcessor::Number(buf[0]);
		newdata.type = PacketProcessor::Number(buf[1]);

		newdata.str = PacketProcessor::Number(buf[2], buf[3]);
		newdata.intl = PacketProcessor::Number(buf[4], buf[5]);
		newdata.wis = PacketProcessor::Number(buf[6], buf[7]);
		newdata.agi = PacketProcessor::Number(buf[8], buf[9]);
		newdata.con = PacketProcessor::Number(buf[10], buf[11]);
		newdata.cha = PacketProcessor::Number(buf[12], buf[13]);

		if (i < numobj && std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data.back().name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	this->file_splits.push_back(std::size_t(std::ftell(fh)));

	Console::Out("%i classes loaded.", this->data.size()-1);

	std::fclose(fh);
}

ECF_Data& ECF::Get(unsigned int id)
{
	if (id < this->data.size())
		return this->data[id];
	else
		return this->data[0];
}

const ECF_Data& ECF::Get(unsigned int id) const
{
	if (id < this->data.size())
		return this->data[id];
	else
		return this->data[0];
}

#undef SAFE_SEEK
#undef SAFE_READ
