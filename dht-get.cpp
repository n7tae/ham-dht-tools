/*
 *   Copyright (c) 2022 by Thomas A. Early N7TAE
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

#include <opendht.h>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <condition_variable>

#include "dht-values.h"

static bool running = true;
static std::condition_variable cv;
static std::mutex mtx;
static char section = 'a';
static bool use_local = false;
static const std::string default_bs("xrf757.openquad.net");
static dht::Where w;
static SMrefdConfig1  mrefdConfig;
static SMrefdPeers1   mrefdPeers;
static SMrefdClients1 mrefdClients;
static SMrefdUsers1   mrefdUsers;
static SUrfdConfig1   urfdConfig;
static SUrfdPeers1    urfdPeers;
static SUrfdClients1  urfdClients;
static SUrfdUsers1    urfdUsers;

enum class ENodeType { urfd, mrefd };

static const char *TimeString(const std::time_t tt)
{
	static char str[32];
	if (use_local)
		strftime(str, 32, "%F %T %Z", localtime(&tt));
	else
		strftime(str, 32, "%FT%TZ", gmtime(&tt));
	return str;
}

static void PrintMrefdConfig()
{
	std::cout << "\"Configuration\":{";
	if (mrefdConfig.timestamp)
	{
		std::cout
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
	std::cout << '}';
	if (section == 'a') std::cout << ',';
}

static void PrintUrfdConfig()
{
	std::cout << "\"Configuration\":{";
	if (urfdConfig.timestamp)
	{
		std::cout
			<< "\"Callsign\":\""           << urfdConfig.callsign        << "\","
			<< "\"Version\":\""            << urfdConfig.version         << "\","
			<< "\"Modules\":\""            << urfdConfig.modules         << "\","
			<< "\"TranscodedModules\":\""  << urfdConfig.transcodedmods  << "\",";
			for (auto c : urfdConfig.modules)
			{
				std::cout << "\"Description" << c << "\":\"" << urfdConfig.description[c-'A'] << "\",";
			}
		std::cout
			<< "\"IPv4Address\":\""        << urfdConfig.ipv4addr                           << "\","
			<< "\"IPv6Address\":\""        << urfdConfig.ipv6addr                           << "\","
			<< "\"URL\":\""                << urfdConfig.url                                << "\","
			<< "\"Country\":\""            << urfdConfig.country                            << "\","
			<< "\"Sponsor\":\""            << urfdConfig.sponsor                            << "\","
			<< "\"Email\":\""              << urfdConfig.email                              << "\","
			<< "\"G3Enable\":"             << (urfdConfig.g3enabled ? "true" : "false")     << ","
			<< "\"DCSPort\":"              << urfdConfig.port[toUType(EUrfdPorts::dcs)]     << ","
			<< "\"DExtraPort\":"           << urfdConfig.port[toUType(EUrfdPorts::dextra)]  << ","
			<< "\"DMRPlusPort\":"          << urfdConfig.port[toUType(EUrfdPorts::dmrplus)] << ","
			<< "\"DPlusPort\":"            << urfdConfig.port[toUType(EUrfdPorts::dplus)]   << ","
			<< "\"M17Port\":"              << urfdConfig.port[toUType(EUrfdPorts::m17)]     << ","
			<< "\"MMDVMPort\":"            << urfdConfig.port[toUType(EUrfdPorts::mmdvm)]   << ","
			<< "\"NXDNPort\":"             << urfdConfig.port[toUType(EUrfdPorts::nxdn)]    << ","
			<< "\"NXDNAutoLinkModule\":\"" << urfdConfig.almod[toUType(EUrfdAlMod::nxdn)]   << "\","
			<< "\"NXDNReflectorID\":"      << urfdConfig.refid[toUType(EUrfdRefId::nxdn)]   << ","
			<< "\"P25Port\":"              << urfdConfig.port[toUType(EUrfdPorts::p25)]     << ","
			<< "\"P25AutoLinkModule\":\""  << urfdConfig.almod[toUType(EUrfdAlMod::p25)]    << "\","
			<< "\"P25ReflectorID\":"       << urfdConfig.refid[toUType(EUrfdRefId::p25)]    << ","
			<< "\"URFPort\":"              << urfdConfig.port[toUType(EUrfdPorts::urf)]     << ","
			<< "\"YSFPort\":"              << urfdConfig.port[toUType(EUrfdPorts::ysf)]     << ","
			<< "\"YSFPort\":"              << urfdConfig.port[toUType(EUrfdPorts::ysf)]     << ","
			<< "\"YSFAutoLinkModule\":\""  << urfdConfig.almod[toUType(EUrfdAlMod::ysf)]    << "\","
			<< "\"YSFDefaultRxFreq\":"     << urfdConfig.ysffreq[toUType(EUrfdTxRx::rx)]    << ","
			<< "\"YSFDefaultTxFreq\":"     << urfdConfig.ysffreq[toUType(EUrfdTxRx::tx)];
	}
	std::cout << '}';
	if (section == 'a') std::cout << ',';
}

static void PrintMrefdPeers()
{
	std::cout << "\"Peers\":[";
	auto pit=mrefdPeers.list.cbegin();
	while (pit != mrefdPeers.list.cend())
	{
		std::cout <<
			"{\"Callsign\":\""   << std::get<toUType(EMrefdPeerFields::Callsign)>(*pit) << "\"," <<
			"\"Modules\":\""     << std::get<toUType(EMrefdPeerFields::Modules)>(*pit)  << "\"," <<
			"\"ConnectTime\":\"" << TimeString(std::get<toUType(EMrefdPeerFields::ConnectTime)>(*pit)) << "\"}";
		if (++pit != mrefdPeers.list.end())
			std::cout << ',';
	}
	std::cout << ']';
	if (section =='a') std::cout << ',';
}

static void PrintUrfdPeers()
{
	std::cout << "\"Peers\":[";
	auto pit=urfdPeers.list.cbegin();
	while (pit != urfdPeers.list.cend())
	{
		std::cout <<
			"{\"Callsign\":\""   << std::get<toUType(EUrfdPeerFields::Callsign)>(*pit) << "\"," <<
			"\"Modules\":\""     << std::get<toUType(EUrfdPeerFields::Modules)>(*pit)  << "\"," <<
			"\"ConnectTime\":\"" << TimeString(std::get<toUType(EUrfdPeerFields::ConnectTime)>(*pit)) << "\"}";
		if (++pit != urfdPeers.list.end())
			std::cout << ',';
	}
	std::cout << ']';
	if (section =='a') std::cout << ',';
}

static void PrintMrefdClients()
{
	std::cout << "\"Clients\":[";
	auto cit = mrefdClients.list.cbegin();
	while (cit != mrefdClients.list.cend())
	{
		std::cout <<
			"{\"Module\":\""       << std::get<toUType(EMrefdClientFields::Module)>(*cit)                    << "\"," <<
			"\"Callsign\":\""      << std::get<toUType(EMrefdClientFields::Callsign)>(*cit)                  << "\"," <<
			"\"IP\":\""            << std::get<toUType(EMrefdClientFields::Ip)>(*cit)                        << "\"," <<
			"\"ConnectTime\":\""   << TimeString(std::get<toUType(EMrefdClientFields::ConnectTime)>(*cit))   << "\"," <<
			"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EMrefdClientFields::LastHeardTime)>(*cit)) << "\"}";
		if (++cit != mrefdClients.list.cend())
			std::cout << ',';
	}
	std::cout << ']';
	if (section == 'a') std::cout << ',';
}

static void PrintUrfdClients()
{
	std::cout << "\"Clients\":[";
	auto cit = urfdClients.list.cbegin();
	while (cit != urfdClients.list.cend())
	{
		std::cout <<
			"{\"Module\":\""       << std::get<toUType(EUrfdClientFields::Module)>(*cit)                    << "\"," <<
			"\"Callsign\":\""      << std::get<toUType(EUrfdClientFields::Callsign)>(*cit)                  << "\"," <<
			"\"IP\":\""            << std::get<toUType(EUrfdClientFields::Ip)>(*cit)                        << "\"," <<
			"\"ConnectTime\":\""   << TimeString(std::get<toUType(EUrfdClientFields::ConnectTime)>(*cit))   << "\"," <<
			"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EUrfdClientFields::LastHeardTime)>(*cit)) << "\"}";
		if (++cit != urfdClients.list.cend())
			std::cout << ',';
	}
	std::cout << ']';
	if (section == 'a') std::cout << ',';
}

static void PrintMrefdUsers()
{
	std::cout << "\"Users\":[";
	auto uit = mrefdUsers.list.cbegin();
	while (uit != mrefdUsers.list.cend())
	{
		std::cout <<
			"{\"Source\":\""       << std::get<toUType(EMrefdUserFields::Source)>(*uit)                    << "\"," <<
			"\"Destination\":\""   << std::get<toUType(EMrefdUserFields::Destination)>(*uit)               << "\"," <<
			"\"Reflector\":\""     << std::get<toUType(EMrefdUserFields::Reflector)>(*uit)                 << "\"," <<
			"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EMrefdUserFields::LastHeardTime)>(*uit)) << "\"}";
		if (++uit != mrefdUsers.list.cend())
			std::cout << ',';
	}
	std::cout << ']';
}

static void PrintUrfdUsers()
{
	std::cout << "\"Users\":[";
	auto uit = urfdUsers.list.cbegin();
	while (uit != urfdUsers.list.cend())
	{
		std::cout <<
			"{\"Callsign\":\""     << std::get<toUType(EUrfdUserFields::Callsign)>(*uit)                  << "\"," <<
			"\"OnModule\":\""      << std::get<toUType(EUrfdUserFields::OnModule)>(*uit)                  << "\"," <<
			"\"ViaNode\":\""       << std::get<toUType(EUrfdUserFields::ViaNode)>(*uit)                   << "\"," <<
			"\"ViaPeer\":\""       << std::get<toUType(EUrfdUserFields::ViaPeer)>(*uit)                   << "\"," <<
			"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EUrfdUserFields::LastHeardTime)>(*uit)) << "\"}";
		if (++uit != urfdUsers.list.cend())
			std::cout << ',';
	}
	std::cout << ']';
}

static void Usage(std::ostream &ostr, const char *comname)
{
	ostr << "usage: " << comname << " [-b bootstrap] [-s {c|l|p|u}] [-l] node_name" << std::endl << std::endl;
	ostr << "Options:" << std::endl;
	ostr << "    -b (bootstrap) argument is any running node on the dht network" << std::endl;
	ostr << "       If not specified, " << default_bs << " will be used." << std::endl;
	ostr << "    -s (section) arguments is one of:" << std::endl;
	ostr << "        c - configuration" << std::endl;
	ostr << "        l - linked client list" << std::endl;
	ostr << "        p - peer list" << std::endl;
	ostr << "        u - user list" << std::endl;
	ostr << "        If no section is specified, all sections will be output." << std::endl;
	ostr << "    -l will output time values in local time, otherwise gmt is reported." << std::endl;
}


int main(int argc, char *argv[])
{
	std::string bs(default_bs);
	while (1)
	{
		int c = getopt(argc, argv, "b:s:l");
		if (c < 0)
		{
			if (1 == argc)
			{
				Usage(std::cout, argv[0]);
				exit(EXIT_SUCCESS);
			}
			break;
		}

		switch (c)
		{
			case 'b':
			bs.assign(optarg);
			break;

			case 'l':
			use_local = true;
			break;

			case 's':
			if (optarg[1])
			{
				std::cerr << argv[0] << ": " << "You can only specify a single section!" << std::endl;
				Usage(std::cerr, argv[0]);
			}
			section = optarg[0];
			if ('c'!=section && 'l'!=section && 'p'!=section && 'u'!=section)
			{
				std::cerr << argv[0] << ": " << "You have specified an illegal section!" << std::endl;
				Usage(std::cerr, argv[0]);
				exit(EXIT_FAILURE);
			}
			break;

			default:
			Usage(std::cerr, argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (optind + 1 != argc)
	{
		std::cerr << argv[0] << ": " << ((optind==argc) ? "No node_name specified!" : "Too many arguments!") << std::endl;
		Usage(std::cerr, argv[0]);
		exit(EXIT_FAILURE);
	}

	auto p = argv[optind];
	while (*p)
	{
		if (std::islower(*p))
			*p = std::toupper(*p);
		p++;
	}
	const std::string key(argv[optind]);
	auto keyhash = dht::InfoHash::get(key);

	mrefdConfig.timestamp = mrefdPeers.timestamp = mrefdClients.timestamp = mrefdUsers.timestamp = 0;
	urfdConfig.timestamp  = urfdPeers.timestamp  = urfdClients.timestamp  = urfdUsers.timestamp  = 0;

	ENodeType keytype;
	if (0 == key.compare(0, 4, "M17-"))
		keytype = ENodeType::mrefd;
	else if (0 == key.compare(0, 3, "URF"))
		keytype = ENodeType::urfd;
	else
	{
		std::cerr << "Don't know how to get '" << key << "'" << std::endl;
		return EXIT_FAILURE;
	}

	switch (keytype)
	{
		case ENodeType::mrefd:
			switch (section)
			{
				case 'c':
				w.id(toUType(EMrefdValueID::Config));
				break;

				case 'p':
				w.id(toUType(EMrefdValueID::Peers));
				break;

				case 'l':
				w.id(toUType(EMrefdValueID::Clients));
				break;

				case 'u':
				w.id(toUType(EMrefdValueID::Users));
				break;
			}
			break;
		case ENodeType::urfd:
			switch (section)
			{
				case 'c':
				w.id(toUType(EUrfdValueID::Config));
				break;

				case 'p':
				w.id(toUType(EUrfdValueID::Peers));
				break;

				case 'l':
				w.id(toUType(EUrfdValueID::Clients));
				break;

				case 'u':
				w.id(toUType(EUrfdValueID::Users));
				break;
			}
			break;
	}

	std::string name("TestGet");
	name += std::to_string(getpid());
	dht::DhtRunner node;
	node.run(17171, dht::crypto::generateIdentity(name));
	node.bootstrap(bs, "17171");
	switch (keytype)
	{
		case ENodeType::mrefd:
			node.get(
				keyhash,
				[](const std::shared_ptr<dht::Value> &v) {
					if (v->checkSignature())
					{
						switch (v->id)
						{
							case toUType(EMrefdValueID::Config):
								if (0 == v->user_type.compare(mrefdConfig.user_type))
								{
									auto rdat = dht::Value::unpack<SMrefdConfig1>(*v);
									if (rdat.timestamp > mrefdConfig.timestamp)
										mrefdConfig = dht::Value::unpack<SMrefdConfig1>(*v);
								}
								break;
							case toUType(EMrefdValueID::Peers):
								if (0 == v->user_type.compare(mrefdPeers.user_type))
								{
									auto rdat = dht::Value::unpack<SMrefdPeers1>(*v);
									if (rdat.timestamp > mrefdPeers.timestamp)
									{
										mrefdPeers = dht::Value::unpack<SMrefdPeers1>(*v);
									} else if (rdat.timestamp==mrefdPeers.timestamp)
									{
										if (rdat.sequence > mrefdPeers.sequence)
											mrefdPeers = dht::Value::unpack<SMrefdPeers1>(*v);
									}
								}
								break;
							case toUType(EMrefdValueID::Clients):
								if (0 == v->user_type.compare(mrefdClients.user_type))
								{
									auto rdat = dht::Value::unpack<SMrefdClients1>(*v);
									if (rdat.timestamp > mrefdClients.timestamp)
									{
										mrefdClients = dht::Value::unpack<SMrefdClients1>(*v);
									} else if (rdat.timestamp==mrefdClients.timestamp)
									{
										if (rdat.sequence > mrefdClients.sequence)
											mrefdClients = dht::Value::unpack<SMrefdClients1>(*v);
									}
								}
								break;
							case toUType(EMrefdValueID::Users):
								if (0 == v->user_type.compare(mrefdUsers.user_type))
								{
									auto rdat = dht::Value::unpack<SMrefdUsers1>(*v);
									if (rdat.timestamp > mrefdPeers.timestamp)
									{
										mrefdUsers = dht::Value::unpack<SMrefdUsers1>(*v);
									} else if (rdat.timestamp==mrefdUsers.timestamp)
									{
										if (rdat.sequence > mrefdUsers.sequence)
											mrefdUsers = dht::Value::unpack<SMrefdUsers1>(*v);
									}
								}
								break;
						}
					}
					else
					{
						std::cout << "Value signature failed!" << std::endl;
					}
					return true;
				},
				[](bool success) {
					if (! success)
					{
						std::cerr << "get() failed!" << std::endl;
					}
					std::unique_lock<std::mutex> lck(mtx);
					running = false;
					cv.notify_all();
				},
				{},	// empty filter
				w
			);
			break;
		case ENodeType::urfd:
			node.get(
				keyhash,
				[](const std::shared_ptr<dht::Value> &v) {
					if (v->checkSignature())
					{
						switch (v->id)
						{
							case toUType(EUrfdValueID::Config):
								if (0 == v->user_type.compare(urfdConfig.user_type))
								{
									auto rdat = dht::Value::unpack<SUrfdConfig1>(*v);
									if (rdat.timestamp > urfdConfig.timestamp)
										urfdConfig = dht::Value::unpack<SUrfdConfig1>(*v);
								}
								break;
							case toUType(EUrfdValueID::Peers):
								if (0 == v->user_type.compare(urfdPeers.user_type))
								{
									auto rdat = dht::Value::unpack<SUrfdPeers1>(*v);
									if (rdat.timestamp > urfdPeers.timestamp)
									{
										urfdPeers = dht::Value::unpack<SUrfdPeers1>(*v);
									} else if (rdat.timestamp==urfdPeers.timestamp)
									{
										if (rdat.sequence > urfdPeers.sequence)
											urfdPeers = dht::Value::unpack<SUrfdPeers1>(*v);
									}
								}
								break;
							case toUType(EUrfdValueID::Clients):
								if (0 == v->user_type.compare(urfdClients.user_type))
								{
									auto rdat = dht::Value::unpack<SUrfdClients1>(*v);
									if (rdat.timestamp > urfdClients.timestamp)
									{
										urfdClients = dht::Value::unpack<SUrfdClients1>(*v);
									} else if (rdat.timestamp==urfdClients.timestamp)
									{
										if (rdat.sequence > urfdClients.sequence)
											urfdClients = dht::Value::unpack<SUrfdClients1>(*v);
									}
								}
								break;
							case toUType(EUrfdValueID::Users):
								if (0 == v->user_type.compare(urfdUsers.user_type))
								{
									auto rdat = dht::Value::unpack<SUrfdUsers1>(*v);
									if (rdat.timestamp > mrefdPeers.timestamp)
									{
										urfdUsers = dht::Value::unpack<SUrfdUsers1>(*v);
									} else if (rdat.timestamp==urfdUsers.timestamp)
									{
										if (rdat.sequence > urfdUsers.sequence)
											urfdUsers = dht::Value::unpack<SUrfdUsers1>(*v);
									}
								}
								break;
						}
					}
					else
					{
						std::cout << "Value signature failed!" << std::endl;
					}
					return true;
				},
				[](bool success) {
					if (! success)
					{
						std::cerr << "get() failed!" << std::endl;
					}
					std::unique_lock<std::mutex> lck(mtx);
					running = false;
					cv.notify_all();
				},
				{},	// empty filter
				w
			);
			break;
	}

	std::unique_lock<std::mutex> lck(mtx);
	while (running)
	{
		cv.wait(lck);
	}

	std::cout << '{';
	switch (section)
	{
		case 'c':
			switch (keytype)
			{
				case ENodeType::mrefd: PrintMrefdConfig(); break;
				case ENodeType::urfd:  PrintUrfdConfig();  break;
			}
			break;
		case 'p':
			switch (keytype)
			{
				case ENodeType::mrefd: PrintMrefdPeers(); break;
				case ENodeType::urfd:  PrintUrfdPeers();  break;
			}
			break;
		case 'l':
			switch (keytype)
			{
				case ENodeType::mrefd: PrintMrefdClients(); break;
				case ENodeType::urfd:  PrintUrfdClients();  break;
			}
			break;
		case 'u':
			switch (keytype)
			{
				case ENodeType::mrefd: PrintMrefdUsers(); break;
				case ENodeType::urfd:  PrintUrfdUsers(); break;
			}
			break;
		default:
			switch (keytype)
			{
				case ENodeType::mrefd:
					if (mrefdConfig.timestamp)
					{
						PrintMrefdConfig();
						PrintMrefdPeers();
						PrintMrefdClients();
						PrintMrefdUsers();
						break;
					}
					break;
				case ENodeType::urfd:
					if (urfdConfig.timestamp)
					{
						PrintUrfdConfig();
						PrintUrfdPeers();
						PrintUrfdClients();
						PrintUrfdUsers();
						break;
					}
					break;
			}
			break;
	}
	std::cout << '}' << std::endl;

	node.join();

	return EXIT_SUCCESS;
}
