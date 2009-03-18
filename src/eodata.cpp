
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
	char buf[EIF::DATA_SIZE] = {0,};
	EIF_Data newdata;

	newdata.id = 0;
	newdata.graphic = 0;
	newdata.classreq = 0;

	newdata.hp = 0;
	newdata.tp = 0;
	newdata.mindam = 0;
	newdata.maxdam = 0;
	newdata.accuracy = 0;
	newdata.evade = 0;
	newdata.armor = 0;

	newdata.str = 0;
	newdata.intl = 0;
	newdata.wis = 0;
	newdata.agi = 0;
	newdata.con = 0;
	newdata.cha = 0;

	newdata.scrollx = 0;
	newdata.scrolly = 0;

	this->nulldata = new EIF_Data;
	*this->nulldata = newdata;

	this->data.resize(numobj+1, *this->nulldata);

	std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = PacketProcessor::Number(namesize);
		namebuf = new char[namesize];
		std::fread(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete namebuf;
		std::fread(buf, sizeof(char), EIF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.graphic = PacketProcessor::Number(buf[0], buf[1]);
		newdata.type = static_cast<EIF::Type>(PacketProcessor::Number(buf[2]));
		newdata.classreq = PacketProcessor::Number(buf[3]);
		newdata.special = static_cast<EIF::Special>(PacketProcessor::Number(buf[4]));
		newdata.hp = PacketProcessor::Number(buf[4], buf[5]);
		newdata.tp = PacketProcessor::Number(buf[6], buf[7]);
		newdata.mindam = PacketProcessor::Number(buf[8], buf[9]);
		newdata.maxdam = PacketProcessor::Number(buf[10], buf[11]);
		newdata.accuracy = PacketProcessor::Number(buf[12], buf[13]);
		newdata.evade = PacketProcessor::Number(buf[14], buf[15]);
		newdata.armor = PacketProcessor::Number(buf[16], buf[17]);
		newdata.str = PacketProcessor::Number(buf[19]);
		newdata.intl = PacketProcessor::Number(buf[20]);
		newdata.wis = PacketProcessor::Number(buf[21]);
		newdata.agi = PacketProcessor::Number(buf[22]);
		newdata.con = PacketProcessor::Number(buf[23]);
		newdata.cha = PacketProcessor::Number(buf[24]);
		newdata.scrollx = PacketProcessor::Number(buf[31]);
		newdata.scrolly = PacketProcessor::Number(buf[32]);

		this->data.assign(i, newdata);

		std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	std::printf("%i items loaded.\n", this->data.size());

	std::fclose(fh);
}

int EIF::GetType(unsigned int id){ if (id > 0 && id < this->data.size()-1){ return this->data[id-1].type; } else { return this->nulldata->type; } }
int EIF::GetGraphic(unsigned int id){ if (id > 0 && id < this->data.size()-1){ return this->data[id-1].graphic; } else { return this->nulldata->graphic; } }
int EIF::GetDollGraphic(unsigned int id){ if (id > 0 && id < this->data.size()-1){ return this->data[id-1].dollgraphic; } else { return this->nulldata->dollgraphic; } }

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
	char buf[ENF::DATA_SIZE] = {0,};
	ENF_Data newdata;

	newdata.id = 0;
	newdata.graphic = 0;
	newdata.boss = 0;
	newdata.child = 0;
	newdata.type = static_cast<ENF::Type>(0);

	this->nulldata = new ENF_Data;
	*this->nulldata = newdata;

	this->data.resize(numobj+1, *this->nulldata);

	this->data[0] = *this->nulldata;

	std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = PacketProcessor::Number(namesize);
		namebuf = new char[namesize];
		std::fread(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete namebuf;
		std::fread(buf, sizeof(char), ENF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		newdata.graphic = PacketProcessor::Number(buf[0], buf[1]);

		newdata.boss = PacketProcessor::Number(buf[3], buf[4]);
		newdata.child = PacketProcessor::Number(buf[5], buf[5]);
		newdata.type = static_cast<ENF::Type>(PacketProcessor::Number(buf[7], buf[8]));
		newdata.exp = PacketProcessor::Number(buf[36], buf[37]);

		this->data.assign(i, newdata);

		std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	std::printf("%i npcs loaded.\n", this->data.size());

	std::fclose(fh);
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
	char buf[ESF::DATA_SIZE] = {0,};
	ESF_Data newdata;

	newdata.id = 0;

	this->nulldata = new ESF_Data;
	*this->nulldata = newdata;

	this->data.resize(numobj+1, *this->nulldata);

	this->data[0] = *this->nulldata;

	std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = PacketProcessor::Number(namesize);
		namebuf = new char[namesize];
		std::fread(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete namebuf;
		std::fread(buf, sizeof(char), ESF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		this->data.assign(i, newdata);

		std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	std::printf("%i spells loaded.\n", this->data.size());

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
	char buf[ECF::DATA_SIZE] = {0,};
	ECF_Data newdata;

	newdata.id = 0;

	this->nulldata = new ECF_Data;
	*this->nulldata = newdata;

	this->data.resize(numobj+1, *this->nulldata);

	this->data[0] = *this->nulldata;

	std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	for (int i = 1; i <= numobj; ++i)
	{
		namesize = PacketProcessor::Number(namesize);
		namebuf = new char[namesize];
		std::fread(namebuf, sizeof(char), namesize, fh);
		name.assign(namebuf,namesize);
		delete namebuf;
		std::fread(buf, sizeof(char), ECF::DATA_SIZE, fh);

		newdata.id = i;
		newdata.name = name;

		this->data.assign(i, newdata);

		std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh);
	}

	if (newdata.name.compare("eof") == 0)
	{
		this->data.pop_back();
	}

	std::printf("%i classes loaded.\n", this->data.size());

	std::fclose(fh);
}

