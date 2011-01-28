
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "scriptreg.hpp"

#ifdef NOSCRIPT

#include "console.hpp"
#include "world.hpp"

void script_register(World &world)
{
	Console::Out("Scripting disabled at compile time");
}

#else // NOSCRIPT

#include <string>

#include <angelscript/scriptmath.h>
#include <angelscript/scriptstdstring.h>
#include <angelscript/scriptdictionary.h>
#include <angelscript/scriptfile.h>

#include "container/container.hpp"
#include "container/iterator.hpp"
#include "container/ptr_vector.hpp"
#include "container/ptr_list.hpp"

#include "arena.hpp"
#include "config.hpp"
#include "console.hpp"
#include "character.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "eoserver.hpp"
#include "hash.hpp"
#include "hook.hpp"
#include "map.hpp"
#include "nanohttp.hpp"
#include "npc.hpp"
#include "packet.hpp"
#include "party.hpp"
#include "player.hpp"
#include "sln.hpp"
#include "timer.hpp"
#include "world.hpp"

#ifdef DEBUG
static bool script_DEBUG = true;
#else // DEBUG
static bool script_DEBUG = false;
#endif // DEBUG

static ScriptEngine *engine;

template <class T> static void REG()
{
	T::ScriptRegister(*engine);
}

template <class T> static void REG_T()
{
	T::ScriptRegisterType(*engine);
}

void script_register(World &world)
{
	engine = &world.hookmanager->engine;

	RegisterScriptMath(engine->as);
	RegisterStdString(engine->as);

	if (world.config["ScriptFileAccess"])
	{
		RegisterScriptFile(engine->as);
	}

	// Common things in EOSERV to be exposed to scripts
	REG_T<GenericPtrIterator>();
	REG_T<GenericPtrVector>();
	REG_T<ScriptPtrVector>();
	REG_T<GenericPtrVector>();
	REG_T<GenericPtrList>();
	REG_T<ScriptPtrList>();
	REG_T<GenericPtrList>();

	REG<GenericPtrIterator>();
	REG<GenericPtrVector>();
	REG<ScriptPtrVector>();
	REG<GenericPtrVector>();
	REG<GenericPtrList>();
	REG<ScriptPtrList>();
	REG<GenericPtrList>();

	Console::ScriptRegister(*engine);
	eoconst::ScriptRegister(*engine);
	hash::ScriptRegister(*engine);
	packet::ScriptRegister(*engine);
	util::ScriptRegister(*engine);

	// Any new script-visible classes must be added here
	REG_T<ActionQueue_Action>();
	REG_T<ActionQueue>();
	REG_T<Arena>();
	REG_T<Arena_Spawn>();
	REG_T<Board_Post>();
	REG_T<Board>();
	REG_T<Config>();
	REG_T<Character_Item>();
	REG_T<Character_Spell>();
	REG_T<Character>();
	REG_T<Database>();
	REG_T<EOClient>();
	REG_T<EOServer>();
	REG_T<EIF>();
	REG_T<EIF_Data>();
	REG_T<ENF>();
	REG_T<ENF_Data>();
	REG_T<ESF>();
	REG_T<ESF_Data>();
	REG_T<ECF>();
	REG_T<ECF_Data>();
	REG_T<Guild_Member>();
	REG_T<Guild_Create>();
	REG_T<GuildManager>();
	REG_T<Guild>();
	REG_T<HookManager>();
	REG_T<HTTP>();
	REG_T<Home>();
	REG_T<IPAddress>();
	REG_T<Map_Chest_Item>();
	REG_T<Map_Chest_Spawn>();
	REG_T<Map_Chest>();
	REG_T<Map_Item>();
	REG_T<Map_Tile>();
	REG_T<Map_Warp>();
	REG_T<Map>();
	REG_T<NPC_Drop>();
	REG_T<NPC_Opponent>();
	REG_T<NPC_Shop_Craft_Ingredient>();
	REG_T<NPC_Shop_Craft_Item>();
	REG_T<NPC_Shop_Trade_Item>();
	REG_T<NPC>();
	REG_T<PacketBuilder>();
	REG_T<PacketProcessor>();
	REG_T<PacketReader>();
	REG_T<Party>();
	REG_T<Player>();
	REG_T<SLN>();
	REG_T<Timer>();
	REG_T<TimeEvent>();
	REG_T<World>();

	REG_T<CharacterEvent>();
	REG_T<NPCEvent>();

	// Any new script-visible classes must be added here (yes, again)

	// TODO: Let enums be registered before reaching here so I can put these back in order
	REG<EIF>();
	REG<EIF_Data>();
	REG<ENF>();
	REG<ENF_Data>();
	REG<ESF>();
	REG<ESF_Data>();
	REG<ECF>();
	REG<ECF_Data>();

	//REG<ActionQueue_Action>();
	//REG<ActionQueue>();
	REG<Arena>();
	REG<Arena_Spawn>();
	REG<Board_Post>();
	REG<Board>();
	REG<Config>();
	REG<Character_Item>();
	REG<Character_Spell>();
	REG<Character>();
	REG<Database>();
	REG<EOClient>();
	REG<EOServer>();
	REG<Guild_Member>();
	REG<Guild_Create>();
	REG<GuildManager>();
	REG<Guild>();
	REG<HookManager>();
	REG<HTTP>();
	REG<Home>();
	REG<IPAddress>();
	REG<Map_Chest_Item>();
	REG<Map_Chest_Spawn>();
	REG<Map_Chest>();
	REG<Map_Item>();
	REG<Map_Tile>();
	REG<Map_Warp>();
	REG<Map>();
	REG<NPC_Drop>();
	REG<NPC_Opponent>();
	REG<NPC_Shop_Craft_Ingredient>();
	REG<NPC_Shop_Craft_Item>();
	REG<NPC_Shop_Trade_Item>();
	REG<NPC>();
	REG<PacketBuilder>();
	REG<PacketProcessor>();
	REG<PacketReader>();
	REG<Party>();
	REG<Player>();
	REG<SLN>();
	REG<Timer>();
	REG<TimeEvent>();
	REG<World>();

	REG<CharacterEvent>();
	REG<NPCEvent>();
}

#endif // NOSCRIPT
