/* handlers/handlers.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef HANDLERS_HPP_INCLUDED
#define HANDLERS_HPP_INCLUDED

#include "../fwd/character.hpp"
#include "../fwd/eoclient.hpp"
#include "../fwd/player.hpp"
#include "../packet.hpp"

#include <array>

#define PACKET_HANDLER_PASTE_AUX2(base, id) base##id
#define PACKET_HANDLER_PASTE_AUX(base, id) PACKET_HANDLER_PASTE_AUX2(base, id)

#define PACKET_HANDLER_REGISTER(family) \
namespace { struct PACKET_HANDLER_PASTE_AUX(packet_handler_register_helper_, family) : public packet_handler_register_helper<family> \
{ \
	PACKET_HANDLER_PASTE_AUX(packet_handler_register_helper_, family)() \
	{ \
		packet_handler_register_init _init; \

#define PACKET_HANDLER_REGISTER_END(family) ; \
	} \
} PACKET_HANDLER_PASTE_AUX(packet_handler_register_helper_instance_, family); }

namespace Handlers
{

typedef void (*void_fn_t)();
typedef void (*client_handler_t)(EOClient *, PacketReader &);
typedef void (*player_handler_t)(Player *, PacketReader &);
typedef void (*character_handler_t)(Character *, PacketReader &);

enum AllowState
{
	None = 0,

	// Packet encryption initialized
	Uninitialized = 1,
	Menu = 2,

	// player != NULL
	Character_Menu = 4,

	// character != NULL
	Logging_In = 8,
	Playing = 16,

	// Flags (for Playing state handlers)
	OutOfBand = 32, // Does not queue packet

	Any = 0xFFFF
};

class packet_handler
{
	public:
		enum FunctionType
		{
			Invalid = 0,
			ClientFn,
			PlayerFn,
			CharacterFn
		};

		FunctionType fn_type;
		unsigned short allow_states;
		double delay;
		void (*f)();

		packet_handler(FunctionType fn_type = Invalid, void_fn_t f = 0, unsigned short allow_states = 0, double delay = 0.0)
			: fn_type(fn_type)
			, allow_states(allow_states)
			, delay(delay)
			, f(f)
		{ }

		operator bool() const
		{
			return fn_type != Invalid;
		}
};

class packet_handler_register
{
	private:
		std::array<std::array<packet_handler, 256>, 256> handlers;

	public:
		void Register(PacketFamily family, PacketAction action, packet_handler handler);

		static bool StateCheck(EOClient *client, unsigned short allow_states);

		void Handle(PacketFamily family, PacketAction action, EOClient *client, PacketReader &reader, bool from_queue = false) const;

		void SetDelay(PacketFamily family, PacketAction action, double delay);
};

extern packet_handler_register *packet_handler_register_instance;

class packet_handler_register_init
{
	private:
		bool master;

	public:
		packet_handler_register_init(bool master = false)
			: master(master)
		{
			static bool initialized;

			if (!initialized)
			{
				initialized = true;
				init();
			}
		}

		void init() const;

		~packet_handler_register_init();
};

template <PacketFamily family> class packet_handler_register_helper
{
	public:
		void Register(PacketAction action, client_handler_t f, unsigned short allow_states, double delay = 0.0) const
		{
			packet_handler_register_instance->Register(family, action, packet_handler(packet_handler::ClientFn, reinterpret_cast<void_fn_t>(f), allow_states, delay));
		}

		void Register(PacketAction action, player_handler_t f, unsigned short allow_states, double delay = 0.0) const
		{
			packet_handler_register_instance->Register(family, action, packet_handler(packet_handler::PlayerFn, reinterpret_cast<void_fn_t>(f), allow_states, delay));
		}

		void Register(PacketAction action, character_handler_t f, unsigned short allow_states, double delay = 0.0) const
		{
			packet_handler_register_instance->Register(family, action, packet_handler(packet_handler::CharacterFn, reinterpret_cast<void_fn_t>(f), allow_states, delay));
		}
};

inline void Handle(PacketFamily family, PacketAction action, EOClient *client, PacketReader &reader, bool from_queue = false)
{
	packet_handler_register_instance->Handle(family, action, client, reader, from_queue);
}

inline void SetDelay(PacketFamily family, PacketAction action, double delay)
{
	packet_handler_register_instance->SetDelay(family, action, delay);
}

}

#endif // HANDLERS_HPP_INCLUDED
