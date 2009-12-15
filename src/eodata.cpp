
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eodata.hpp"

#include "console.hpp"
#include "packet.hpp"

static const char *safe_fail_filename;

static void safe_fail(int line)
{
	Console::Err("Invalid file / failed read/seek: %s -- %i", safe_fail_filename, line);
	std::exit(1);
}

#define SAFE_SEEK(fh, offset, from) if (std::fseek(fh, offset, from) != 0) { std::fclose(fh); safe_fail(__LINE__); }
#define SAFE_READ(buf, size, count, fh) if (std::fread(buf, size, count, fh) != static_cast<int>(count)) { std::fclose(fh); safe_fail(__LINE__); }

EIF::EIF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);
	SAFE_READ(this->len, sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	unsigned char namesize;
	std::string name;
	char buf[EIF::DATA_SIZE] = {0};

	this->data.resize(numobj+1);

	this->data[0] = new EIF_Data;

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		EIF_Data *newdata = new EIF_Data;

		namesize = PacketProcessor::Number(namesize);
		name.resize(namesize);
		SAFE_READ(&name[0], sizeof(char), namesize, fh);
		SAFE_READ(buf, sizeof(char), EIF::DATA_SIZE, fh);

		newdata->id = i;
		newdata->name = name;

		newdata->graphic = PacketProcessor::Number(buf[0], buf[1]);
		newdata->type = static_cast<EIF::Type>(PacketProcessor::Number(buf[2]));
		newdata->subtype = static_cast<EIF::SubType>(PacketProcessor::Number(buf[3]));
		// Ranged gun hack
		if (newdata->id == 365 && newdata->name == "Gun")
		{
			newdata->subtype = EIF::Ranged;
		}
		// / Ranged gun hack
		newdata->special = static_cast<EIF::Special>(PacketProcessor::Number(buf[4]));
		newdata->hp = PacketProcessor::Number(buf[5], buf[6]);
		newdata->tp = PacketProcessor::Number(buf[7], buf[8]);
		newdata->mindam = PacketProcessor::Number(buf[9], buf[10]);
		newdata->maxdam = PacketProcessor::Number(buf[11], buf[12]);
		newdata->accuracy = PacketProcessor::Number(buf[13], buf[14]);
		newdata->evade = PacketProcessor::Number(buf[15], buf[16]);
		newdata->armor = PacketProcessor::Number(buf[17], buf[18]);
		newdata->str = PacketProcessor::Number(buf[20]);
		newdata->intl = PacketProcessor::Number(buf[21]);
		newdata->wis = PacketProcessor::Number(buf[22]);
		newdata->agi = PacketProcessor::Number(buf[23]);
		newdata->con = PacketProcessor::Number(buf[24]);
		newdata->cha = PacketProcessor::Number(buf[25]);
		newdata->scrollmap = PacketProcessor::Number(buf[32]);
		newdata->scrollx = PacketProcessor::Number(buf[35]);
		newdata->scrolly = PacketProcessor::Number(buf[36]);

		newdata->classreq = PacketProcessor::Number(buf[39]);

		newdata->weight = PacketProcessor::Number(buf[55]);

		this->data[i] = newdata;

		if (std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data.back()->name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	Console::Out("%i items loaded.", this->data.size()-1);

	std::fclose(fh);
}

EIF_Data *EIF::Get(unsigned int id)
{
	if (id > 0 && id < this->data.size())
	{
		return this->data[id];
	}
	else
	{
		return this->data[0];
	}
}

ENF::ENF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);
	SAFE_READ(this->len, sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	unsigned char namesize;
	std::string name;
	char buf[ENF::DATA_SIZE] = {0};

	this->data.resize(numobj+1);

	this->data[0] = new ENF_Data;

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		ENF_Data *newdata = new ENF_Data;

		namesize = PacketProcessor::Number(namesize);
		name.resize(namesize);
		SAFE_READ(&name[0], sizeof(char), namesize, fh);
		SAFE_READ(buf, sizeof(char), ENF::DATA_SIZE, fh);

		newdata->id = i;
		newdata->name = name;

		newdata->graphic = PacketProcessor::Number(buf[0], buf[1]);

		newdata->boss = PacketProcessor::Number(buf[3], buf[4]);
		newdata->child = PacketProcessor::Number(buf[5], buf[6]);
		newdata->type = static_cast<ENF::Type>(PacketProcessor::Number(buf[7], buf[8]));
		newdata->hp = PacketProcessor::Number(buf[11], buf[12], buf[13]);

		newdata->mindam = PacketProcessor::Number(buf[16], buf[17]);
		newdata->maxdam = PacketProcessor::Number(buf[18], buf[19]);

		newdata->accuracy = PacketProcessor::Number(buf[20], buf[21]);
		newdata->evade = PacketProcessor::Number(buf[22], buf[23]);
		newdata->armor = PacketProcessor::Number(buf[24], buf[25]);

		newdata->exp = PacketProcessor::Number(buf[36], buf[37]);

		this->data[i] = newdata;

		if (std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data.back()->name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	Console::Out("%i npc types loaded.", this->data.size()-1);

	std::fclose(fh);
}

ENF_Data *ENF::Get(unsigned int id)
{
	if (id > 0 && id < this->data.size())
	{
		return this->data[id];
	}
	else
	{
		return this->data[0];
	}
}

ESF::ESF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);
	SAFE_READ(this->len, sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	unsigned char namesize, shoutsize;
	std::string name, shout;
	char buf[ESF::DATA_SIZE] = {0};

	this->data.resize(numobj+1);

	this->data[0] = new ESF_Data;

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	SAFE_READ(static_cast<void *>(&shoutsize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		ESF_Data *newdata = new ESF_Data;

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

		newdata->id = i;
		newdata->name = name;
		newdata->shout = shout;

		this->data[i] = newdata;

		if (std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}

		if (std::fread(static_cast<void *>(&shoutsize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data.back()->name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	Console::Out("%i spells loaded.", this->data.size()-1);

	std::fclose(fh);
}

ECF::ECF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");
	safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	SAFE_SEEK(fh, 3, SEEK_SET);
	SAFE_READ(this->rid, sizeof(char), 4, fh);
	SAFE_READ(this->len, sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	SAFE_SEEK(fh, 1, SEEK_CUR);

	unsigned char namesize;
	std::string name;
	char buf[ECF::DATA_SIZE] = {0};

	this->data.resize(numobj+1);

	this->data[0] = new ECF_Data;

	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		ECF_Data *newdata = new ECF_Data;

		namesize = PacketProcessor::Number(namesize);
		name.resize(namesize);
		SAFE_READ(&name[0], sizeof(char), namesize, fh);

		SAFE_READ(buf, sizeof(char), ECF::DATA_SIZE, fh);

		newdata->id = i;
		newdata->name = name;

		this->data[i] = newdata;

		if (std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}
	}

	if (this->data.back()->name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	Console::Out("%i classes loaded.", this->data.size()-1);

	std::fclose(fh);
}

