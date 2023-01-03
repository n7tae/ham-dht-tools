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
#include <set>
#include <map>
#include <list>
#include <atomic>

#include "mrefd-dht-values.h"

static const std::string default_bs("xlx757.openquad.net");
static dht::Where w;
SMrefdPeers1 peers;
static std::map<std::string, std::set<std::string>> Web;
static std::atomic<bool> ready;

static void FindPeers(dht::DhtRunner &node, const std::string &refcs, const char module)
{
	peers.timestamp = 0;
	peers.sequence = 0;
	peers.list.clear();
	ready = false;
	node.get(
		dht::InfoHash::get(refcs),
		[](const std::shared_ptr<dht::Value> &v)
		{
			if (v->checkSignature())
			{
				if (0 == v->user_type.compare("mrefd-peers-1"))
				{
					auto rdat = dht::Value::unpack<SMrefdPeers1>(*v);
					if (rdat.timestamp > peers.timestamp)
					{
						peers = dht::Value::unpack<SMrefdPeers1>(*v);
					}
					else if (rdat.timestamp == peers.timestamp)
					{
						if (rdat.sequence > peers.sequence)
							peers = dht::Value::unpack<SMrefdPeers1>(*v);
					}
				}
			}
			else
			{
				std::cout << "Value signature failed!" << std::endl;
			}
			return true;
		},
		[](bool success)
		{
			if (!success)
			{
				std::cerr << "get() failed!" << std::endl;
			}
			ready = true;
		},
		{}, // empty filter
		w
	);

	// wait for node.get() to complete
	while (! ready)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	// add the webnode to the map
	std::set<std::string> peerset;
	for (const auto &p : peers.list)
	{
		const auto modules = std::get<toUType(EMrefdPeerFields::Modules)>(p);
		if (std::string::npos != modules.find(module)) // add only if the peer is using this module
		{
			const auto ref = std::get<toUType(EMrefdPeerFields::Callsign)>(p);
			auto rval = peerset.insert(ref);
			if (false == rval.second)
				std::cout << "WARNING: " << ref << "could not be added to the " << refcs << " peers!" << std::endl;
		}
	}
	auto rval = Web.emplace(refcs, peerset);
	if (false == rval.second)
	{
		std::cerr << "ERROR: Could not create Web map item for " << refcs << std::endl;
		exit(EXIT_FAILURE);
	}

	// now, keep looking
	for (const auto &pstr : Web[refcs])
	{
		if (Web.end() == Web.find(pstr))
			FindPeers(node, pstr, module);
	}
}

static void Usage(std::ostream &ostr, const char *comname)
{
	ostr << "usage: " << comname << " [-b bootstrap] node_name module" << std::endl << std::endl;
	ostr << "Options:" << std::endl;
	ostr << "    -b (bootstrap) argument is any running node on the dht network" << std::endl;
	ostr << "       If not specified, " << default_bs << " will be used." << std::endl;
}

int main(int argc, char *argv[])
{
	// parse the command line
	std::string bs(default_bs);
	while (1)
	{
		int c = getopt(argc, argv, "b:");
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

		default:
			Usage(std::cerr, argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (optind + 2 != argc)
	{
		std::cerr << "Error: " << argv[0] << " needs two arguments!" << std::endl;
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

	p = argv[++optind];
	if (1 != std::strlen(p) || ! std::isalpha(*p))
	{
		std::cerr << "Error: second argument must specify a single module!" << std::endl;
		Usage(std::cerr, argv[0]);
		exit(EXIT_FAILURE);
	}

	const char module = std::toupper(*p);
	// command line parsing done

	// log into the dht
	std::string name("Spider");
	name += std::to_string(getpid());
	dht::DhtRunner node;
	node.run(17171, dht::crypto::generateIdentity(name));
	node.bootstrap(bs, "17171");
	std::cout << "Running node using name " << name << " and bootstrapping from " << bs << std::endl;

	// set up the where condition
	w.id(toUType(EMrefdValueID::Peers));

	// start the spider
	FindPeers(node, key, module);

	// print the connect matrix
	std::list<std::string> group;
	for (const auto &row : Web)
	{
		group.push_back(row.first);
	}
	const std::string space(9, ' ');
	std::cout << space << std::string(2*Web.size()+1, '=') << std::endl;
	for (const auto &row : group)
	{
		std::cout << row << " |";
		for (const auto &col : group)
		{
			if (row == col)
			{
				std::cout << ' ' << ((Web[row].size()) ? '=' : '?');
			}
			else
			{
				std::cout << ' ' << ((Web[row].end() == Web[row].find(col)) ? ' ' : '+');
			}
		}
		std::cout << " | " << row.substr(4) << std::endl;
	}
	std::cout << space << std::string(2*Web.size()+1, '=') << std::endl;
	for (int i=4; i<7; i++)
	{
		std::cout << space;
		for (const auto &ref : group)
		{
			std::cout << ' ' << ref.at(i);
		}
		std::cout << std::endl;
	}

	node.join();

	return EXIT_SUCCESS;
}
