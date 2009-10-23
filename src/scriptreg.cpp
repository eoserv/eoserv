
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "scriptreg.hpp"

#include "console.hpp"
#include "world.hpp"
#include "hook.hpp"

void script_register(World &world)
{
	ScriptEngine &engine = world.hookmanager->engine;

	if (world.config["ScriptFileAccess"])
	{
		RegisterScriptFile(engine.as);
	}

	if (world.config["ScriptLibCAccess"])
	{
		RegisterScriptLibC(engine.as);
	}

	// Common things in EOSERV to be exposed to scripts
	//Console::ScriptRegister(engine);

	// Any new script-visible classes must be added here
	Board_Post::ScriptRegisterType(engine);
	Board::ScriptRegisterType(engine);
	HookManager::ScriptRegisterType(engine);

	// Any STL types used must be added here
	RegisterScriptList<Board_Post *>("_list_Board_Post_ptr", "Board_Post_ptr", engine.as);

	// Any new script-visible classes must be added here (yes, again)
	Board_Post::ScriptRegister(engine);
	Board::ScriptRegister(engine);
	HookManager::ScriptRegister(engine);
}
