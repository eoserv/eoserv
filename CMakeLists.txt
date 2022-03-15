# $Id$
# EOSERV is released under the zlib license.
# See LICENSE.txt for more info.

cmake_minimum_required(VERSION 3.8)
project(eoserv C CXX)

set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
set(bindir "${CMAKE_CURRENT_BINARY_DIR}")

set(CMAKE_MODULE_PATH "${srcdir}/cmake")

# ---------
#  Options
# ---------

option(EOSERV_WANT_MYSQL "Enables MariaDB/MySQL server database support." ON)
option(EOSERV_WANT_SQLITE "Enables SQLite3 embedded database support." ON)

# Requires CMake 3.16 or later
if(COMMAND target_precompile_headers)
	option(EOSERV_USE_PRECOMPILED_HEADERS "Uses a precompiled header to speed up compilation." ON)
	option(EOSERV_GEN_PRECOMPILED_HEADERS "Generate precompiled header automatically. Requires a shell with basic utilities including grep and awk." OFF)
endif()

# Note: Unity builds may be enabled by setting the CMAKE_UNITY_BUILD variable (on CMake 3.16 or later)

option(EOSERV_NO_DATA "Disables copying of data files in to build directory." OFF)

option(EOSERV_DEBUG_QUERIES "Enables printing of database queries to debug output." OFF)

# --------------
#  Source files
# --------------

include(SourceFileList)

set(sources ${eoserv_ALL_SOURCE_FILES})

# Platform-specific source files
if(WIN32)
	list(APPEND sources ${eoserv_WIN32_SOURCE_FILES})
	list(APPEND sources "src/winres.rc")
endif()

# ---------
#  Outputs
# ---------

add_executable(eoserv
	${sources}
)

# ----------------
#  Compiler flags
# ----------------

set_target_properties(eoserv PROPERTIES CXX_STANDARD 17)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(eoserv_GCC TRUE)
	set(eoserv_COMPILER_SUPPORTED TRUE)

	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7")
		set(eoserv_COMPILER_SUPPORTED FALSE)
	endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(eoserv_CLANG TRUE)
	set(eoserv_COMPILER_SUPPORTED TRUE)

	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6")
		set(eoserv_COMPILER_SUPPORTED FALSE)
	endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(eoserv_MSVC TRUE)
	set(eoserv_COMPILER_SUPPORTED TRUE)

	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.14")
		set(eoserv_COMPILER_SUPPORTED FALSE)
	endif()
endif()

# These non-standard anti-optimizations are currently required for a correctly functioning server
if(eoserv_GCC OR eoserv_CLANG)
	target_compile_options(eoserv PRIVATE -fwrapv -fno-strict-aliasing)
elseif(eoserv_MSVC)
	message(WARNING "Compiling with optimizations enabled in Visual Studio may result in a server that is unstable/exploitable.")
endif()

if(NOT eoserv_COMPILER_SUPPORTED)
	message(WARNING "You are using an unsupported compiler, the build may fail or result in an unstable/exploitable server. Detected compiler was ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}. EOSERV is only tested on GCC 7+, Clang 6+ and Visual C++ 2017 15.7 or later.")
endif()

if(EOSERV_DEBUG_QUERIES)
	target_compile_definitions(eoserv PRIVATE DATABASE_DEBUG)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_compile_definitions(eoserv PRIVATE DEBUG)
endif()

# -----------
#  Libraries
# -----------

# mariadb.org / mysql.com
# MariaDB Connector/C v2.x or v3.x are both acceptable (LGPL)
# MySQL libraries are also acceptable (GPL)
if(EOSERV_WANT_MYSQL)
	find_package(MariaDB)

	if(MARIADB_FOUND)
		target_include_directories(eoserv PRIVATE "${MARIADB_INCLUDE_DIR}")
		target_link_libraries(eoserv PRIVATE "${MARIADB_LIBRARY}")
		target_compile_definitions(eoserv PRIVATE DATABASE_MYSQL)
	else()
		message(WARNING "MariaDB/MySQL library not found, disabling support")
	endif()
endif()

# sqlite.org
# Any SQLite 3 version is acceptable
if(EOSERV_WANT_SQLITE)
	find_package(SQLite3)

	if(SQLITE3_FOUND)
		target_include_directories(eoserv PRIVATE "${SQLITE3_INCLUDE_DIR}")
		target_link_libraries(eoserv PRIVATE "${SQLITE3_LIBRARY}")
		target_compile_definitions(eoserv PRIVATE DATABASE_SQLITE)
	else()
		message(WARNING "SQLite3 library not found, disabling support")
	endif()
endif()

if(NOT EOSERV_WANT_MYSQL AND NOT EOSERV_WANT_SQLITE)
	message(FATAL_ERROR "Either MySQL or SQLite support must be enabled.")
endif()

# Platfrom-specific libraries
if(WIN32)
	target_link_libraries(eoserv PRIVATE winmm ws2_32)

	set(WINVER 0x4000)

	# Use Wspiapi.h if avaialble for pre-XP compatibility
	# (lowercase header name for compatibility with MinGW on Linux)
	if(MINGW)
		include(CheckIncludeFileCXX)
		check_include_file_cxx(wspiapi.h WSPIAPI_AVAILABLE)
		if(WSPIAPI_AVAILABLE)
			target_compile_definitions(eoserv PRIVATE WSPIAPI_AVAILABLE)
		else()
			message(WARNING "Wspiapi.h not found - compiling for Windows XP or later")
			set(WINVER 0x0501)
		endif()
	endif()

	target_compile_definitions(eoserv PRIVATE "WINVER=${WINVER}" "_WIN32_WINNT=${WINVER}")
endif()

install(TARGETS eoserv RUNTIME DESTINATION .)

foreach(File ${ExtraFiles})
	get_filename_component(Dir "${File}" DIRECTORY)

	if(Dir STREQUAL "")
		set(Dir ".")
	endif()

	install(FILES "${File}" DESTINATION "${Dir}")
endforeach()

# ---------------------
#  Precompiled Headers
# ---------------------

if(COMMAND target_precompile_headers AND EOSERV_USE_PRECOMPILED_HEADERS)
	if(EOSERV_GEN_PRECOMPILED_HEADERS)
		add_custom_command(OUTPUT "${bindir}/eoserv-pch.h"
			COMMAND "${srcdir}/autogen-pch.sh" ${eoserv_ALL_SOURCE_FILES} > "${bindir}/eoserv-pch.h"
			MAIN_DEPENDENCY "${srcdir}/autogen-pch.sh"
		)
	else()
		add_custom_command(OUTPUT "${bindir}/eoserv-pch.h"
			COMMAND "${CMAKE_COMMAND}" -E copy "${srcdir}/src/stdafx.h" "${bindir}/eoserv-pch.h"
			MAIN_DEPENDENCY src/stdafx.h
			DEPENDS src/stdafx.h
		)
	endif()

	target_precompile_headers(eoserv PRIVATE "${bindir}/eoserv-pch.h")

	# target_precompile_headers does not create a dependency itself
	# use a dummy target to force eoserv-pch.h to be generated before building
	add_custom_target(eoserv-pch
		DEPENDS "${bindir}/eoserv-pch.h"
	)

	add_dependencies(eoserv eoserv-pch)
endif()

# ------------
#  Data files
# ------------

if(NOT EOSERV_NO_DATA)
	foreach(File ${ExtraFiles})
		configure_file("${File}" "${bindir}/${File}" COPYONLY)
	endforeach()
endif()
