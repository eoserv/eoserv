
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "quest.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>

#include "util.hpp"

#include "character.hpp"
#include "config.hpp"
#include "console.hpp"
#include "dialog.hpp"
#include "eoplus.hpp"
#include "map.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "world.hpp"

static void validate_state(const EOPlus::State& state)
{
	struct info_t
	{
		int min_args;
		int max_args;

		info_t(int min_args, int max_args = 0)
			: min_args(min_args)
			, max_args(max_args < 0 ? max_args : std::max(min_args, max_args))
		{ }
	};

	static std::map<std::string, info_t> action_argument_info{
		{"setstate", 1},
		{"reset", 0},
		{"end", 0},

		{"addnpctext", 2},
		{"addnpcinput", 3},

		{"addnpcchat", 2}, // TODO: AddNpcChat
		{"showhint", 1},
		{"quake", {0, 1}},
		{"quakeworld", {0, 1}},

		{"setmap", 3},
		{"setcoord", 3},
		{"playsound", 1},
		{"giveexp", 1},
		{"giveitem", {1, 2}},
		{"removeitem", {1, 2}},
		{"setclass", 1},
		{"setrace", 1},
		{"removekarma", 1},
		{"givekarma", 1}
	};

	static std::map<std::string, info_t> rule_argument_info{
		{"inputnpc", 1},
		{"talkedtonpc", 1},

		{"always", 1},

		{"entermap", 1},
		{"entercoord", 3},
		{"leavemap", 1},
		{"leavecoord", 3},

		{"killednpcs", {1, 2}},
		{"killedplayers", 1},

		{"gotitems", {1, 2}},
		{"lostitems", {1, 2}},
		{"useditem", {1, 2}}
	};

	auto check = [](std::string type, std::string function, std::size_t args, info_t& info)
	{
		if (args < std::size_t(info.min_args))
			throw EOPlus::Runtime_Error(type + " " + function + " requires at least " + util::to_string(info.min_args) + " argument(s)");

		if (info.max_args != -1 && args > std::size_t(info.max_args))
			throw EOPlus::Runtime_Error(type + " " + function + " requires at most " + util::to_string(info.max_args) + " argument(s)");
	};

	UTIL_CFOREACH(state.actions, action)
	{
		const auto it = action_argument_info.find(action.expr.function);

		if (it == action_argument_info.end())
			throw EOPlus::Runtime_Error("Unknown action: " + action.expr.function);

		check("Action", action.expr.function, action.expr.args.size(), it->second);
	}

	UTIL_CFOREACH(state.rules, rule)
	{
		const EOPlus::Action& action = rule.action;

		const auto it = action_argument_info.find(action.expr.function);

		if (it == action_argument_info.end())
			throw EOPlus::Runtime_Error("Unknown action: " + action.expr.function);

		check("Action", action.expr.function, action.expr.args.size(), it->second);
	}

	UTIL_CFOREACH(state.rules, rule)
	{
		const auto it = rule_argument_info.find(rule.expr.function);

		if (it == rule_argument_info.end())
			throw EOPlus::Runtime_Error("Unknown rule: " + rule.expr.function);

		check("Rule", rule.expr.function, rule.expr.args.size(), it->second);
	}
}

Quest::Quest(short id, World* world)
	: world(world)
	, quest(0)
	, id(id)
{
	this->Load();
}

void Quest::Load()
{
	char namebuf[6];

	std::string filename = this->world->config["QuestDir"];
	std::sprintf(namebuf, "%05i", this->id);
	filename += namebuf;
	filename += ".eqf";

	std::ifstream f(filename);

	if (!f)
		throw std::runtime_error("Failed to load quest");

	try
	{
		this->quest = new EOPlus::Quest(f);
	}
	catch (EOPlus::Syntax_Error& e)
	{
		Console::Err("Could not load quest: %s", filename.c_str());
		Console::Err("Syntax Error: %s (Line %i)", e.what(), e.line());
		throw;
	}
	catch (...)
	{
		Console::Err("Could not load quest: %s", filename.c_str());
		throw;
	}
}

short Quest::ID() const
{
	return this->id;
}

std::string Quest::Name() const
{
	return this->quest->info.name;
}

Quest::~Quest()
{
	if (this->quest)
		delete this->quest;
}

Quest_Context::Quest_Context(Character* character, const Quest* quest)
	: Context(quest->GetQuest())
	, character(character)
	, quest(quest)
{ }

void Quest_Context::BeginState(const std::string& name, const EOPlus::State& state)
{
	this->state_desc = state.desc;
	this->state_name = name;

	validate_state(state);

	this->progress.clear();
	this->dialogs.clear();

	UTIL_CFOREACH(state.actions, action)
	{
		std::string function_name = action.expr.function;

		if (function_name == "addnpctext" || function_name == "addnpcinput")
		{
			short vendor_id = int(action.expr.args[0]);
			auto it = this->dialogs.find(vendor_id);

			if (it == this->dialogs.end())
				it = this->dialogs.insert(std::make_pair(vendor_id, std::shared_ptr<Dialog>(new Dialog()))).first;

			if (function_name == "addnpctext")
				it->second->AddPage(std::string(action.expr.args[1]));
			else if (function_name == "addnpcinput")
				it->second->AddLink(int(action.expr.args[1]), std::string(action.expr.args[2]));
		}
	}
}

bool Quest_Context::DoAction(const EOPlus::Action& action)
{
	std::string function_name = action.expr.function;

	if (function_name == "setstate")
	{
		this->SetState(action.expr.args[0]);
		return true;
	}
	else if (function_name == "reset")
	{
		this->character->ResetQuest(this->quest->ID());
		return true;
		// *this is not valid after this point
	}
	else if (function_name == "end")
	{
		this->SetState("end");
		return true;
	}
	else if (function_name == "showhint")
	{
		this->character->StatusMsg(action.expr.args[0]);
	}
	else if (function_name == "quake")
	{
		int strength = 5;

		if (action.expr.args.size() >= 1)
			strength = std::max(1, std::min(8, int(action.expr.args[1])));

		this->character->map->Effect(MAP_EFFECT_QUAKE, strength);
	}
	else if (function_name == "quakeworld")
	{
		int strength = 5;

		if (action.expr.args.size() >= 1)
			strength = std::max(1, std::min(8, int(action.expr.args[1])));

		UTIL_FOREACH(this->character->world->maps, map)
		{
			if (map->exists)
				map->Effect(MAP_EFFECT_QUAKE, strength);
		}
	}
	else if (function_name == "setmap" || function_name == "setcoord")
	{
		this->character->Warp(int(action.expr.args[0]), int(action.expr.args[1]), int(action.expr.args[2]));
	}
	else if (function_name == "playsound")
	{
		this->character->PlaySound(int(action.expr.args[0]));
	}
	else if (function_name == "giveexp")
	{
		bool level_up = false;

		this->character->exp += int(action.expr.args[0]);

		this->character->exp = std::min(this->character->exp, int(this->character->map->world->config["MaxExp"]));

		while (this->character->level < int(this->character->map->world->config["MaxLevel"])
		 && this->character->exp >= this->character->map->world->exp_table[this->character->level+1])
		{
			level_up = true;
			++this->character->level;
			this->character->statpoints += int(this->character->map->world->config["StatPerLevel"]);
			this->character->skillpoints += int(this->character->map->world->config["SkillPerLevel"]);
			this->character->CalculateStats();
		}

		PacketBuilder builder(PACKET_RECOVER, PACKET_REPLY, 11);
		builder.AddInt(this->character->exp);
		builder.AddShort(this->character->karma);
		builder.AddChar(this->character->level);

		if (level_up)
		{
			builder.AddShort(this->character->statpoints);
			builder.AddShort(this->character->skillpoints);
		}

		this->character->Send(builder);

		if (level_up)
		{
			UTIL_FOREACH(this->character->map->characters, character)
			{
				if (character != this->character && this->character->InRange(character))
				{
					PacketBuilder builder(PACKET_ITEM, PACKET_ACCEPT, 2);
					builder.AddShort(character->player->id);
					character->Send(builder);
				}
			}
		}
	}
	else if (function_name == "giveitem")
	{
		int id = int(action.expr.args[0]);
		int amount = (action.expr.args.size() >= 2) ? int(action.expr.args[1]) : 1;

		if (this->character->AddItem(id, amount))
		{
			if (id == 1)
			{
				PacketBuilder builder(PACKET_ITEM, PACKET_GET, 9);
				builder.AddShort(0); // UID
				builder.AddShort(id);
				builder.AddThree(amount);
				builder.AddChar(this->character->weight);
				builder.AddChar(this->character->maxweight);
				this->character->Send(builder);
			}
			else
			{
				PacketBuilder builder(PACKET_ITEM, PACKET_OBTAIN, 6);
				builder.AddShort(id);
				builder.AddThree(this->character->HasItem(id));
				builder.AddChar(this->character->weight);
				this->character->Send(builder);
			}
		}
	}
	else if (function_name == "removeitem")
	{
		int id = int(action.expr.args[0]);
		int amount = (action.expr.args.size() >= 2) ? int(action.expr.args[1]) : 1;

		if (this->character->DelItem(id, amount))
		{
			PacketBuilder builder(PACKET_ITEM, PACKET_KICK, 7);
			builder.AddShort(id);
			builder.AddInt(this->character->HasItem(id));
			builder.AddChar(this->character->weight);
			this->character->Send(builder);
		}
	}
	else if (function_name == "setclass")
	{
		this->character->clas = int(action.expr.args[0]);

		this->character->CalculateStats();

		PacketBuilder builder(PACKET_RECOVER, PACKET_LIST, 32);

		builder.AddShort(this->character->clas);
		builder.AddShort(this->character->display_str);
		builder.AddShort(this->character->display_intl);
		builder.AddShort(this->character->display_wis);
		builder.AddShort(this->character->display_agi);
		builder.AddShort(this->character->display_con);
		builder.AddShort(this->character->display_cha);
		builder.AddShort(this->character->maxhp);
		builder.AddShort(this->character->maxtp);
		builder.AddShort(this->character->maxsp);
		builder.AddShort(this->character->maxweight);
		builder.AddShort(this->character->mindam);
		builder.AddShort(this->character->maxdam);
		builder.AddShort(this->character->accuracy);
		builder.AddShort(this->character->evade);
		builder.AddShort(this->character->armor);

		this->character->Send(builder);
	}
	else if (function_name == "setrace")
	{
		this->character->race = Skin(int(action.expr.args[0]));
		this->character->Warp(this->character->map->id, this->character->x, this->character->y);
	}
	else if (function_name == "removekarma")
	{
		this->character->karma -= int(action.expr.args[0]);

		if (this->character->karma < 0)
			this->character->karma = 0;

		PacketBuilder builder(PACKET_RECOVER, PACKET_REPLY, 7);
		builder.AddInt(this->character->exp);
		builder.AddShort(this->character->karma);
		builder.AddChar(0);
		this->character->Send(builder);
	}
	else if (function_name == "givekarma")
	{
		this->character->karma += int(action.expr.args[0]);

		if (this->character->karma > 2000)
			this->character->karma = 2000;

		PacketBuilder builder(PACKET_RECOVER, PACKET_REPLY, 7);
		builder.AddInt(this->character->exp);
		builder.AddShort(this->character->karma);
		builder.AddChar(0);
		this->character->Send(builder);
	}

	return false;
}

bool Quest_Context::CheckRule(const EOPlus::Rule& rule)
{
	std::string function_name = rule.expr.function;

	if (function_name == "always")
	{
		return true;
	}
	else if (function_name == "entermap")
	{
		return this->character->map->id == int(rule.expr.args[0]);
	}
	else if (function_name == "entercoord")
	{
		return this->character->map->id == int(rule.expr.args[0])
		    && this->character->x == int(rule.expr.args[1])
		    && this->character->y == int(rule.expr.args[2]);
	}
	else if (function_name == "gotitems")
	{
		return this->character->HasItem(int(rule.expr.args[0])) >= int(rule.expr.args[1]);
	}
	else if (function_name == "lostitems")
	{
		return this->character->HasItem(int(rule.expr.args[0])) < int(rule.expr.args[1]);
	}

	return false;
}

const Quest* Quest_Context::GetQuest() const
{
	return this->quest;
}

const Dialog* Quest_Context::GetDialog(short id) const
{
	auto it = this->dialogs.find(id);

	if (it == this->dialogs.end())
		return 0;

	// WARNING: returns a non-tracked reference to shared_ptr
	return it->second.get();
}

std::string Quest_Context::StateName() const
{
	return this->state_name;
}

std::string Quest_Context::Desc() const
{
	return this->state_desc;
}

Quest_Context::ProgressInfo Quest_Context::Progress() const
{
	BookIcon icon = BOOK_ICON_TALK;
	std::string goal_progress_key;
	short goal_progress = 0;
	short goal_goal = 0;

	const EOPlus::Rule* goal = this->GetGoal();

	if (goal)
	{
		if (goal->expr.function == "gotitems")
		{
			icon = BOOK_ICON_ITEM;
			goal_progress = std::min<int>(goal->expr.args[1], this->character->HasItem(int(goal->expr.args[0])));
			goal_goal = int(goal->expr.args[1]);
		}
		else if (goal->expr.function == "useditem")
		{
			icon = BOOK_ICON_ITEM;
			goal_progress_key = goal->expr.function + "/" + std::string(goal->expr.args[0]);
			goal_goal = int(goal->expr.args[1]);
		}
		else if (goal->expr.function == "killednpcs")
		{
			icon = BOOK_ICON_KILL;
			goal_progress_key = goal->expr.function + "/" + std::string(goal->expr.args[0]);
			goal_goal = int(goal->expr.args[1]);
		}
		else if (goal->expr.function == "killedplayers")
		{
			icon = BOOK_ICON_KILL;
			goal_progress_key = goal->expr.function;
			goal_goal = int(goal->expr.args[0]);
		}
		else if (goal->expr.function == "entercoord" || goal->expr.function == "leavecoord" || goal->expr.function == "entermap" || goal->expr.function == "leavemap")
		{
			icon = BOOK_ICON_STEP;
		}
	}

	if (!goal_progress_key.empty())
	{
		auto it = this->progress.find(goal_progress_key);

		if (it != this->progress.end())
			goal_progress = it->second;
	}

	return ProgressInfo{icon, goal_progress, goal_goal};
}

std::string Quest_Context::SerializeProgress() const
{
	std::string serialized = "{";

	UTIL_CIFOREACH(this->progress, it)
	{
		serialized += it->first;
		serialized += "=";
		serialized += util::to_string(it->second);

		if (it != std::prev(this->progress.cend()))
		{
			serialized += ",";
		}
	}

	serialized += "}";

	return serialized;
}

std::string::const_iterator Quest_Context::UnserializeProgress(std::string::const_iterator it, std::string::const_iterator end)
{
	if (it == end || *it != '{')
		return it;

	++it;

	std::string key;
	short value = 0;
	int state = 0;

	for (; it != end && *it != '}'; ++it)
	{
		if (state == 0) // Reading key
		{
			if (*it == '=')
			{
				state = 1;
			}
			else
			{
				key += *it;
			}
		}
		else if (state == 1) // Reading value
		{
			if (*it == ',')
			{
				this->progress[key] = value;
				key = "";
				value = 0;
				state = 0;
			}
			else
			{
				value *= 10;
				value += *it - '0';
			}
		}
	}

	if (state == 1)
		this->progress[key] = value;

	if (it != end)
		++it;

	return it;
}

bool Quest_Context::DialogInput(char link_id)
{
	return this->TriggerRule("inputnpc", [link_id](const std::deque<util::variant>& args) { return int(args[0]) == link_id; });
}

bool Quest_Context::TalkedNPC(char vendor_id)
{
	return this->TriggerRule("talkedtonpc", [vendor_id](const std::deque<util::variant>& args) { return int(args[0]) == vendor_id; });
}

void Quest_Context::GotItems(short id, int amount)
{
	this->TriggerRule("gotitems", [id, amount](const std::deque<util::variant>& args) { return int(args[0]) == id && amount >= int(args[1]); });
}

void Quest_Context::LostItems(short id, int amount)
{
	this->TriggerRule("lostitems", [id, amount](const std::deque<util::variant>& args) { return int(args[0]) == id && amount < int(args[1]); });
}

void Quest_Context::UsedItem(short id)
{
	bool check = this->QueryRule("useditem", [id](const std::deque<util::variant>& args) { return int(args[0]) == id; });
	short amount = 0;

	if (check)
		amount = ++this->progress["useditem/" + util::to_string(id)];

	if (this->TriggerRule("useditem", [id, amount](const std::deque<util::variant>& args) { return int(args[0]) == id && amount >= int(args[1]); }))
		this->progress.erase("useditem/" + util::to_string(id));
}

void Quest_Context::KilledNPC(short id)
{
	bool check = this->QueryRule("killednpcs", [id](const std::deque<util::variant>& args) { return int(args[0]) == id; });
	short amount = 0;

	if (check)
		amount = ++this->progress["killednpcs/" + util::to_string(id)];

	if (this->TriggerRule("killednpcs", [id, amount](const std::deque<util::variant>& args) { return int(args[0]) == id && amount >= int(args[1]); }))
	{
		this->progress.erase("killednpcs/" + util::to_string(id));
	}
}

void Quest_Context::KilledPlayer()
{
	bool check = this->QueryRule("killedplayers");
	short amount = 0;

	if (check)
		amount = ++this->progress["killedplayers"];

	if (this->TriggerRule("killedplayers", [amount](const std::deque<util::variant>& args) { return amount >= int(args[0]); }))
		this->progress.erase("killedplayers");
}

void Quest_Context::EnterMap(short map)
{
	this->TriggerRule("entermap", [map](const std::deque<util::variant>& args) { return int(args[0]) == map; });
}

void Quest_Context::LeaveMap(short map)
{
	this->TriggerRule("leavemap", [map](const std::deque<util::variant>& args) { return int(args[0]) == map; });
}

void Quest_Context::EnterCoord(short map, unsigned char x, unsigned char y)
{
	this->TriggerRule("entercoord", [map, x, y](const std::deque<util::variant>& args) { return int(args[0]) == map && int(args[1]) == x && int(args[2]) == y; });
}

void Quest_Context::LeaveCoord(short map, unsigned char x, unsigned char y)
{
	this->TriggerRule("leavecoord", [map, x, y](const std::deque<util::variant>& args) { return int(args[0]) == map && int(args[1]) == x && int(args[2]) == y; });
}

Quest_Context::~Quest_Context()
{

}