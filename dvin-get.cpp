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

static const char *TimeString(const std::time_t tt)
{
	static char str[32];
	strftime(str, 32, "%D %R", localtime(&tt));
	return str;
}

int main(int argc, char *argv[])
{
	if (2 > argc || argc > 3)
	{
		std::cout << "usage: " << argv[0] << " <bootstrap> key" << std::endl;
		return EXIT_FAILURE;
	}

	const std::string bs((2==argc) ? "xrf757.openquad.net" : argv[1]);
	auto p = argv[argc-1];
	while (*p)
	{
		if (std::islower(*p))
			*p = std::toupper(*p);
		p++;
	}
	const std::string key(argv[argc-1]);
	auto keyhash = dht::InfoHash::get(key);

	std::string name("TestGet");
	name += std::to_string(getpid());
	dht::DhtRunner node;
	node.run(17171, dht::crypto::generateIdentity(name));
	node.bootstrap(bs, "17171");
	std::cout << "Joined the DHT at " << bs << " using an id of " << name << std::endl;
	std::cout << "Getting Data for " << key << " with hash " << keyhash << std::endl;
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
							std::cout << "###Configuration###" << std::endl;
							std::cout << "Callsign=" << rdat.cs << std::endl;
							std::cout << "Version=" << rdat.version << std::endl;
							std::cout << "Modules=" << rdat.mods << std::endl;
							std::cout << "IPv4 Address=" << rdat.ipv4 << std::endl;
							std::cout << "IPv6 Address=" << rdat.ipv6 << std::endl;
							std::cout << "Port=" << rdat.port << std::endl;
							std::cout << "URL=" << rdat.url << std::endl;
							std::cout << "Country=" << rdat.country << std::endl;
							std::cout << "Sponsor=" << rdat.sponsor << std::endl;
							std::cout << "Email Address=" << rdat.email << std::endl;
						}
						else
							std::cout << "Unknown Configuration user_type: " << v->user_type << std::endl;
						break;
					case toUType(EMrefdValueID::Peers):
						if (0 == v->user_type.compare("mrefd-peers-0"))
						{
							auto rdat = dht::Value::unpack<SMrefdPeers0>(*v);
							if (rdat.peers.size())
							{
								std::cout << "###Peers###" << std::endl << "Callsign,Modules,Connect" << std::endl;
								for (const auto &peer : rdat.peers)
								{
									std::cout <<
									std::get<toUType(EMrefdPeerFields::Callsign)>(peer) << ',' <<
									std::get<toUType(EMrefdPeerFields::Modules)>(peer) << ',' <<
									TimeString(std::get<toUType(EMrefdPeerFields::ConnectTime)>(peer)) << std::endl;
								}
							}
							else
								std::cout << "###Peers list is empty###" << std::endl;
						}
						else
							std::cout << "Unknown Peers user_type: " << v->user_type << std::endl;
						break;
					case toUType(EMrefdValueID::Clients):
						if (0 == v->user_type.compare("mrefd-clients-0"))
						{
							auto rdat = dht::Value::unpack<SMrefdClients0>(*v);
							if (rdat.clients.size())
							{
								std::cout << "###Clients###" << std::endl << "Module,Callsign,IP,Connect,LastHeard" << std::endl;
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
								std::cout << "###Clients list is empty###" << std::endl;
						}
						else
							std::cout << "Unknown Clients user_type: " << v->user_type << std::endl;
						break;
					case toUType(EMrefdValueID::Users):
						if (0 == v->user_type.compare("mrefd-users-0"))
						{
							auto rdat = dht::Value::unpack<SMrefdUsers0>(*v);
							if (rdat.users.size())
							{
								std::cout << "###Users###" << std::endl << "Source,Destination,Reflector,LastHeard" << std::endl;
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
								std::cout << "###Users list is empty###" << std::endl;
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
		}
	);

	std::unique_lock<std::mutex> lck(mtx);
	while (running)
		cv.wait(lck);

	std::chrono::duration<double> elapsed(std::chrono::steady_clock::now() - starttime);
	std::cout << "time: " << std::setprecision(3) << elapsed.count() << " seconds" << std::endl;

	node.join();

	return EXIT_SUCCESS;
}
