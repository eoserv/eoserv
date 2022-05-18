/* eodata.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eodata.hpp"

#include "console.hpp"
#include "packet.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <type_traits>

static const char *eodata_safe_fail_filename;

std::string convert_pub_filename(const std::string& s)
{
	std::string result;

	for (const char* p = &s[0]; *p != '\0'; ++p)
	{
		if ((p - &s[0]) < s.size() - 3 && p[0] == '0' && p[1] == '0' && p[2] == '1')
		{
			result += "%03d";
			p += 2;
			continue;
		}

		if (*p == '%')
			result += '%';

		result += *p;
	}

	return result;
}

static void eodata_safe_fail(int line)
{
	Console::Err("Invalid file / failed read/seek: %s -- %i", eodata_safe_fail_filename, line);
	std::exit(1);
}

#define SAFE_SEEK(fh, offset, from) if (std::fseek(fh, offset, from) != 0) { std::fclose(fh); eodata_safe_fail(__LINE__); }
#define SAFE_READ(buf, size, count, fh) if (std::fread(buf, size, count, fh) != static_cast<std::size_t>(count)) { std::fclose(fh); eodata_safe_fail(__LINE__); }

void pub_read_record(EIF_Data& newdata, char* buf)
{
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
}


void pub_read_record(ENF_Data& newdata, char* buf)
{
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
}

void pub_read_record(ESF_Data& newdata, char* buf)
{
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
}

void pub_read_record(ECF_Data& newdata, char* buf)
{
	newdata.base = PacketProcessor::Number(buf[0]);
	newdata.type = PacketProcessor::Number(buf[1]);

	newdata.str = PacketProcessor::Number(buf[2], buf[3]);
	newdata.intl = PacketProcessor::Number(buf[4], buf[5]);
	newdata.wis = PacketProcessor::Number(buf[6], buf[7]);
	newdata.agi = PacketProcessor::Number(buf[8], buf[9]);
	newdata.con = PacketProcessor::Number(buf[10], buf[11]);
	newdata.cha = PacketProcessor::Number(buf[12], buf[13]);
}

template <class T>
int read_single_file(T& pub, Pub_File& file, bool auto_split, int version, int first_id)
{
	std::FILE *fh = std::fopen(file.filename.c_str(), "rb");
	eodata_safe_fail_filename = file.filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", file.filename.c_str());
		std::exit(1);
	}

	// Only attempt to auto-split the first file
	if (first_id != 1)
		auto_split = false;

	unsigned char namesize, shoutsize;
	std::string name, shout;
	char buf[T::DATA_SIZE] = {0};

	int readobj = 0;
	int max_entries = T::FILE_MAX_ENTRIES;

	if constexpr (std::is_same_v<T, ECF>)
	{
		if (version >= 1)
			max_entries = T::FILE_MAX_ENTRIES_V2;
	}

	if (version < 1 && auto_split == true)
		max_entries = 64000;

	SAFE_SEEK(fh, 10, SEEK_SET);
	SAFE_READ(static_cast<void *>(&namesize), sizeof(char), 1, fh);

	if constexpr (std::is_same_v<T, ESF>)
	{
		SAFE_READ(static_cast<void *>(&shoutsize), sizeof(char), 1, fh);
	}

	file.splits.reserve(4);

	for (int i = 0; i < max_entries; ++i)
	{
		if (auto_split && (i % T::FILE_MAX_ENTRIES == 0))
		{
			std::size_t split = std::size_t(std::ftell(fh)) - 1;
			int size = (int)split - (file.splits.empty() ? 0 : (int)file.splits.back());

			if (size > 63992)
				Console::Err("Auto-split file is too large (%d bytes too long): %s", size - 63992, file.filename.c_str());

			file.splits.push_back(std::size_t(std::ftell(fh)) - 1);
		}

		namesize = PacketProcessor::Number(namesize);
		name.resize(namesize);

		if (namesize > 0)
			SAFE_READ(&name[0], sizeof(char), namesize, fh);

		if constexpr (std::is_same_v<T, ESF>)
		{
			shoutsize = PacketProcessor::Number(shoutsize);
			shout.resize(shoutsize);

			if (shoutsize > 0)
				SAFE_READ(&shout[0], sizeof(char), shoutsize, fh);
		}

		SAFE_READ(buf, sizeof(char), T::DATA_SIZE, fh);

		typename T::data_t& newdata = pub.data[first_id + i];

		++readobj;

		newdata.id = first_id + i;
		newdata.name = name;

		if constexpr (std::is_same_v<T, ESF>)
		{
			newdata.shout = shout;
		}

		pub_read_record(newdata, buf);

		if (first_id + i >= pub.data.size()
		 || std::fread(static_cast<void *>(&namesize), sizeof(char), 1, fh) != 1)
		{
			break;
		}

		if constexpr (std::is_same_v<T, ESF>)
		{
			if (std::fread(static_cast<void *>(&shoutsize), sizeof(char), 1, fh) != 1)
			{
				break;
			}
		}
	}

	std::size_t last_split = std::size_t(std::ftell(fh));
	int size = (int)last_split - (int)file.splits.back();

	if (size > 63992)
	{
		if (auto_split && file.splits.size() > 1)
			Console::Err("Auto-split file is too large (%d bytes too long): %s", size - 63992, file.filename.c_str());
		else
			Console::Err("File is too large (%d bytes too long): %s", size - 63992, file.filename.c_str());
	}

	file.splits.push_back(last_split);

	std::fclose(fh);

	return readobj;
}

template <class T>
void read_pub_files(T& pub, const std::string& filename, bool auto_split)
{
	pub.files.clear();
	pub.data.clear();

	std::string filename_template = convert_pub_filename(filename);
	char fn_buf[256];
	int file_number = 1;

	std::snprintf(fn_buf, sizeof fn_buf, filename_template.c_str(), file_number);

	std::FILE *fh = std::fopen(fn_buf, "rb");
	eodata_safe_fail_filename = filename.c_str();

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		std::exit(1);
	}

	char header_buf[10];
	SAFE_READ(header_buf, sizeof(char), 10, fh);

	std::memcpy(pub.rid.data(), header_buf + 3, 4);
	std::memcpy(pub.len.data(), header_buf + 7, 2);

	int readobj = 0;
	int numobj = PacketProcessor::Number(pub.len[0], pub.len[1]);
	int version = PacketProcessor::Number(header_buf[9]);

	pub.data.resize(numobj + 1);

	while (readobj < numobj && file_number < 1000)
	{
		std::snprintf(fn_buf, sizeof fn_buf, filename_template.c_str(), file_number++);
		pub.files.push_back(Pub_File{fn_buf, {}});
		Pub_File& pub_file = pub.files.back();
		readobj += read_single_file(pub, pub_file, auto_split, version, readobj + 1);

		// Only a single file is loaded if auto-splitting occurs
		if (pub_file.splits.size() > 2)
			break;
	}

	if (version < 1 && pub.data.back().name == "eof")
		pub.data.pop_back();
}

void EIF::Read(const std::string& filename, bool auto_split)
{
	read_pub_files(*this, filename, auto_split);
	Console::Out("%i items loaded.", this->data.size()-1);
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

void ENF::Read(const std::string& filename, bool auto_split)
{
	read_pub_files(*this, filename, auto_split);
	Console::Out("%i npc types loaded.", this->data.size()-1);
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

void ESF::Read(const std::string& filename, bool auto_split)
{
	read_pub_files(*this, filename, auto_split);
	Console::Out("%i spells loaded.", this->data.size()-1);
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

void ECF::Read(const std::string& filename, bool auto_split)
{
	read_pub_files(*this, filename, auto_split);
	Console::Out("%i classes loaded.", this->data.size()-1);
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
