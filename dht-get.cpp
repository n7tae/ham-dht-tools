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

#include <opendht.h>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <condition_variable>

#include "dht-values.h"
#include "dht-helpers.h"

static bool running = true;
static std::condition_variable cv;
static std::mutex mtx;
static char section = 'a';
static bool use_local = false;
static const std::string default_bs("xrf757.openquad.net");
static dht::Where w;
static SMrefdConfig1  mrefdConfig;
static SMrefdPeers1   mrefdPeers;
static SUrfdPeers1    urfdPeers;
static SUrfdConfig1   urfdConfig;

enum class ENodeType { urfd, mrefd };

static void Usage(std::ostream &ostr, const char *comname)
{
	ostr << "usage: " << comname << " [-b bootstrap] [-s {c|l|p|u}] [-l] node_name" << std::endl << std::endl;
	ostr << "Options:" << std::endl;
	ostr << "    -b (bootstrap) argument is any running node on the dht network" << std::endl;
	ostr << "       If not specified, " << default_bs << " will be used." << std::endl;
	ostr << "    -s (section) arguments is one of:" << std::endl;
	ostr << "        c - configuration" << std::endl;
	ostr << "        p - peer list" << std::endl;
	ostr << "        If no section is specified, both sections will be output." << std::endl;
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
			if ('c'!=section && 'p'!=section)
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

	mrefdConfig.timestamp = mrefdPeers.timestamp = 0;
	urfdConfig.timestamp  = urfdPeers.timestamp  = 0;

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
			}
			break;
	}

	std::string name("HamGet");
	name += std::to_string(getpid());
	dht::DhtRunner node;
	try {
		node.run(17171, dht::crypto::generateIdentity(name), true, 59973);
		node.bootstrap(bs, "17171");
	} catch (const std::exception &ex) {
		std::cout << argv[0] << " can't connect to the Ham-DHT! " << ex.what() << std::endl;
		return 1;
	}
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
								if (0 == v->user_type.compare(MREFD_CONFIG_1))
								{
									auto rdat = dht::Value::unpack<SMrefdConfig1>(*v);
									if (rdat.timestamp > mrefdConfig.timestamp)
										mrefdConfig = dht::Value::unpack<SMrefdConfig1>(*v);
								}
								break;
							case toUType(EMrefdValueID::Peers):
								if (0 == v->user_type.compare(MREFD_PEERS_1))
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
								if (0 == v->user_type.compare(URFD_CONFIG_1))
								{
									auto rdat = dht::Value::unpack<SUrfdConfig1>(*v);
									if (rdat.timestamp > urfdConfig.timestamp)
										urfdConfig = dht::Value::unpack<SUrfdConfig1>(*v);
								}
								break;
							case toUType(EUrfdValueID::Peers):
								if (0 == v->user_type.compare(URFD_PEERS_1))
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
				case ENodeType::mrefd: PrintMrefdConfig(mrefdConfig, std::cout); break;
				case ENodeType::urfd:  PrintUrfdConfig(urfdConfig, std::cout);  break;
			}
			break;
		case 'p':
			switch (keytype)
			{
				case ENodeType::mrefd: PrintMrefdPeers(mrefdPeers, use_local, std::cout); break;
				case ENodeType::urfd:  PrintUrfdPeers(urfdPeers, use_local, std::cout);  break;
			}
			break;
		default:
			switch (keytype)
			{
				case ENodeType::mrefd:
					if (mrefdConfig.timestamp)
					{
						PrintMrefdConfig(mrefdConfig, std::cout);
						std::cout << ',';
						PrintMrefdPeers(mrefdPeers, use_local, std::cout);
					}
					break;
				case ENodeType::urfd:
					if (urfdConfig.timestamp)
					{
						PrintUrfdConfig(urfdConfig, std::cout);
						std::cout << ',';
						PrintUrfdPeers(urfdPeers, use_local, std::cout);
					}
					break;
			}
	}
	std::cout << '}' << std::endl;

	node.join();

	return EXIT_SUCCESS;
}
