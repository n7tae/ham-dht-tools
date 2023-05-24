# Ham Radio Distributed Hash Table Network Specification

The ham radio distributed hash table network, *ham-dht*, uses an open-source implementation of dht, [OpenDHT](https://github.com/savoirfairelinux/opendht). The purpose of this network is to allow hams using a ham client application running on the internet easy and direct access to the current state of server nodes that are also running on the *ham-dht* network.

## A quick introduction

An OpenDHT network is a loose collection of nodes that share information with one-another. Any node on the network is connected only to other nodes that are "close", where "close" is an abstract definition that is an implementation detail of OpenDHT. Even though they may not be "close", any two nodes are not very far away from one another on an OpenDHT network. Any node can publish information on an OpenDHT network and that data will be sparsely duplicated on the network. Then any other node can retrieve that data quickly just by knowing the key to the published data. In OpenDHT, a key is a 120-bit hash value of up to a 20-character string. In this way, the published data is a file on the OpenDHT network, sparsely duplicated throughout the network, and the key to that file is a filename of up to 20 characters in length.

By far, the best documentation to use the *ham-dht* in a ham radio internet application is the [OpenDHT Wiki](https://github.com/savoirfairelinux/opendht/wiki). Any developers wishing to develop application(s) that use *ham-dht* should become very familiar with the OpenDHT Wiki. The Wiki describes how an application can connect to a dht, put information on the dht and get information from the dht. OpenDHT values are typically stored using a [MSGPACK library](https://msgpack.org/) that allows data of different types to be efficiently stored in a binary format.

## *ham-dht* state data

Digital Voice reflectors, specifically URF and M17 reflectors, are the first kind of node on the *ham-dht* to publish information on the the network. DV reflectors publish their current state. If the state changes, the data is republished.  Both DV reflectors publish four kinds of state data:

### 1. Configuration
This data is most useful to a connecting client node and contains not only its IP address(es) and port numbers for all supported protocols, but also available modules and the capability of those modules. For example, URF reflectors publish which modules are supported by transcoding hardware, and M17 reflectors publish which modules will retransmit encrypted voice-streams.
### 2. Connected Peers
DV reflectors can interlink with other reflectors, creating a network of interlinked systems. This information can be used to analyze these connections. For example, This data can be used by simple client nodes to discover new reflectors that might have been previously unknown to the client. A special spider node has already been created to examine the interconnection state of a shared channel, allowing administrators to maintain their network topology.
### 3. Connected Clients
The callsigns of the currently connected clients show to which module they are connected. These connected nodes will usually be repeaters or hot-spots, but could also be protocol-specific connections to other networks.
### 4. Users
 This data is essentially a *last-heard* list and is useful to see who's been talking recently on each reflector module.

In addition to the user-defined data, OpenDHT values also contain two other useful items: A string, `user_type` and a long unsigned integer, `id`. Both of these values are important for *ham-dht*. The `id` value can be thought of as a value sub-type where a complete value published on a DHT can have different parts, each with its own `id`. Breaking up a server value by `id` improved efficiency for both the server and client. The `user_type` string is used by *ham-dht* to designate a release version. This allows each value to evolve as needed. While servers will only publish one version of its state data, clients will be able to distinguish and deal with different versions of values from different servers.

State data declarations for all known *ham-dht* nodes are listed below:

### M17 reflectors

```c++
struct SMrefdConfig1
{
	std::time_t timestamp;
	std::string callsign, ipv4addr, ipv6addr, modules, encryptedmods, url, email, sponsor, country, version;
	uint16_t port;

	MSGPACK_DEFINE(timestamp, callsign, ipv4addr, ipv6addr, modules, encryptedmods, url, email, sponsor, country, version, port)
};
```

State data can be broken up into pieces. OpenDHT supports this through an unsigned 64-bit *id* that is assigned by the publisher. The *id* can be any non-zero unsigned integer. An *enum class* is an ideal way for clients and servers to coordinate the usage of an *id* value:

```c++
// for M17 reflectors
enum class EMrefdValueID : uint64_t { Config=1, Peers=2, Clients=3, Users=4 };
// for URF reflectors
enum class EUrfdValueID : uint64_t { Config=1, Peers=2, Clients=3, Users=4 };
```

Using an *enum class* is a self-documented way for both client and server to be sure they are referring to the same thing, but help is needed because

```c++
const unsigned config_id = EUrfdValueID::Config;
```

won't work without a cast. Rigorous type-checking will flag this as an error. C++ version 14 provides an elegant solution:

```c++
template<typename E> constexpr auto toUType(E enumerator) noexcept
{
	return static_cast<std::underlying_type_t<E>>(enumerator);
} // Item #10 in "Effective Modern C++", by Scott Meyers, O'REILLY
```

That makes this possible:

```c++
const unsigned config_id = toUType(EUrfdValueID::Config);
```

Note that *toUType* is a *constexpr* and so is known at compile time.

Breaking a reflector's state data up into multiple pieces improves efficiency of both the publication and the retrieval of the state data. Configuration data only needs to be published once while Connected Peers can be republished whenever the interconnection(s) change. Connected Clients and Last Heard will constantly change on a busy reflector. Clients can retrieve all four parts of a reflector's published state data, or they can retrieve only one part. Simple connecting clients will generally only be interested in the configuration data.

## Some history

Historically, connection information has been implemented in a variety of ways, most commonly, using a file-based lookup table (dictionary) that the application can use to connect to a target node on the internet. In order to keep up with the ever-changing participants, the lookup file needs to be updated on a regular basis as new server nodes become available, or are decommissioned and are no longer available. Client nodes all have some capability to update the lookup table at any time, but once a table is generated, in just a few minutes, the information is potentially inaccurate. Also, in nearly all cases the file-based lookup table contains a bare minimum of information, usually just an IP address and maybe a port number. D-Star provides a highly specialized, pseudo real-time "routing network" that uses a central server that monitors all clients and provides routing information to interested client wishing to connect to a particular target. In either method, the connecting client just needs to know the callsign of the target in order to connect to it. There are also several closed-source DMR networks that, once connected using known configuration information, allow you to essentially route to a destination just knowing a talk-group number. Yaesu has also developed a closed-source network accessible from their System Fusion radios.

## Some *ham-dht* basics

First and foremost, *ham-dht* is a file sharing network. New nodes join the network by first contacting any node that is already on the network. Any node can can "publish" information on the network, but in general, only nodes that act in some capacity as a server will publish information on the *ham-dht*. That is to say, server nodes will have open ports accepting connections from other nodes. Digital voice reflectors are excellent examples of server nodes that can publish state information on the *ham-dht*.

Here is an example of how an M17 reflector, M17-EFG, would join *ham-dht*:

```c++
#include <opendht.h>

const std::string callsign("M17-EFG");
dht::DhtRunner node;

node.run(17171, dht::crypto::generateIdentity(callsign));
node.bootstrap("m17-usa.openquad.net", "17171");
```

First, a node of type *dht::DhtRunner* is defined. *DhtRunner* is a thread-safe interface to OpenDHT, providing both IPv4 and IPv6 access to the file sharing network. Then, the node is run passing the UDP port number (*ham-dht* uses port 17171) and a cryptographically generated identity based on the reflector callsign. This id allows this server node to published signed documents. Finally, the running node joins the *ham-dht* by boot-strapping from any existing node on the network.
