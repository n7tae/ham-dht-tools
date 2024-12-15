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

void PrintUrfdConfig(const SUrfdConfig1 urfdConfig, std::ostream &stream)
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
			<< "\"DCSPort\":"              << urfdConfig.port[toUType(EUrfdPorts::dcs)]     << ","
			<< "\"DExtraPort\":"           << urfdConfig.port[toUType(EUrfdPorts::dextra)]  << ","
			<< "\"DMRPlusPort\":"          << urfdConfig.port[toUType(EUrfdPorts::dmrplus)] << ","
			<< "\"DPlusPort\":"            << urfdConfig.port[toUType(EUrfdPorts::dplus)]   << ","
			<< "\"M17Port\":"              << urfdConfig.port[toUType(EUrfdPorts::m17)]     << ","
			<< "\"MMDVMPort\":"            << urfdConfig.port[toUType(EUrfdPorts::mmdvm)]   << ","
			<< "\"NXDNPort\":"             << urfdConfig.port[toUType(EUrfdPorts::nxdn)]    << ","
			<< "\"NXDNAutoLinkModule\":\"" << urfdConfig.almod[toUType(EUrfdAlMod::nxdn)]    << "\","
			<< "\"NXDNReflectorID\":"      << urfdConfig.refid[toUType(EUrfdRefId::nxdn)]    << ","
			<< "\"P25Port\":"              << urfdConfig.port[toUType(EUrfdPorts::p25)]     << ","
			<< "\"P25AutoLinkModule\":\""  << urfdConfig.almod[toUType(EUrfdAlMod::p25)]     << "\","
			<< "\"P25ReflectorID\":"       << urfdConfig.refid[toUType(EUrfdRefId::p25)]     << ","
			<< "\"URFPort\":"              << urfdConfig.port[toUType(EUrfdPorts::urf)]     << ","
			<< "\"YSFPort\":"              << urfdConfig.port[toUType(EUrfdPorts::ysf)]     << ","
			<< "\"YSFPort\":"              << urfdConfig.port[toUType(EUrfdPorts::ysf)]     << ","
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
