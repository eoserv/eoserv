# SourceFileList.cmake
# EOSERV is released under the zlib license.
# See LICENSE.txt for more info.

set(eoserv_UNITY_SOURCE_FILES
	tu/main.cpp
	tu/system.cpp
	tu/game_1.cpp
	tu/game_2.cpp
	tu/game_3.cpp
	tu/handlers.cpp
	tu/commands.cpp
	tu/eoplus.cpp
	tu/sha256.c
)

set(eoserv_ALL_SOURCE_FILES
	src/arena.cpp
	src/arena.hpp
	src/character.cpp
	src/character.hpp
	src/command_source.cpp
	src/command_source.hpp
	src/commands/admin.cpp
	src/commands/char_mod.cpp
	src/commands/commands.cpp
	src/commands/commands.hpp
	src/commands/debug.cpp
	src/commands/info.cpp
	src/commands/map.cpp
	src/commands/moderation.cpp
	src/commands/server.cpp
	src/commands/warp.cpp
	src/config.cpp
	src/config.hpp
	src/console.cpp
	src/console.hpp
	src/database.cpp
	src/database.hpp
	src/database_impl.hpp
	src/dialog.cpp
	src/dialog.hpp
	src/eoclient.cpp
	src/eoclient.hpp
	src/eodata.cpp
	src/eodata.hpp
	src/eoplus/context.cpp
	src/eoplus/context.hpp
	src/eoplus.cpp
	src/eoplus/fwd/context.hpp
	src/eoplus/fwd/lex.hpp
	src/eoplus/fwd/parse.hpp
	src/eoplus.hpp
	src/eoplus/lex.cpp
	src/eoplus/lex.hpp
	src/eoplus/parse.cpp
	src/eoplus/parse.hpp
	src/eoserv_config.cpp
	src/eoserv_config.hpp
	src/eoserver.cpp
	src/eoserver.hpp
	src/extra/seose_compat.cpp
	src/extra/seose_compat.hpp
	src/fwd/arena.hpp
	src/fwd/character.hpp
	src/fwd/command_source.hpp
	src/fwd/config.hpp
	src/fwd/console.hpp
	src/fwd/database.hpp
	src/fwd/dialog.hpp
	src/fwd/eoclient.hpp
	src/fwd/eodata.hpp
	src/fwd/eoplus.hpp
	src/fwd/eoserver.hpp
	src/fwd/guild.hpp
	src/fwd/hook.hpp
	src/fwd/i18n.hpp
	src/fwd/map.hpp
	src/fwd/nanohttp.hpp
	src/fwd/npc.hpp
	src/fwd/npc_data.hpp
	src/fwd/packet.hpp
	src/fwd/party.hpp
	src/fwd/player.hpp
	src/fwd/quest.hpp
	src/fwd/socket.hpp
	src/fwd/timer.hpp
	src/fwd/world.hpp
	src/guild.cpp
	src/guild.hpp
	src/handlers/Account.cpp
	src/handlers/AdminInteract.cpp
	src/handlers/Attack.cpp
	src/handlers/Bank.cpp
	src/handlers/Barber.cpp
	src/handlers/Board.cpp
	src/handlers/Book.cpp
	src/handlers/Chair.cpp
	src/handlers/Character.cpp
	src/handlers/Chest.cpp
	src/handlers/Citizen.cpp
	src/handlers/Connection.cpp
	src/handlers/Door.cpp
	src/handlers/Emote.cpp
	src/handlers/Face.cpp
	src/handlers/Global.cpp
	src/handlers/Guild.cpp
	src/handlers/handlers.cpp
	src/handlers/handlers.hpp
	src/handlers/Init.cpp
	src/handlers/Internal.cpp
	src/handlers/Item.cpp
	src/handlers/Jukebox.cpp
	src/handlers/Locker.cpp
	src/handlers/Login.cpp
	src/handlers/Message.cpp
	src/handlers/Paperdoll.cpp
	src/handlers/Party.cpp
	src/handlers/Players.cpp
	src/handlers/Quest.cpp
	src/handlers/Refresh.cpp
	src/handlers/Shop.cpp
	src/handlers/Sit.cpp
	src/handlers/Spell.cpp
	src/handlers/StatSkill.cpp
	src/handlers/Talk.cpp
	src/handlers/Trade.cpp
	src/handlers/Walk.cpp
	src/handlers/Warp.cpp
	src/handlers/Welcome.cpp
	src/hash.cpp
	src/hash.hpp
	src/i18n.cpp
	src/i18n.hpp
	src/main.cpp
	src/map.cpp
	src/map.hpp
	src/nanohttp.cpp
	src/nanohttp.hpp
	src/npc.cpp
	src/npc.hpp
	src/npc_data.cpp
	src/npc_data.hpp
	src/packet.cpp
	src/packet.hpp
	src/party.cpp
	src/party.hpp
	src/platform.h
	src/player.cpp
	src/player.hpp
	src/quest.cpp
	src/quest.hpp
	src/sha256.c
	src/sha256.h
	src/socket.cpp
	src/socket.hpp
	src/socket_impl.hpp
	src/timer.cpp
	src/timer.hpp
	src/util.cpp
	src/util.hpp
	src/util/rpn.cpp
	src/util/rpn.hpp
	src/util/secure_string.hpp
	src/util/variant.cpp
	src/util/variant.hpp
	src/version.h
	src/world.cpp
	src/world.hpp
)

set(eoserv_WIN32_SOURCE_FILES
	src/eoserv_windows.h
	src/extra/ntservice.cpp
	src/extra/ntservice.hpp
)

# ----------

set(ConfigFiles
	admin.ini
	config.ini
	
	config/boards.ini
	config/console.ini
	config/database.ini
	config/extra.ini
	config/files.ini
	config/guilds.ini
	config/limits.ini
	config/misc.ini
	config/npc.ini
	config/pk.ini
	config/rates.ini
	config/server.ini
	
	config/database/mysql.ini
	config/database/sqlite.ini
)

set(LangFiles
	lang/en.ini
	lang/nl.ini
	lang/pl.ini
)

set(DataFiles
	data/arenas.ini
	data/drops.ini
	data/formulas.ini
	data/home.ini
	data/news.txt
	data/shops.ini
	data/skills.ini
)

# No default set of pub files is available yet.
set(PubFiles
#	data/pub/dat001.eif
#	data/pub/dtn001.enf
#	data/pub/dsl001.esf
#	data/pub/dat001.ecf
)

# No default set of map files is available yet.
set(MapFiles
#	data/maps/00001.emf
)

# No default set of quest files is available yet.
set(QuestFiles
)

set(ExtraFiles
	LICENSE.txt
	install.sql
	upgrade/0.5.2_to_0.5.3.sql
	upgrade/0.6.2_to_0.7.0.sql

	${ConfigFiles}
	${LangFiles}
	${DataFiles}
	${PubFiles}
	${MapFiles}
	${QuestFiles}
)
