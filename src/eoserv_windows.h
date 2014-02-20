#ifndef EOSERV_WINDOWS_H_INCLUDED
#define EOSERV_WINDOWS_H_INCLUDED

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

// Include winsock2.h before windows.h to prevent winsock.h being included
#include <winsock2.h>
#include <windows.h>

// Non-lean-and-mean headers we need
#include <mmsystem.h>

// Undefine conflicting function names:

// Character::PlaySound
#ifdef PlaySound
#undef PlaySound
#endif // PlaySound

#endif // EOSERV_WINDOWS_H_INCLUDED
