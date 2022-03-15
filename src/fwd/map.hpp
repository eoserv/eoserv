/* fwd/map.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_MAP_HPP_INCLUDED
#define FWD_MAP_HPP_INCLUDED

class Map;

struct Map_Item;
struct Map_Warp;
struct Map_Tile;
struct Map_Chest_Item;
struct Map_Chest_Spawn;
struct Map_Chest;

enum MapEffect : unsigned char
{
	MAP_EFFECT_QUAKE = 1
};

#endif // FWD_MAP_HPP_INCLUDED
