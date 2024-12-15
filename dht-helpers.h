/*
 *   Copyright (c) 2022-2024 by Thomas A. Early N7TAE
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once

#include <iostream>

#include "dht-values.h"

#ifdef USE_MREFD_VALUES
extern void PrintMrefdConfig(const SMrefdConfig1 &mrefdConfig, std::ostream &stream);
extern void PrintMrefdPeers(const SMrefdPeers1 &mrefdPeers, bool use_local, std::ostream &stream);
extern void PrintMrefdClients(const SMrefdClients1 &mrefdClients, bool use_local, std::ostream &stream);
extern void PrintMrefdUsers(const SMrefdUsers1 &mrefdUsers, bool use_local, std::ostream &stream);
#endif

#ifdef USE_URFD_VALUES
extern void PrintUrfdConfig(const SUrfdConfig1 urfdConfig, std::ostream &stream);
extern void PrintUrfdPeers(const SUrfdPeers1 &urfdPeers, bool use_local, std::ostream &stream);
#endif
