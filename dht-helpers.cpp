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

#include "dht-values.h"

static const char *TimeString(const std::time_t tt, bool use_local)
{
	static char str[32];
	if (use_local)
		strftime(str, 32, "%F %T %Z", localtime(&tt));
	else
		strftime(str, 32, "%FT%TZ", gmtime(&tt));
	return str;
}

#ifdef USE_MREFD_VALUES
void PrintMrefdConfig(const SMrefdConfig1 &mrefdConfig, std::ostream &stream)
{
	stream << "\"Configuration\":{";
	if (mrefdConfig.timestamp)
	{
		stream
			<< "\"Callsign\":\""    << mrefdConfig.callsign      << "\","
			<< "\"Version\":\""     << mrefdConfig.version       << "\","
			<< "\"Modules\":\""     << mrefdConfig.modules       << "\","
			<< "\"EncryptMods\":\"" << mrefdConfig.encryptedmods << "\","
			<< "\"IPv4Address\":\"" << mrefdConfig.ipv4addr      << "\","
			<< "\"IPv6Address\":\"" << mrefdConfig.ipv6addr      << "\","
			<< "\"URL\":\""         << mrefdConfig.url           << "\","
			<< "\"Country\":\""     << mrefdConfig.country       << "\","
			<< "\"Sponsor\":\""     << mrefdConfig.sponsor       << "\","
			<< "\"Email\":\""       << mrefdConfig.email         << "\","
			<< "\"Port\":"          << mrefdConfig.port;
	}
	stream << '}';
}

void PrintMrefdPeers(const SMrefdPeers1 &mrefdPeers, bool use_local, std::ostream &stream)
{
	stream << "\"Peers\":[";
	auto pit=mrefdPeers.list.cbegin();
	while (pit != mrefdPeers.list.cend())
	{
		stream <<
			"{\"Callsign\":\""   << std::get<toUType(EMrefdPeerFields::Callsign)>(*pit) << "\"," <<
			"\"Modules\":\""     << std::get<toUType(EMrefdPeerFields::Modules)>(*pit)  << "\"," <<
			"\"ConnectTime\":\"" << TimeString(std::get<toUType(EMrefdPeerFields::ConnectTime)>(*pit), use_local) << "\"}";
		if (++pit != mrefdPeers.list.end())
			stream << ',';
	}
	stream << ']';
}

void PrintMrefdClients(const SMrefdClients1 &mrefdClients, bool use_local, std::ostream &stream)
{
	stream << "\"Clients\":[";
	auto cit = mrefdClients.list.cbegin();
	while (cit != mrefdClients.list.cend())
	{
		stream <<
			"{\"Module\":\""       << std::get<toUType(EMrefdClientFields::Module)>(*cit)                    << "\"," <<
			"\"Callsign\":\""      << std::get<toUType(EMrefdClientFields::Callsign)>(*cit)                  << "\"," <<
			"\"IP\":\""            << std::get<toUType(EMrefdClientFields::Ip)>(*cit)                        << "\"," <<
			"\"ConnectTime\":\""   << TimeString(std::get<toUType(EMrefdClientFields::ConnectTime)>(*cit), use_local)   << "\"," <<
			"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EMrefdClientFields::LastHeardTime)>(*cit), use_local) << "\"}";
		if (++cit != mrefdClients.list.cend())
			stream << ',';
	}
	stream << ']';
}

void PrintMrefdUsers(const SMrefdUsers1 &mrefdUsers, bool use_local, std::ostream &stream)
{
	stream << "\"Users\":[";
	auto uit = mrefdUsers.list.cbegin();
	while (uit != mrefdUsers.list.cend())
	{
		stream <<
			"{\"Source\":\""       << std::get<toUType(EMrefdUserFields::Source)>(*uit)                    << "\"," <<
			"\"Destination\":\""   << std::get<toUType(EMrefdUserFields::Destination)>(*uit)               << "\"," <<
			"\"Reflector\":\""     << std::get<toUType(EMrefdUserFields::Reflector)>(*uit)                 << "\"," <<
			"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EMrefdUserFields::LastHeardTime)>(*uit), use_local) << "\"}";
		if (++uit != mrefdUsers.list.cend())
			stream << ',';
	}
	stream << ']';
}
#endif

#ifdef USE_URFD_VALUES
void CopyUrfdConfigure1to2(const struct SUrfdConfig1 &c1, struct SUrfdConfig2 &c2)
{
	c2.timestamp = c1.timestamp;
	c2.callsign = c1.callsign;
	c2.ipv4addr = c1.ipv4addr;
	c2.ipv6addr = c1.ipv6addr;
	c2.modules = c1.modules;
	c2.transcodedmods = c1.transcodedmods;
	c2.url = c1.url;
	c2.email = c1.email;
	c2.sponsor = c1.sponsor;
	c2.country = c1.country;
	c2.version = c1.version;
	c2.almod = c1.almod;
	c2.ysffreq = c1.ysffreq;
	c2.refid = c1.refid;
	c2.description = c1.description;
	c2.g3enabled = c1.g3enabled;
	c2.port[toUType(EUrfdPorts2::dcs)]     = c1.port[toUType(EUrfdPorts::dcs)];
	c2.port[toUType(EUrfdPorts2::dextra)]  = c1.port[toUType(EUrfdPorts::dextra)];
	c2.port[toUType(EUrfdPorts2::dmrplus)] = c1.port[toUType(EUrfdPorts::dmrplus)];
	c2.port[toUType(EUrfdPorts2::dplus)]   = c1.port[toUType(EUrfdPorts::dplus)];
	c2.port[toUType(EUrfdPorts2::dsd)]     = 0;
	c2.port[toUType(EUrfdPorts2::m17)]     = c1.port[toUType(EUrfdPorts::m17)];
	c2.port[toUType(EUrfdPorts2::mmdvm)]   = c1.port[toUType(EUrfdPorts::mmdvm)];
	c2.port[toUType(EUrfdPorts2::nxdn)]    = c1.port[toUType(EUrfdPorts::nxdn)];
	c2.port[toUType(EUrfdPorts2::p25)]     = c1.port[toUType(EUrfdPorts::p25)];
	c2.port[toUType(EUrfdPorts2::urf)]     = c1.port[toUType(EUrfdPorts::urf)];
	c2.port[toUType(EUrfdPorts2::ysf)]     = c1.port[toUType(EUrfdPorts::ysf)];
	c2.g3enabled = false;
}

void CopyUrfdClients1to2(const struct SUrfdClients1 &c1, struct SUrfdClients2 &c2)
{
	c2.timestamp = c1.timestamp;
	c2.sequence = c1.sequence;
	c2.list.clear();
	for (const auto &item : c1.list)
	{
	c2.list.emplace_back(UrfdClientTuple2(std::get<toUType(EUrfdClientFields1::Callsign)>(item),
		"",
		std::get<toUType(EUrfdClientFields1::Ip)>(item),
		std::get<toUType(EUrfdClientFields1::Module)>(item),
		std::get<toUType(EUrfdClientFields1::ConnectTime)>(item),
		std::get<toUType(EUrfdClientFields1::LastHeardTime)>(item)
		));
	}
}

void PrintUrfdConfig(const SUrfdConfig2 urfdConfig, std::ostream &stream)
{
	stream << "\"Configuration\":{";
	if (urfdConfig.timestamp)
	{
		stream
			<< "\"Callsign\":\""           << urfdConfig.callsign        << "\","
			<< "\"Version\":\""            << urfdConfig.version         << "\","
			<< "\"Modules\":\""            << urfdConfig.modules         << "\","
			<< "\"TranscodedModules\":\""  << urfdConfig.transcodedmods  << "\",";
			for (const auto c : urfdConfig.modules)
			{
				stream << "\"Description" << c << "\":\"" << urfdConfig.description.at(c) << "\",";
			}
		stream
			<< "\"IPv4Address\":\""        << urfdConfig.ipv4addr                            << "\","
			<< "\"IPv6Address\":\""        << urfdConfig.ipv6addr                            << "\","
			<< "\"URL\":\""                << urfdConfig.url                                 << "\","
			<< "\"Country\":\""            << urfdConfig.country                             << "\","
			<< "\"Sponsor\":\""            << urfdConfig.sponsor                             << "\","
			<< "\"Email\":\""              << urfdConfig.email                               << "\","
			<< "\"DCSPort\":"              << urfdConfig.port[toUType(EUrfdPorts2::dcs)]     << ","
			<< "\"DExtraPort\":"           << urfdConfig.port[toUType(EUrfdPorts2::dextra)]  << ","
			<< "\"DMRPlusPort\":"          << urfdConfig.port[toUType(EUrfdPorts2::dmrplus)] << ","
			<< "\"DPlusPort\":"            << urfdConfig.port[toUType(EUrfdPorts2::dplus)]   << ","
			<< "\"DSDPort\":"              << urfdConfig.port[toUType(EUrfdPorts2::dsd)]     << ","
			<< "\"M17Port\":"              << urfdConfig.port[toUType(EUrfdPorts2::m17)]     << ","
			<< "\"MMDVMPort\":"            << urfdConfig.port[toUType(EUrfdPorts2::mmdvm)]   << ","
			<< "\"NXDNPort\":"             << urfdConfig.port[toUType(EUrfdPorts2::nxdn)]    << ","
			<< "\"NXDNAutoLinkModule\":\"" << urfdConfig.almod[toUType(EUrfdAlMod::nxdn)]    << "\","
			<< "\"NXDNReflectorID\":"      << urfdConfig.refid[toUType(EUrfdRefId::nxdn)]    << ","
			<< "\"P25Port\":"              << urfdConfig.port[toUType(EUrfdPorts2::p25)]     << ","
			<< "\"P25AutoLinkModule\":\""  << urfdConfig.almod[toUType(EUrfdAlMod::p25)]     << "\","
			<< "\"P25ReflectorID\":"       << urfdConfig.refid[toUType(EUrfdRefId::p25)]     << ","
			<< "\"URFPort\":"              << urfdConfig.port[toUType(EUrfdPorts2::urf)]     << ","
			<< "\"YSFPort\":"              << urfdConfig.port[toUType(EUrfdPorts2::ysf)]     << ","
			<< "\"YSFPort\":"              << urfdConfig.port[toUType(EUrfdPorts2::ysf)]     << ","
			<< "\"YSFAutoLinkModule\":\""  << urfdConfig.almod[toUType(EUrfdAlMod::ysf)]     << "\","
			<< "\"YSFDefaultRxFreq\":"     << urfdConfig.ysffreq[toUType(EUrfdTxRx::rx)]     << ","
			<< "\"YSFDefaultTxFreq\":"     << urfdConfig.ysffreq[toUType(EUrfdTxRx::tx)];
	}
	stream << '}';
}

void PrintUrfdPeers(const SUrfdPeers1 &urfdPeers, bool use_local, std::ostream &stream)
{
	stream << "\"Peers\":[";
	auto pit=urfdPeers.list.cbegin();
	while (pit != urfdPeers.list.cend())
	{
		stream <<
			"{\"Callsign\":\""   << std::get<toUType(EUrfdPeerFields::Callsign)>(*pit) << "\"," <<
			"\"Modules\":\""     << std::get<toUType(EUrfdPeerFields::Modules)>(*pit)  << "\"," <<
			"\"ConnectTime\":\"" << TimeString(std::get<toUType(EUrfdPeerFields::ConnectTime)>(*pit), use_local) << "\"}";
		if (++pit != urfdPeers.list.end())
			stream << ',';
	}
	stream << ']';
}

void PrintUrfdClients(const SUrfdClients2 &urfdClients, bool use_local, std::ostream &stream)
{
	stream << "\"Clients\":[";
	auto cit = urfdClients.list.cbegin();
	while (cit != urfdClients.list.cend())
	{
		const auto &protocol = std::get<toUType(EUrfdClientFields2::Protocol)>(*cit);
		stream <<
			"{\"Module\":\""       << std::get<toUType(EUrfdClientFields2::Module)>(*cit)                    << "\"," <<
			"\"Callsign\":\""      << std::get<toUType(EUrfdClientFields2::Callsign)>(*cit)                  << "\"," <<
			"\"IP\":\""            << std::get<toUType(EUrfdClientFields2::Ip)>(*cit)                        << "\",";
		if (protocol.size())
		{
			stream << "\"Protocol\":\"" << protocol << "\",";
		}
		stream <<
			"\"ConnectTime\":\""   << TimeString(std::get<toUType(EUrfdClientFields2::ConnectTime)>(*cit), use_local)   << "\"," <<
			"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EUrfdClientFields2::LastHeardTime)>(*cit), use_local) << "\"}";
		if (++cit != urfdClients.list.cend())
			stream << ',';
	}
	stream << ']';
}

void PrintUrfdUsers(const SUrfdUsers1 &urfdUsers, bool use_local, std::ostream &stream)
{
	stream << "\"Users\":[";
	auto uit = urfdUsers.list.cbegin();
	while (uit != urfdUsers.list.cend())
	{
		stream <<
			"{\"Callsign\":\""     << std::get<toUType(EUrfdUserFields::Callsign)>(*uit)                  << "\"," <<
			"\"OnModule\":\""      << std::get<toUType(EUrfdUserFields::OnModule)>(*uit)                  << "\"," <<
			"\"ViaNode\":\""       << std::get<toUType(EUrfdUserFields::ViaNode)>(*uit)                   << "\"," <<
			"\"ViaPeer\":\""       << std::get<toUType(EUrfdUserFields::ViaPeer)>(*uit)                   << "\"," <<
			"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EUrfdUserFields::LastHeardTime)>(*uit), use_local) << "\"}";
		if (++uit != urfdUsers.list.cend())
			stream << ',';
	}
	stream << ']';
}
#endif
