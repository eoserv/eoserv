/* fwd/eodata.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_EODATA_HPP_INCLUDED
#define FWD_EODATA_HPP_INCLUDED

class EIF;
class ENF;
class ESF;
class ECF;

template <class EIF> struct EIF_Data_Base;
typedef EIF_Data_Base<EIF> EIF_Data;

template <class ENF> struct ENF_Data_Base;
typedef ENF_Data_Base<ENF> ENF_Data;

template <class ESF> struct ESF_Data_Base;
typedef ESF_Data_Base<ESF> ESF_Data;

template <class ECF> struct ECF_Data_Base;
typedef ECF_Data_Base<ECF> ECF_Data;

enum FileType : unsigned char
{
	FILE_MAP = 1,
	FILE_ITEM = 2,
	FILE_NPC = 3,
	FILE_SPELL = 4,
	FILE_CLASS = 5
};

#endif // FWD_EODATA_HPP_INCLUDED
