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

#include "mrefd-dht-values.h"

static bool running = true;
static std::condition_variable cv;
static std::mutex mtx;
static char section = 'a';
static bool use_local = false;
static const std::string default_bs("xrf757.openquad.net");
static dht::Where w;
static SMrefdConfig1  config;
static SMrefdPeers1   peers;
static SMrefdClients1 clients;
static SMrefdUsers1   users;

static const char *TimeString(const std::time_t tt)
{
	static char str[32];
	if (use_local)
		strftime(str, 32, "%F %T %Z", localtime(&tt));
	else
		strftime(str, 32, "%FT%TZ", gmtime(&tt));
	return str;
}

static void PrintConfig()
{
	if (config.timestamp)
	{
		std::cout << "\"Configuration\":{"
			<< "\"Callsign\":\""    << config.cs      << "\","
			<< "\"Version\":\""     << config.version << "\","
			<< "\"Modules\":\""     << config.mods    << "\","
			<< "\"EncryptMods\":\"" << config.emods   << "\","
			<< "\"IPv4Address\":\"" << config.ipv4    << "\","
			<< "\"IPv6Address\":\"" << config.ipv6    << "\","
			<< "\"URL\":\""         << config.url     << "\","
			<< "\"Country\":\""     << config.country << "\","
			<< "\"Sponsor\":\""     << config.sponsor << "\","
			<< "\"Email\":\""       << config.email   << "\","
			<< "\"Port\":"          << config.port    << '}';
		if (section == 'a') std::cout << ',';
	}
}

static void PrintPeers()
{
	if (peers.timestamp)
	{
		std::cout << "\"Peers\":[";
		auto pit=peers.list.cbegin();
		while (pit != peers.list.cend())
		{
			std::cout <<
				"{\"Callsign\":\""   << std::get<toUType(EMrefdPeerFields::Callsign)>(*pit) << "\"," <<
				"\"Modules\":\""     << std::get<toUType(EMrefdPeerFields::Modules)>(*pit)  << "\"," <<
				"\"ConnectTime\":\"" << TimeString(std::get<toUType(EMrefdPeerFields::ConnectTime)>(*pit)) << "\"}";
			if (++pit != peers.list.end())
				std::cout << ',';
		}
		std::cout << ']';
		if (section =='a') std::cout << ',';
	}
}

static void PrintClients()
{
	if (clients.timestamp)
	{
		std::cout << "\"Clients\":[";
		auto cit = clients.list.cbegin();
		while (cit != clients.list.cend())
		{
			std::cout <<
				"{\"Module\":\""       << std::get<toUType(EMrefdClientFields::Module)>(*cit)                    << "\"," <<
				"\"Callsign\":\""      << std::get<toUType(EMrefdClientFields::Callsign)>(*cit)                  << "\"," <<
				"\"IP\":\""            << std::get<toUType(EMrefdClientFields::Ip)>(*cit)                        << "\"," <<
				"\"ConnectTime\":\""   << TimeString(std::get<toUType(EMrefdClientFields::ConnectTime)>(*cit))   << "\"," <<
				"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EMrefdClientFields::LastHeardTime)>(*cit)) << "\"}";
			if (++cit != clients.list.cend())
				std::cout << ',';
		}
		std::cout << ']';
		if (section == 'a') std::cout << ',';
	}
}

static void PrintUsers()
{
	if (users.timestamp)
	{
		std::cout << "\"Users\":[";
		auto uit = users.list.cbegin();
		while (uit != users.list.cend())
		{
			std::cout <<
				"{\"Source\":\""       << std::get<toUType(EMrefdUserFields::Source)>(*uit)                    << "\"," <<
				"\"Destination\":\""   << std::get<toUType(EMrefdUserFields::Destination)>(*uit)               << "\"," <<
				"\"Reflector\":\""     << std::get<toUType(EMrefdUserFields::Reflector)>(*uit)                 << "\"," <<
				"\"LastHeardTime\":\"" << TimeString(std::get<toUType(EMrefdUserFields::LastHeardTime)>(*uit)) << "\"}";
			if (++uit != users.list.cend())
				std::cout << ',';
		}
		std::cout << ']';
	}
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

	config.timestamp = peers.timestamp = clients.timestamp = users.timestamp = 0;

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

	std::string name("TestGet");
	name += std::to_string(getpid());
	dht::DhtRunner node;
	node.run(17171, dht::crypto::generateIdentity(name));
	node.bootstrap(bs, "17171");
	node.get(
		keyhash,
		[](const std::shared_ptr<dht::Value> &v) {
			if (v->checkSignature())
			{
				switch (v->id)
				{
					case toUType(EMrefdValueID::Config):
						if (0 == v->user_type.compare("mrefd-config-1"))
						{
							auto rdat = dht::Value::unpack<SMrefdConfig1>(*v);
							if (rdat.timestamp > config.timestamp)
								config = dht::Value::unpack<SMrefdConfig1>(*v);
						}
						break;
					case toUType(EMrefdValueID::Peers):
						if (0 == v->user_type.compare("mrefd-peers-1"))
						{
							auto rdat = dht::Value::unpack<SMrefdPeers1>(*v);
							if (rdat.timestamp > peers.timestamp)
							{
								peers = dht::Value::unpack<SMrefdPeers1>(*v);
							} else if (rdat.timestamp==peers.timestamp)
							{
								if (rdat.sequence > peers.sequence)
									peers = dht::Value::unpack<SMrefdPeers1>(*v);
							}
						}
						break;
					case toUType(EMrefdValueID::Clients):
						if (0 == v->user_type.compare("mrefd-clients-1"))
						{
							auto rdat = dht::Value::unpack<SMrefdClients1>(*v);
							if (rdat.timestamp > peers.timestamp)
							{
								clients = dht::Value::unpack<SMrefdClients1>(*v);
							} else if (rdat.timestamp==clients.timestamp)
							{
								if (rdat.sequence > clients.sequence)
									clients = dht::Value::unpack<SMrefdClients1>(*v);
							}
						}
						break;
					case toUType(EMrefdValueID::Users):
						if (0 == v->user_type.compare("mrefd-users-1"))
						{
							auto rdat = dht::Value::unpack<SMrefdUsers1>(*v);
							if (rdat.timestamp > peers.timestamp)
							{
								users = dht::Value::unpack<SMrefdUsers1>(*v);
							} else if (rdat.timestamp==users.timestamp)
							{
								if (rdat.sequence > users.sequence)
									users = dht::Value::unpack<SMrefdUsers1>(*v);
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

	std::unique_lock<std::mutex> lck(mtx);
	while (running)
	{
		cv.wait(lck);
	}

	std::cout << '{';
	switch (section)
	{
		case 'c':
			PrintConfig();
			break;
		case 'p':
			PrintPeers();
			break;
		case 'l':
			PrintClients();
			break;
		case 'u':
			PrintUsers();
			break;
		default:
			if (config.timestamp)
			{
				PrintConfig();
				PrintPeers();
				PrintClients();
				PrintUsers();
				break;
			}
			break;
	}
	std::cout << '}' << std::endl;

	node.join();

	return EXIT_SUCCESS;
}
