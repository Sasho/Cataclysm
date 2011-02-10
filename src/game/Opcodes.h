/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/// \addtogroup u2w
/// @{
/// \file

#ifndef _OPCODES_H
#define _OPCODES_H

#include "Common.h"

// Note: this include need for be sure have full definition of class WorldSession
//       if this class definition not complite then VS for x64 release use different size for
//       struct OpcodeHandler in this header and Opcode.cpp and get totally wrong data from
//       table opcodeTable in source when Opcode.h included but WorldSession.h not included
#include "WorldSession.h"

/// List of Opcodes
enum Opcodes
{
    UNKNOWN_OPCODE = 0xFFFF,
#error You should add opcodes here, and then uncomment all "Fix opcodes here" blocks.
};

/// Player state
enum SessionStatus
{
    STATUS_AUTHED = 0,                                      ///< Player authenticated (_player==NULL, m_playerRecentlyLogout = false or will be reset before handler call, m_GUID have garbage)
    STATUS_LOGGEDIN,                                        ///< Player in game (_player!=NULL, m_GUID == _player->GetGUID(), inWorld())
    STATUS_TRANSFER,                                        ///< Player transferring to another map (_player!=NULL, m_GUID == _player->GetGUID(), !inWorld())
    STATUS_LOGGEDIN_OR_RECENTLY_LOGGEDOUT,                  ///< _player!= NULL or _player==NULL && m_playerRecentlyLogout, m_GUID store last _player guid)
    STATUS_NEVER,                                           ///< Opcode not accepted from client (deprecated or server side only)
    STATUS_UNHANDLED                                        ///< We don' handle this opcode yet
};

enum PacketProcessing
{
    PROCESS_INPLACE = 0,                                    //process packet whenever we receive it - mostly for non-handled or non-implemented packets
    PROCESS_THREADUNSAFE,                                   //packet is not thread-safe - process it in World::UpdateSessions()
    PROCESS_THREADSAFE                                      //packet is thread-safe - process it in Map::Update()
};

class WorldPacket;

typedef void(WorldSession::*pOpcodeHandler)(WorldPacket& recvPacket);

struct OpcodeHandler
{
    OpcodeHandler() {}
    OpcodeHandler(const char* _name, SessionStatus _status, PacketProcessing _processing, pOpcodeHandler _handler)
        : name(_name), status(_status), packetProcessing(_processing), handler(_handler) {}

    char const* name;
    SessionStatus status;
    PacketProcessing packetProcessing;
    pOpcodeHandler handler;
};

#define MAX_OPCODE_HANDLER_INDEX    2048

#define DEFINE_OPCODE_HANDLER(opcode, status, processing, handler)                              \
    if (opcode != UNKNOWN_OPCODE) {                                                             \
        uint32 condensed = CondenseOpcode(opcode);                                             \
        if (condensed >= MAX_OPCODE_HANDLER_INDEX)                                              \
        {                                                                                       \
            sLog.outError("Condensed opcode out of range " #opcode " %u -> %u",                 \
                opcode, condensed);                                                             \
        }                                                                                       \
        else if (opcodeTable[condensed] != NULL)                                                \
        {                                                                                       \
            sLog.outError("Tried to override handler of %s with %s (cond %u)",                  \
                opcodeTable[condensed]->name, #opcode, condensed);                              \
        }                                                                                       \
        else opcodeTable[condensed] = new OpcodeHandler(#opcode, status, processing, handler);  \
    }

extern OpcodeHandler* opcodeTable[MAX_OPCODE_HANDLER_INDEX];
void InitializeOpcodes();

/// Condense an opcode to a value that can be used as an index
inline int CondenseOpcode(uint16 id)
{
    uint32 i = uint32(id);
    return ((i & 0xC | ((i & 0x60 | ((i & 0x1F00 | (i >> 1) & 0x6000) >> 1)) >> 1)) >> 2);
}

/// Lookup opcode name for human understandable logging
inline const char* LookupOpcodeName(uint16 id)
{
    OpcodeHandler* handler = opcodeTable[CondenseOpcode(id)];
    return handler ? handler->name : "UNKNOWN OPCODE";
}

#endif
/// @}
