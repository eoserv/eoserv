
#include "eodata.hpp"

#include <cstdio>

#include "packet.hpp"

EIF::EIF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		std::fprintf(stderr, "Could not load file: %s\n", filename.c_str());
		return;
	}

	std::fseek(fh, 3, SEEK_SET);
	std::fread(this->rid, sizeof(char), 4, fh);
	std::fread(this->len, sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	std::fseek(fh, 1, SEEK_CUR);

	unsigned char namesize;
	char *namebuf;
	std::string name;
	char buf[EIF::DATA_SIZE] = {0};
	EIF_Data newdata;

	this->data.resize(numobj+1, newdata);

	this->data[0] = newdata;

	std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = PacketProcessor::Number(namesize);
		namebuf = new char[namesize];
		std::fread(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete[] namebuf;
		std::fread(buf, sizeof(char), EIF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.graphic = PacketProcessor::Number(buf[0], buf[1]);
		newdata.type = static_cast<EIF::Type>(PacketProcessor::Number(buf[2]));

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
		newdata.scrollmap = PacketProcessor::Number(buf[32]);
		newdata.scrollx = PacketProcessor::Number(buf[35]);
		newdata.scrolly = PacketProcessor::Number(buf[36]);

		newdata.classreq = PacketProcessor::Number(buf[39]);

		newdata.weight = PacketProcessor::Number(buf[55]);

		this->data[i] = newdata;

		std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	std::printf("%i items loaded.\n", this->data.size()-1);

	std::fclose(fh);
}

EIF_Data *EIF::Get(unsigned int id)
{
	if (id > 0 && id < this->data.size())
	{
		return &this->data[id];
	}
	else
	{
		return &this->data[0];
	}
}

ENF::ENF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		std::fprintf(stderr, "Could not load file: %s\n", filename.c_str());
		return;
	}

	std::fseek(fh, 3, SEEK_SET);
	std::fread(this->rid, sizeof(char), 4, fh);
	std::fread(this->len, sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	std::fseek(fh, 1, SEEK_CUR);

	unsigned char namesize;
	char *namebuf;
	std::string name;
	char buf[ENF::DATA_SIZE] = {0};
	ENF_Data newdata;

	this->data.resize(numobj+1, newdata);

	this->data[0] = newdata;

	std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = PacketProcessor::Number(namesize);
		namebuf = new char[namesize];
		std::fread(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete[] namebuf;
		std::fread(buf, sizeof(char), ENF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.graphic = PacketProcessor::Number(buf[0], buf[1]);

		newdata.boss = PacketProcessor::Number(buf[3], buf[4]);
		newdata.child = PacketProcessor::Number(buf[5], buf[6]);
		newdata.type = static_cast<ENF::Type>(PacketProcessor::Number(buf[7], buf[8]));
		newdata.hp = PacketProcessor::Number(buf[11], buf[12], buf[13]);
		newdata.mindam = PacketProcessor::Number(buf[16], buf[17]);
		newdata.maxdam = PacketProcessor::Number(buf[18], buf[19]);

		// TODO: Check these are in the right order
		newdata.accuracy = PacketProcessor::Number(buf[20], buf[21]);
		newdata.evade = PacketProcessor::Number(buf[22], buf[23]);
		newdata.armor = PacketProcessor::Number(buf[24], buf[25]);

		newdata.exp = PacketProcessor::Number(buf[36], buf[37]);

		this->data[i] = newdata;

		std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	std::printf("%i npc types loaded.\n", this->data.size()-1);

	std::fclose(fh);
}

ENF_Data *ENF::Get(unsigned int id)
{
	if (id > 0 && id < this->data.size())
	{
		return &this->data[id];
	}
	else
	{
		return &this->data[0];
	}
}

ESF::ESF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		std::fprintf(stderr, "Could not load file: %s\n", filename.c_str());
		return;
	}

	std::fseek(fh, 3, SEEK_SET);
	std::fread(this->rid, sizeof(char), 4, fh);
	std::fread(this->len, sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	std::fseek(fh, 1, SEEK_CUR);

	unsigned char namesize;
	char *namebuf;
	std::string name;
	char buf[ESF::DATA_SIZE] = {0};
	ESF_Data newdata;

	this->data.resize(numobj+1, newdata);

	this->data[0] = newdata;

	std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = PacketProcessor::Number(namesize);
		namebuf = new char[namesize];
		std::fread(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete[] namebuf;
		std::fread(buf, sizeof(char), ESF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		this->data[i] = newdata;

		std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	std::printf("%i spells loaded.\n", this->data.size()-1);

	std::fclose(fh);
}

ECF::ECF(std::string filename)
{
	std::FILE *fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		std::fprintf(stderr, "Could not load file: %s\n", filename.c_str());
		return;
	}

	std::fseek(fh, 3, SEEK_SET);
	std::fread(this->rid, sizeof(char), 4, fh);
	std::fread(this->len, sizeof(char), 2, fh);
	int numobj = PacketProcessor::Number(this->len[0], this->len[1]);
	std::fseek(fh, 1, SEEK_CUR);

	unsigned char namesize;
	char *namebuf;
	std::string name;
	char buf[ECF::DATA_SIZE] = {0};
	ECF_Data newdata;

	this->data.resize(numobj+1, newdata);

	this->data[0] = newdata;

	std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = PacketProcessor::Number(namesize);
		namebuf = new char[namesize];
		std::fread(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete[] namebuf;
		std::fread(buf, sizeof(char), ECF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		this->data[i] = newdata;

		std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	std::printf("%i classes loaded.\n", this->data.size()-1);

	std::fclose(fh);
}

