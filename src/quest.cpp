/* quest.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "quest.hpp"

#include "character.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "eoplus.hpp"
#include "map.hpp"
#include "packet.hpp"
#include "world.hpp"

#include "console.hpp"
#include "util.hpp"
#include "util/rpn.hpp"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <deque>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

static int quest_day()
{
	return (std::time(nullptr) / 86400) & 0x7FFF;
}

struct Validation_Error : public EOPlus::Runtime_Error
{
	private:
		std::string state_;

	public:
		Validation_Error(const std::string &what_, const std::string& state_)
			: Runtime_Error(what_)
			, state_(state_)
		{ }

		const std::string& state() const
		{
			return state_;
		}

		~Validation_Error() noexcept
		{

		}
};

static void validate_state(const EOPlus::Quest& quest, const std::string& name, const EOPlus::State& state)
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
		{"resetdaily", 0},
		{"end", 0},

		{"startquest", {1, 2}},
		{"resetquest", 1},
		{"setqueststate", 2},

		{"addnpctext", 2},
		{"addnpcinput", 3},

		{"addnpcchat", 2}, // TODO: AddNpcChat
		{"showhint", 1},
		{"quake", {0, 1}},
		{"quakeworld", {0, 1}},

		{"setmap", 3}, // Alias for SetCoord
		{"setcoord", 3},
		{"playsound", 1},
		{"giveexp", 1},
		{"giveitem", {1, 2}},
		{"removeitem", {1, 2}},
		{"setclass", 1},
		{"setrace", 1},
		{"removekarma", 1},
		{"givekarma", 1},

		{"settitle", 1},
		{"setfiance", 1},
		{"setpartner", 1},
		{"sethome", 1},

		{"setstat", 2},
		{"givestat", 2},
		{"removestat", 2},

		{"roll", 1},
	};

	static std::map<std::string, info_t> rule_argument_info{
		{"inputnpc", 1},
		{"talkedtonpc", 1},

		{"always", 0},

		{"donedaily", 1},

		{"entermap", 1},
		{"entercoord", 3},
		{"leavemap", 1},
		{"leavecoord", 3},

		{"killednpcs", {1, 2}},
		{"killedplayers", 1},

		{"gotitems", {1, 2}},
		{"lostitems", {1, 2}},
		{"useditem", {1, 2}},

		{"isgender", 1},
		{"isclass", 1},
		{"israce", 1},
		{"iswearing", 1},

		{"gotspell", {1, 2}},
		{"lostspell", 1},
		{"usedspell", {1, 2}},

		{"citizenof", 1},

		{"rolled", {1, 2}},

		// Only needed until expression support is added
		{"statis", 2},
		{"statnot", 2},
		{"statgreater", 2},
		{"statless", 2},
		{"statbetween", 3},
		{"statrpn", 1}
	};

	auto check = [&](std::string type, std::string function, const std::deque<util::variant>& args, info_t& info)
	{
		if (args.size() < std::size_t(info.min_args))
			throw Validation_Error(type + " " + function + " requires at least " + util::to_string(info.min_args) + " argument(s)", name);

		if (info.max_args != -1 && args.size() > std::size_t(info.max_args))
			throw Validation_Error(type + " " + function + " requires at most " + util::to_string(info.max_args) + " argument(s)", name);

		if (function == "setstate")
		{
			std::string state = util::lowercase(args[0]);
			auto it = quest.states.find(state);

			if (it == quest.states.end())
				throw Validation_Error("Unknown quest state: " + state, name);
		}
	};

	UTIL_FOREACH(state.actions, action)
	{
		const auto it = action_argument_info.find(action.expr.function);

		if (it == action_argument_info.end())
			throw Validation_Error("Unknown action: " + action.expr.function, name);

		check("Action", action.expr.function, action.expr.args, it->second);
	}

	UTIL_FOREACH(state.rules, rule)
	{
		const EOPlus::Action& action = rule.action;

		const auto it = action_argument_info.find(action.expr.function);

		if (it == action_argument_info.end())
			throw Validation_Error("Unknown action: " + action.expr.function, name);

		check("Action", action.expr.function, action.expr.args, it->second);
	}

	UTIL_FOREACH(state.rules, rule)
	{
		const auto it = rule_argument_info.find(rule.expr.function);

		if (it == rule_argument_info.end())
			throw Validation_Error("Unknown rule: " + rule.expr.function, name);

		check("Rule", rule.expr.function, rule.expr.args, it->second);
	}
}

static void validate_quest(const EOPlus::Quest& quest)
{
	UTIL_CIFOREACH(quest.states, it)
	{
		validate_state(quest, it->first, it->second);
	}
}

static bool modify_stat(std::string name, std::function<int(int)> f, Character* victim)
{
	bool appearance = false;
	bool level = false;
	bool stats = false;
	bool karma = false;

	bool statpoints = false;
	bool skillpoints = false;

		 if (name == "level") (level = true, victim->level) = util::clamp<int>(f(victim->level), 0, victim->world->config["MaxLevel"]);
	else if (name == "exp")   (level = true, victim->exp)   = util::clamp<int>(f(victim->exp),   0, victim->world->config["MaxEXP"]);
	else if (name == "str")   (stats = true, victim->str)   = util::clamp<int>(f(victim->str),   0, victim->world->config["MaxStat"]);
	else if (name == "int")   (stats = true, victim->intl)  = util::clamp<int>(f(victim->intl),  0, victim->world->config["MaxStat"]);
	else if (name == "wis")   (stats = true, victim->wis)   = util::clamp<int>(f(victim->wis),   0, victim->world->config["MaxStat"]);
	else if (name == "agi")   (stats = true, victim->agi)   = util::clamp<int>(f(victim->agi),   0, victim->world->config["MaxStat"]);
	else if (name == "con")   (stats = true, victim->con)   = util::clamp<int>(f(victim->con),   0, victim->world->config["MaxStat"]);
	else if (name == "cha")   (stats = true, victim->cha)   = util::clamp<int>(f(victim->cha),   0, victim->world->config["MaxStat"]);
	else if (name == "statpoints")  (statpoints = true,  victim->statpoints)  = util::clamp<int>(f(victim->statpoints),  0, int(victim->world->config["MaxLevel"]) * int(victim->world->config["StatPerLevel"]));
	else if (name == "skillpoints") (skillpoints = true, victim->skillpoints) = util::clamp<int>(f(victim->skillpoints), 0, int(victim->world->config["MaxLevel"]) * int(victim->world->config["SkillPerLevel"]));
	else if (name == "admin")
	{
		AdminLevel level = util::clamp<AdminLevel>(AdminLevel(f(victim->admin)), ADMIN_PLAYER, ADMIN_HGM);

		if (level == ADMIN_PLAYER && victim->admin != ADMIN_PLAYER)
			victim->world->DecAdminCount();
		else if (level != ADMIN_PLAYER && victim->admin == ADMIN_PLAYER)
			victim->world->IncAdminCount();

		victim->admin = level;
	}
	else if (name == "gender")    (appearance = true, victim->gender)    = util::clamp<Gender>(Gender(f(victim->gender)), Gender(0), Gender(1));
	else if (name == "hairstyle") (appearance = true, victim->hairstyle) = util::clamp<int>(f(victim->hairstyle),         0, victim->world->config["MaxHairStyle"]);
	else if (name == "haircolor") (appearance = true, victim->haircolor) = util::clamp<int>(f(victim->haircolor),         0, victim->world->config["MaxHairColor"]);
	else if (name == "race")      (appearance = true, victim->race)      = util::clamp<Skin>(Skin(f(victim->race)),       Skin(0), Skin(int(victim->world->config["MaxSkin"])));
	else if (name == "guildrank") victim->guild_rank = util::clamp<int>(f(victim->guild_rank), 0, 9);
	else if (name == "karma") (karma = true, victim->karma) = util::clamp<int>(f(victim->karma), 0, 2000);
	else if (name == "class") (stats = true, victim->clas)  = util::clamp<int>(f(victim->clas),  0, victim->world->ecf->data.size() - 1);
	else return false;

	// Easiest way to get the character to update on everyone nearby's screen
	if (appearance)
		victim->Warp(victim->map->id, victim->x, victim->y);

	// TODO: Good way of updating skillpoints
	(void)skillpoints;

	if (stats || statpoints)
	{
		victim->CalculateStats();

		PacketBuilder builder(PACKET_RECOVER, PACKET_LIST, 32);

		if (statpoints)
		{
			builder.SetID(PACKET_STATSKILL, PACKET_PLAYER);
			builder.AddShort(victim->statpoints);
		}
		else
		{
			builder.AddShort(victim->clas);
		}

		builder.AddShort(victim->display_str);
		builder.AddShort(victim->display_intl);
		builder.AddShort(victim->display_wis);
		builder.AddShort(victim->display_agi);
		builder.AddShort(victim->display_con);
		builder.AddShort(victim->display_cha);
		builder.AddShort(victim->maxhp);
		builder.AddShort(victim->maxtp);
		builder.AddShort(victim->maxsp);
		builder.AddShort(victim->maxweight);
		builder.AddShort(victim->mindam);
		builder.AddShort(victim->maxdam);
		builder.AddShort(victim->accuracy);
		builder.AddShort(victim->evade);
		builder.AddShort(victim->armor);
		victim->Send(builder);
	}

	if (karma || level)
	{
		PacketBuilder builder(PACKET_RECOVER, PACKET_REPLY, 7);
		builder.AddInt(victim->exp);
		builder.AddShort(victim->karma);
		builder.AddChar(level ? victim->level : 0);
		victim->Send(builder);
	}

	if (!stats && !skillpoints && !appearance)
	{
		victim->CheckQuestRules();
	}

	return true;
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
		validate_quest(*this->quest);
	}
	catch (EOPlus::Syntax_Error& e)
	{
		Console::Err("Could not load quest: %s", filename.c_str());
		Console::Err("Syntax Error: %s (Line %i)", e.what(), e.line());
		throw;
	}
	catch (Validation_Error& e)
	{
		Console::Err("Could not load quest: %s", filename.c_str());
		Console::Err("Validation Error: %s (State: %s)", e.what(), e.state().c_str());
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

bool Quest::Disabled() const
{
	return this->quest->info.disabled;
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

	for (auto it = this->progress.begin(); it != this->progress.end(); )
	{
		if (it->first != "d" && it->first != "c")
			it = this->progress.erase(it);
		else
			++it;
	}

	this->dialogs.clear();

	if (this->quest->Disabled())
		return;

	UTIL_FOREACH(state.actions, action)
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
	if (this->quest->Disabled())
		return false;

	std::string function_name = action.expr.function;

	if (function_name == "setstate")
	{
		std::string state = util::lowercase(action.expr.args[0]);
		this->SetState(state);
		return true;
	}
	else if (function_name == "reset")
	{
		if (this->progress.find("c") == this->progress.end())
		{
			this->character->ResetQuest(this->quest->ID());
		}
		else
		{
			this->SetState("done");
		}

		return true;
		// *this may not be valid after this point
	}
	else if (function_name == "resetdaily")
	{
		this->progress["d"] = quest_day();
		++this->progress["c"];
		this->SetState("done");
		return true;
	}
	else if (function_name == "end")
	{
		this->SetState("end");
		return true;
	}
	else if (function_name == "startquest")
	{
		short id = int(action.expr.args[0]);

		auto context = character->GetQuest(id);

		if (!context)
		{
			auto it = this->character->world->quests.find(id);

			if (it != this->character->world->quests.end())
			{
				// WARNING: holds a non-tracked reference to shared_ptr
				Quest* quest = it->second.get();
				auto context = std::make_shared<Quest_Context>(this->character, quest);
				this->character->quests[it->first] = context;
				context->SetState(action.expr.args.size() >= 2 ? std::string(action.expr.args[1]) : "begin");
			}
		}
		else if (context->StateName() == "done")
		{
			context->SetState(action.expr.args.size() >= 2 ? std::string(action.expr.args[1]) : "begin");
		}
	}
	else if (function_name == "resetquest")
	{
		short this_id = this->quest->ID();
		short id = int(action.expr.args[0]);

		auto context = this->character->GetQuest(id);

		if (context)
		{
			if (this->progress.find("c") == this->progress.end())
				context->SetState("done");
			else
				this->character->ResetQuest(id);
		}

		if (id == this_id)
		{
			return true;
			// *this is not valid after this point
		}
	}
	else if (function_name == "setqueststate")
	{
		short this_id = this->quest->ID();
		short id = int(action.expr.args[0]);
		std::string state = std::string(action.expr.args[1]);

		// WARNING: holds a non-tracked reference to shared_ptr
		Quest_Context* quest = this->character->GetQuest(id).get();

		if (quest)
		{
			quest->SetState(state);

			if (id == this_id)
				return true;
		}
	}
	else if (function_name == "showhint")
	{
		this->character->StatusMsg(action.expr.args[0]);
	}
	else if (function_name == "quake")
	{
		int strength = 5;

		if (action.expr.args.size() >= 1)
			strength = std::max(1, std::min(8, int(action.expr.args[0])));

		this->character->map->Effect(MAP_EFFECT_QUAKE, strength);
	}
	else if (function_name == "quakeworld")
	{
		int strength = 5;

		if (action.expr.args.size() >= 1)
			strength = std::max(1, std::min(8, int(action.expr.args[0])));

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
		builder.AddChar(level_up ? this->character->level : 0);

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
					builder.AddShort(character->PlayerID());
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
				builder.AddThree(amount);
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
	else if (function_name == "settitle")
	{
		this->character->title = std::string(action.expr.args[0]);
	}
	else if (function_name == "setfiance")
	{
		this->character->fiance = std::string(action.expr.args[0]);
	}
	else if (function_name == "setpartner")
	{
		this->character->partner = std::string(action.expr.args[0]);
	}
	else if (function_name == "sethome")
	{
		this->character->home = std::string(action.expr.args[0]);
	}
	else if (function_name == "setstat")
	{
		std::string stat = action.expr.args[0];
		int value = action.expr.args[1];

		if (!modify_stat(stat, [value](int) { return value; }, this->character))
			throw EOPlus::Runtime_Error("Unknown stat: " + stat);
	}
	else if (function_name == "givestat")
	{
		std::string stat = action.expr.args[0];
		int value = action.expr.args[1];

		if (!modify_stat(stat, [value](int x) { return x + value; }, this->character))
			throw EOPlus::Runtime_Error("Unknown stat: " + stat);
	}
	else if (function_name == "removestat")
	{
		std::string stat = action.expr.args[0];
		int value = action.expr.args[1];

		if (!modify_stat(stat, [value](int x) { return x - value; }, this->character))
			throw EOPlus::Runtime_Error("Unknown stat: " + stat);
	}
	else if (function_name == "roll")
	{
		this->progress["r"] = util::rand(1, int(action.expr.args[0]));
	}

	return false;
}

static bool rpn_char_eval(std::stack<util::variant>&& s, Character* character)
{
	std::unordered_map<std::string, double> formula_vars;
	character->FormulaVars(formula_vars);
	return bool(rpn_eval(s, formula_vars));
}

static bool rpn_char_eval(std::deque<util::variant>&& dq, Character* character)
{
	std::reverse(UTIL_RANGE(dq));
	std::stack<util::variant> s(std::move(dq));
	return rpn_char_eval(std::move(s), character);
}

bool Quest_Context::CheckRule(const EOPlus::Expression& expr)
{
	if (this->quest->Disabled())
		return false;

	std::string function_name = expr.function;

	if (function_name == "always")
	{
		return true;
	}
	else if (function_name == "donedaily")
	{
		if (this->progress["d"] == quest_day())
		{
			return this->progress["c"] >= int(expr.args[0]);
		}
		else
		{
			this->progress["d"] = quest_day();
			this->progress["c"] = 0;
			return false;
		}
	}
	else if (function_name == "entermap")
	{
		return this->character->map->id == int(expr.args[0]);
	}
	else if (function_name == "entercoord")
	{
		return this->character->map->id == int(expr.args[0])
		    && this->character->x == int(expr.args[1])
		    && this->character->y == int(expr.args[2]);
	}
	else if (function_name == "leavemap")
	{
		return this->character->map->id != int(expr.args[0]);
	}
	else if (function_name == "leavecoord")
	{
		return this->character->map->id != int(expr.args[0])
		    || this->character->x != int(expr.args[1])
		    || this->character->y != int(expr.args[2]);
	}
	else if (function_name == "gotitems")
	{
		return this->character->HasItem(int(expr.args[0])) >= (expr.args.size() >= 2 ? int(expr.args[1]) : 1);
	}
	else if (function_name == "lostitems")
	{
		return this->character->HasItem(int(expr.args[0])) < (expr.args.size() >= 2 ? int(expr.args[1]) : 1);
	}
	else if (function_name == "gotspell")
	{
		return this->character->HasSpell(int(expr.args[0]))
		    && (expr.args.size() < 2 || this->character->SpellLevel(int(expr.args[0])) >= int(expr.args[1]));
	}
	else if (function_name == "lostspell")
	{
		return !this->character->HasSpell(int(expr.args[0]));
	}
	else if (function_name == "isgender")
	{
		return this->character->gender == Gender(int(expr.args[0]));
	}
	else if (function_name == "isclass")
	{
		return this->character->clas == int(expr.args[0]);
	}
	else if (function_name == "israce")
	{
		return this->character->race == int(expr.args[0]);
	}
	else if (function_name == "iswearing")
	{
		return std::find(UTIL_CRANGE(this->character->paperdoll), int(expr.args[0])) != this->character->paperdoll.end();
	}
	else if (function_name == "citizenof")
	{
		return this->character->home == std::string(expr.args[0]);
	}
	else if (function_name == "rolled")
	{
		int roll = this->progress["r"];

		if (expr.args.size() < 2)
		{
			return roll == int(expr.args[0]);
		}
		else
		{
			return roll >= int(expr.args[0])
			    && roll <= int(expr.args[1]);
		}
	}
	else if (function_name == "statis")
	{
		return rpn_char_eval({expr.args[1], expr.args[0], "="}, character);
	}
	else if (function_name == "statnot")
	{
		return rpn_char_eval({expr.args[1], expr.args[0], "="}, character);
	}
	else if (function_name == "statgreater")
	{
		return rpn_char_eval({expr.args[1], expr.args[0], ">"}, character);
	}
	else if (function_name == "statless")
	{
		return rpn_char_eval({expr.args[1], expr.args[0], "<"}, character);
	}
	else if (function_name == "statbetween")
	{
		return rpn_char_eval({expr.args[1], expr.args[0], "gte", expr.args[2], expr.args[0], "lte", "&"}, character);
	}
	else if (function_name == "statrpn")
	{
		return rpn_char_eval(rpn_parse(expr.args[0]), character);
	}

	return false;
}

const Quest* Quest_Context::GetQuest() const
{
	return this->quest;
}

const Dialog* Quest_Context::GetDialog(short id) const
{
	if (this->quest->Disabled())
		return 0;

	auto it = this->dialogs.find(id);

	if (it == this->dialogs.end())
		return 0;

	// WARNING: returns a non-tracked reference to shared_ptr
	return it->second.get();
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
		if (goal->expr.function == "gotitems" || goal->expr.function == "gotspell")
		{
			icon = BOOK_ICON_ITEM;
			goal_progress = std::min<int>(goal->expr.args[1], this->character->HasItem(int(goal->expr.args[0])));
			goal_goal = goal->expr.args.size() >= 2 ? int(goal->expr.args[1]) : 1;
		}
		else if (goal->expr.function == "useditem" || goal->expr.function == "usedspell")
		{
			icon = BOOK_ICON_ITEM;
			goal_progress_key = goal->expr.function + "/" + std::string(goal->expr.args[0]);
			goal_goal = goal->expr.args.size() >= 2 ? int(goal->expr.args[1]) : 1;
		}
		else if (goal->expr.function == "killednpcs")
		{
			icon = BOOK_ICON_KILL;
			goal_progress_key = goal->expr.function + "/" + std::string(goal->expr.args[0]);
			goal_goal = goal->expr.args.size() >= 2 ? int(goal->expr.args[1]) : 1;
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
	if (this->quest->Disabled())
		return false;

	return this->TriggerRule("inputnpc", [link_id](const std::deque<util::variant>& args) { return int(args[0]) == link_id; });
}

bool Quest_Context::TalkedNPC(char vendor_id)
{
	if (this->quest->Disabled())
		return false;

	return this->TriggerRule("talkedtonpc", [vendor_id](const std::deque<util::variant>& args) { return int(args[0]) == vendor_id; });
}

void Quest_Context::UsedItem(short id)
{
	if (this->quest->Disabled())
		return;

	bool check = this->QueryRule("useditem", [id](const std::deque<util::variant>& args) { return int(args[0]) == id; });
	short amount = 0;

	if (check)
		amount = ++this->progress["useditem/" + util::to_string(id)];

	if (this->TriggerRule("useditem", [id, amount](const std::deque<util::variant>& args) { return int(args[0]) == id && amount >= int(args[1]); }))
		this->progress.erase("useditem/" + util::to_string(id));
}

void Quest_Context::UsedSpell(short id)
{
	if (this->quest->Disabled())
		return;

	bool check = this->QueryRule("usedspell", [id](const std::deque<util::variant>& args) { return int(args[0]) == id; });
	short amount = 0;

	if (check)
		amount = ++this->progress["usedspell/" + util::to_string(id)];

	if (this->TriggerRule("usedspell", [id, amount](const std::deque<util::variant>& args) { return int(args[0]) == id && amount >= int(args[1]); }))
		this->progress.erase("usedspell/" + util::to_string(id));
}

void Quest_Context::KilledNPC(short id)
{
	if (this->quest->Disabled())
		return;

	bool check = this->QueryRule("killednpcs", [id](const std::deque<util::variant>& args) { return int(args[0]) == id; });
	short amount = 0;

	if (check)
		amount = ++this->progress["killednpcs/" + util::to_string(id)];

	if (this->TriggerRule("killednpcs", [id, amount](const std::deque<util::variant>& args) { return int(args[0]) == id && amount >= int(args[1]); }))
		this->progress.erase("killednpcs/" + util::to_string(id));
}

void Quest_Context::KilledPlayer()
{
	if (this->quest->Disabled())
		return;

	bool check = this->QueryRule("killedplayers");
	short amount = 0;

	if (check)
		amount = ++this->progress["killedplayers"];

	if (this->TriggerRule("killedplayers", [amount](const std::deque<util::variant>& args) { return amount >= int(args[0]); }))
		this->progress.erase("killedplayers");
}

bool Quest_Context::IsHidden() const
{
	return this->GetQuest()->GetQuest()->info.hidden == EOPlus::Info::Hidden
		|| (this->GetQuest()->GetQuest()->info.hidden == EOPlus::Info::HiddenEnd && this->StateName() == "end")
	    || this->StateName() == "done";
}

Quest_Context::~Quest_Context()
{

}
