//  Copyright © 2022 Thomas A. Early, N7TAE
//
// ----------------------------------------------------------------------------
//    This is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    with this software.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#pragma once

#include <opendht.h>

#define USE_MREFD_VALUES
#define USE_URFD_VALUES

/* HELPERS */
template<typename E> constexpr auto toUType(E enumerator) noexcept
{
	return static_cast<std::underlying_type_t<E>>(enumerator);
} // Item #10 in "Effective Modern C++", by Scott Meyers, O'REILLY

// every value type needs a const user_type and a timestamp
struct SDhtValueBase
{
	SDhtValueBase(const std::string &s) : user_type(s) {}
	std::string user_type;
	std::time_t timestamp;
};

#ifdef USE_MREFD_VALUES

// dht::Value ids of the different parts of the document
// can be assigned any unsigned value except 0
enum class EMrefdValueID : uint64_t { Config=1, Peers=2, Clients=3, Users=4 };

using MrefdPeerTuple = std::tuple<std::string, std::string, std::time_t>;
enum class EMrefdPeerFields { Callsign, Modules, ConnectTime };
struct SMrefdPeers1 : public SDhtValueBase
{
	SMrefdPeers1() : SDhtValueBase("mrefd-peers-1") {}
	unsigned int sequence;
	std::list<MrefdPeerTuple> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

using MrefdClientTuple = std::tuple<std::string, std::string, char, std::time_t, std::time_t>;
enum class EMrefdClientFields { Callsign, Ip, Module, ConnectTime, LastHeardTime };
struct SMrefdClients1 : public SDhtValueBase
{
	SMrefdClients1() : SDhtValueBase("mrefd-clients-1") {}
	unsigned int sequence;
	std::list<MrefdClientTuple> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

using MrefdUserTuple = std::tuple<std::string, std::string, std::string, std::time_t>;
enum class EMrefdUserFields { Source, Destination, Reflector, LastHeardTime };
struct SMrefdUsers1 : public SDhtValueBase
{
	SMrefdUsers1() : SDhtValueBase("mrefd-users-1") {}
	unsigned int sequence;
	std::list<MrefdUserTuple> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

struct SMrefdConfig1 : public SDhtValueBase
{
	SMrefdConfig1() : SDhtValueBase("mrefd-config-1") {}
	std::string callsign, ipv4addr, ipv6addr, modules, encryptedmods, url, email, sponsor, country, version;
	uint16_t port;

	MSGPACK_DEFINE(timestamp, callsign, ipv4addr, ipv6addr, modules, encryptedmods, url, email, sponsor, country, version, port)
};

#endif

#ifdef USE_URFD_VALUES

enum class EUrfdValueID : uint64_t { Config=1, Peers=2, Clients=3, Users=4 };

using UrfdPeerTuple = std::tuple<std::string, std::string, std::time_t>;
enum class EUrfdPeerFields { Callsign, Modules, ConnectTime };
struct SUrfdPeers1 : public SDhtValueBase
{
	SUrfdPeers1() : SDhtValueBase("urfd-peers-1") {}
	unsigned int sequence;
	std::list<UrfdPeerTuple> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

using UrfdClientTuple = std::tuple<std::string, std::string, char, std::time_t, std::time_t>;
enum class EUrfdClientFields { Callsign, Ip, Module, ConnectTime, LastHeardTime };
struct SUrfdClients1 : public SDhtValueBase
{
	SUrfdClients1() : SDhtValueBase("urfd-clients-1") {}
	unsigned int sequence;
	std::list<UrfdClientTuple> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

using UrfdUserTuple = std::tuple<std::string, std::string, char, std::string, std::time_t>;
enum class EUrfdUserFields { Callsign, ViaNode, OnModule, ViaPeer, LastHeardTime };
struct SUrfdUsers1 : public SDhtValueBase
{
	SUrfdUsers1() : SDhtValueBase("urfd-user-1") {}
	unsigned int sequence;
	std::list<UrfdUserTuple> list;

	MSGPACK_DEFINE(timestamp, sequence, list)
};

// 'SIZE' has to be last value for these scoped enums
enum class EUrfdPorts : unsigned { dcs, dextra, dmrplus, dplus, m17, mmdvm, nxdn, p25, urf, ysf, SIZE };
enum class EUrfdAlMod : unsigned { nxdn, p25, ysf, SIZE };
enum class EUrfdTxRx  : unsigned { rx, tx, SIZE };
enum class EUrfdRefId : unsigned { nxdn, p25, SIZE };
struct SUrfdConfig1 : public SDhtValueBase
{
	SUrfdConfig1() : SDhtValueBase("urfd-config-1") {}
	std::string callsign, ipv4addr, ipv6addr, modules, transcodedmods, url, email, sponsor, country, version;
	std::array<uint16_t, toUType(EUrfdPorts::SIZE)> port;
	std::array<char, toUType(EUrfdAlMod::SIZE)> almod;
	std::array<unsigned long, toUType(EUrfdTxRx::SIZE)> ysffreq;
	std::array<unsigned, toUType(EUrfdRefId::SIZE)> refid;
	std::unordered_map<char, std::string> description;
	bool g3enabled;

	MSGPACK_DEFINE(timestamp, callsign, ipv4addr, ipv6addr, modules, transcodedmods, url, email, sponsor, country, version, almod, ysffreq, refid, g3enabled, port, description)
};

#endif
