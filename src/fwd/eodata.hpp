
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_EODATA_HPP_INCLUDED
#define FWD_EODATA_HPP_INCLUDED

class EIF;
class ENF;
class ESF;
class ECF;

struct EIF_Data;
struct ENF_Data;
struct ESF_Data;
struct ECF_Data;

enum FileType : unsigned char
{
	FILE_MAP = 1,
	FILE_ITEM = 2,
	FILE_NPC = 3,
	FILE_SPELL = 4,
	FILE_CLASS = 5
};

#endif // FWD_EODATA_HPP_INCLUDED
