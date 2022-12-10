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

#include "reflectordht.h"

static bool running = true;
static std::condition_variable cv;
static std::mutex mtx;
static bool use_json = false;
static bool use_gmt = false;
static const std::string default_bs("xrf757.openquad.net");
dht::Where w;

static const char *TimeString(const std::time_t tt)
{
	static char str[32];
	strftime(str, 32, "%D %R", use_gmt ? gmtime(&tt) : localtime(&tt));
	return str;
}

static void PrintConfig0(const SMrefdConfig0 &rdat)
{
	if (use_json)
	{

	}
	else
	{
		std::cout << "# Configuration" << std::endl;
		std::cout << "Callsign=" << rdat.cs << std::endl;
		std::cout << "Version=" << rdat.version << std::endl;
		std::cout << "Modules=" << rdat.mods << std::endl;
		std::cout << "IPv4Address=" << rdat.ipv4 << std::endl;
		std::cout << "IPv6Address=" << rdat.ipv6 << std::endl;
		std::cout << "Port=" << rdat.port << std::endl;
		std::cout << "URL=" << rdat.url << std::endl;
		std::cout << "Country=" << rdat.country << std::endl;
		std::cout << "Sponsor=" << rdat.sponsor << std::endl;
		std::cout << "EmailAddress=" << rdat.email << std::endl;
	}
}

static void PrintPeers0(const SMrefdPeers0 &rdat)
{
	if (use_json)
	{

	}
	else
	{
		if (rdat.peers.size())
		{
			std::cout << "# Peers" << std::endl << "Callsign,Modules,Connect" << std::endl;
			for (const auto &peer : rdat.peers)
			{
				std::cout <<
				std::get<toUType(EMrefdPeerFields::Callsign)>(peer) << ',' <<
				std::get<toUType(EMrefdPeerFields::Modules)>(peer) << ',' <<
				TimeString(std::get<toUType(EMrefdPeerFields::ConnectTime)>(peer)) << std::endl;
			}
		}
		else
			std::cout << "# Peers is empty" << std::endl;
	}
}

static void PrintClients0(const SMrefdClients0 &rdat)
{
	if (use_json)
	{

	}
	else
	{
		if (rdat.clients.size())
		{
			std::cout << "# Clients" << std::endl << "Module,Callsign,IP,Connect,LastHeard" << std::endl;
			for (const auto &client : rdat.clients)
			{
				std::cout <<
				std::get<toUType(EMrefdClientFields::Module)>(client) << ',' <<
				std::get<toUType(EMrefdClientFields::Callsign)>(client) << ',' <<
				std::get<toUType(EMrefdClientFields::Ip)>(client) << ',' <<
				TimeString(std::get<toUType(EMrefdClientFields::ConnectTime)>(client)) << ',' <<
				TimeString(std::get<toUType(EMrefdClientFields::LastHeardTime)>(client)) << std::endl;
			}
		}
		else
			std::cout << "# Clients is empty###" << std::endl;
	}
}

static void PrintUsers0(const SMrefdUsers0 &rdat)
{
	if (use_json)
	{

	}
	else
	{
		if (rdat.users.size())
		{
			std::cout << "# Users" << std::endl << "Source,Destination,Reflector,LastHeard" << std::endl;
			for (const auto &user : rdat.users)
			{
				std::cout <<
				std::get<toUType(EMrefdUserFields::Source)>(user) << ',' <<
				std::get<toUType(EMrefdUserFields::Destination)>(user) << ',' <<
				std::get<toUType(EMrefdUserFields::Reflector)>(user) << ',' <<
				TimeString(std::get<toUType(EMrefdUserFields::LastHeardTime)>(user)) << std::endl;
			}
		}
		else
			std::cout << "# Users is empty" << std::endl;
	}

}

static void Usage(std::ostream &ostr, const char *comname)
{
	ostr << "usage: " << comname << " [-b bootstrap] [-s {c|l|p|u}] [-j] [-g] node_name" << std::endl;
	ostr << "  If no bootstrap is supplied, " << default_bs << " will be used." << std::endl;
	ostr << "  -s (section) arguments are:" << std::endl;
	ostr << "    c: configuration" << std::endl;
	ostr << "    l: linked client list" << std::endl;
	ostr << "    p: peer list" << std::endl;
	ostr << "    u: user list" << std::endl;
	ostr << "  If no section is specified, all sections will be output." << std::endl;
	ostr << "  -j will output section(s) in json format." << std::endl;
	ostr << "  -g will output time values in gmt, otherwise local" << std::endl;
}


int main(int argc, char *argv[])
{
	std::string bs(default_bs);
	char section = 'a';
	while (1)
	{
		int c = getopt(argc, argv, "b:s:jg");
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

			case 'j':
			//use_json = true;
			break;

			case 'g':
			use_gmt = true;
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
	auto starttime = std::chrono::steady_clock::now();
	node.get(
		keyhash,
		[](const std::shared_ptr<dht::Value> &v) {
			if (v->checkSignature())
			{
				switch (v->id)
				{
					case toUType(EMrefdValueID::Config):
						if (0 == v->user_type.compare("mrefd-config-0"))
						{
							auto rdat = dht::Value::unpack<SMrefdConfig0>(*v);
							PrintConfig0(rdat);
						}
						else
							std::cout << "Unknown Configuration user_type: " << v->user_type << std::endl;
						break;
					case toUType(EMrefdValueID::Peers):
						if (0 == v->user_type.compare("mrefd-peers-0"))
						{
							auto rdat = dht::Value::unpack<SMrefdPeers0>(*v);
							PrintPeers0(rdat);
						}
						else
							std::cout << "Unknown Peers user_type: " << v->user_type << std::endl;
						break;
					case toUType(EMrefdValueID::Clients):
						if (0 == v->user_type.compare("mrefd-clients-0"))
						{
							auto rdat = dht::Value::unpack<SMrefdClients0>(*v);
							PrintClients0(rdat);
						}
						else
							std::cout << "Unknown Clients user_type: " << v->user_type << std::endl;
						break;
					case toUType(EMrefdValueID::Users):
						if (0 == v->user_type.compare("mrefd-users-0"))
						{
							auto rdat = dht::Value::unpack<SMrefdUsers0>(*v);
							PrintUsers0(rdat);
						}
						else
							std::cout << "Unknown Users user_type: " << v->user_type << std::endl;
						break;
					default:
						std::cout << "Don't recognize id=0x" << std::hex << v->id << std::dec << std::endl;
						break;
				}
			}
			else
				std::cout << "Value signature failed!" << std::endl;
			return true;
		},
		[](bool success) {
			if (! success)
				std::cerr << "get failed!" << std::endl;
			std::unique_lock<std::mutex> lck(mtx);
			running = false;
			cv.notify_all();
		},
		{},	// empty filter
		w
	);

	std::unique_lock<std::mutex> lck(mtx);
	while (running)
		cv.wait(lck);

	std::chrono::duration<double> elapsed(std::chrono::steady_clock::now() - starttime);
	std::cout << "time: " << std::setprecision(3) << elapsed.count() << " seconds" << std::endl;

	node.join();

	return EXIT_SUCCESS;
}
