# ham-dht-tools

Tools for finding data on the ham radio network distributed hash table called *ham-dht*. The *ham-dht* uses OpenDHT, a Kademlia implementation of a DHT network. These tools use the OpenDHT library to access information that exists on this network. Systems using this network publish important information about their configuration and current state.

## The case for *ham-dht*

Digital voice radio nodes, commonly consisting of a low-powered transceiver, *i.e.*, hot-spot, and handheld radio can connect to other nodes from around the world using the internet to complete the connection. Up to now, software running on these hot-spot relied on host files that are tables of data that are supplied and updated by third parties. Currently, this is in the form of simple lookup tables that tie a target destination in the form of a unique callsign or designation to an IP address. They are simple, flat files, sometimes called host files. Sometimes these files might contain some other limited information, like port numbers or handshaking keywords. The *ham-dht* network is a way for nodes to get the connection information for any target *directly from the target* and so, eliminate the third party. Using a distributed hash table network provides several important advantages over a host table:

- ***The dht is guaranteed to be current and accurate.*** The information needed by the client is published directly by the target server. If the configuration of the server changes, that updated information is available immediately, not when some third gatekeeper decides to update a host file. Included with this feature, when a target goes silent, its published information will also become unavailable. The client need not to even attempt to connect if its publication(s) aren't available.
- ***Connection information isn't limited to just a few pieces of data.*** For example, a complex server node, like a transcoding reflector can publish which modules are available and which of those modules are transcoded. Realtime state information is also available, like the list of peers, clients and users. In fact, a published document is usually broken up into sections and published as need for each section. A client can just ask for a particular section of a published document, or the whole document. Further, a client can do a simple *get* where a request is usually completed in a second or two, or the client can do a *listen* and will receive a new document or section whenever it changes. Doing a *listen* is especially useful if a client is interested in state information.
- ***A distributed hash table network is a perfect match for ham users:***
	- Information is published by individual nodes and available to any other node on the network.
	- It works very similar to a mesh network. It doesn't rely on a single conduit for information to get from a server to a client. If a node becomes unavailable, the network continues to function. No node has all the data, and all the data is available to any node. Copies of publications are kept by by a few nodes "evenly" distributed throughout the network, making the availability of data fault tolerant, self-healing, and and protected from malicious attacks.
	- The API for OpenDHT is both flexible and easy to use. Published data is easily packaged in most standard C++ containers. Text, numeric or any binary data can be published and the actual documents are packed efficiently using a simple interface.

## The *ham-dht* implementation

### Examples and information

The definitive source of information about how to use OpenDHT is in the [OpenDHT Wiki](https://github.com/savoirfairelinux/opendht/wiki). The "API Overview" and "Running a node in your program" are great places to start with if you are new to OpenDHT.

An example for a client on the *ham-dht* network is here in this repo. Published data on the network, otherwise known as a `Value` is published using a 20-byte hash of a `key`. The `Value` of keys published by *mrefd* and *urfd* are described in the file `dht-values.h`, while two different application in the files `dht-get.cpp` and `dht-spider.cpp` get Values from either type of reflector and use it in various ways:
- dht-get gets all or a part of a reflector's document from a working reflector using its designator as a value key, or more properly, a 20-byte hash of the designator. For example, a key might be `M17-USA` or `URF307`.
- dht-spider uses reflector peer Values to evaluate the peer connection state of a particular module of a chosen reflector.
- The `dht-help` files are examples of useful subroutines for handling *mrefd* and *urfd* reflectors.

Examples of OpenDHT C++ code for servers on the *ham-dht* network are [*mrefd*][https://github.com/n7tae/mrefd], and [*urfd*](https://github.com/n7tae/urfd). All of the code having to do with the *ham-dht* network are in `mrefd/reflector.{h,cpp}` and `urfd/reflector/Reflector.{h,cpp}` files, respectively.

### Publication data, the heart of any dht-network

The center of the *ham-dht* network is the data, *i.e.*, the `Value`s, that servers publish. So far, two reflectors use *ham-dht*, *mrefd* and *urfd*, and in fact there are two slightly different versions of *urfd* Values that might be encountered. The four-part documents that each reflector publishes on the net are defined in the `dht-values.h` file in this repo. The four sections of a reflector *ham-dht* document are:
1. Configuration - This is all run-time configuration information that could be useful to a client, like the its IP address(es), the url of its dashboard, and its modules configuration.
2. Peers - A list of its connected peers. This list also contains the list of shared modules, and how long the peer has been connected.
3. Clients - A list of its connected clients and and its connection info.
4. Users - An ordered list of its most recent "last heard" users transmitting through the reflector.

The Configuration and Peers sections are published as *permanent* values. That is, the will reside in the DHT as long as the publisher is still connected to the DHT. If the publisher disconnect, usually by termination, these two published values will no longer be available to other nodes on the DHT network.

The Clients and Users sections are updated when their publishing node changes, but they are *not permanent* and so have a limited lifetime on the DHT network. Another tool is in the works that will *listen* for these sections from a particular publisher. Each time the publisher republished their Clients or Users sections, this listen tool will list the new values.

As seen in the `dht-values.h` file, each section's data is encapsulated in a `struct`, with a final `MSGPACK_DEFINE` statement telling OpenDHT the packing order of the data. The configuration `struct`s for the M17 and URF reflectors are quite different, with the M17 configuration mostly containing strings while the URF configuration contains several arrays. An OpenDHT document can contain most any standard C++ containers, including arrays, lists and maps. The elements of these containers should be basic C++ types, most any numeric types as well as strings. C++ tuples are also fully supported, allowing the the programmer to very easily encapsulate related data in a user-defined object. In fact, the three parts of a reflector's publication having to do with state information (1, 2 and 3) all use tuples.

Near the top of the `dht-values.h` file is a useful `const_expr` subroutine called `toUType` that take an element of a `enum class` and returns its underlying value. Using this allow elements of an array or tuple to be indexed based on an element of an `enum class`. While a resulting C++ expression might look complicated upon first examination, the beauty of using `toUType` is when programming in a context aware development environment like MS Code where *intellisense* is always proving the programmer with allowable choices when selecting a specific member of an array or tuple. It's a best practice to provide a `enum class` with every unique `std::array` or `std::tuple` used in a value part.

In addition to the user defined data that make up a published value, there are two other values if importance:
1. `dht::Value::user_type` is a `std::string` and by *ham-dht* convention is used to identify the version release of the value part.
2. `dht::Value::id` is a 64-bit unsigned and identifies the Value part. (This cannot be zero.) This id is important because in a call to `get()` or `listen()`, you can specify the part you want and then your call will only receive that part. This is specified in a dht::Where object that is defined and used in both *dht-get* and *dht-spider*.

Note that these two Value members are outside the packed portion of the Value and so are available before unpacking. These two values are used to ensure that the packed portion of the Value is unpacked into the correct `struct`.

### Publication lifetimes

Publications have a lifetime in OpenDHT. By default this lifetime is a few minutes. It is the publisher's responsibility to republish this data before the a publication's lifetime has expired. Or, a publisher can tag the publication as "permanent". The *permanent* tag means that the document will be available on the *ham-dht* as long as the publisher is connected to the net.

For the two reflectors that currently use *ham-dht* parts 1 and 2 are published using the *permanent* tag. While reflector administrators can change the peers while running, this is usually a rare event. Parts 3 and 4 are more transient in nature and so are only published by the reflector when that part of the reflector's state changes.

The transient *vs.* permanent state of the different parts of reflector's document are easily handled by a client interested in a reflector's document. Using the `get()` OpenDHT call is a one-shot retrieval of a document and this would be the appropriate way to retrieve either parts 1 or 2. Clients interested in the more transient parts 3 and 4 might wish to use OpenDHT's `listen()` and so retrieve those parts every time a reflector republishes them.

Finally, there is the possibility that a `get()` might receive more that one published Value, so each part also contains a std::time_t value so that the client can recognize the most recently published Value.

## Tools

So far there are three tools, several more are planned.

### dht-get

*dht-get* is a command line tool that will print a section, or all sections, of a target's dht document. For a reflector there are four sections of its document:

1. The running configuration
2. The current list of peers
3. The current list of linked (*i.e.*, connected) clients
4. The *last heard* users list

Command lines options allow you specify a specific section, or, if you don't specify a specific section, all sections will be printed. Other options allow you to bootstrap into the *ham-dht* at any node already connected to the *ham-dht*. If you don't specify a bootstrap, *dht-get* will try to bootstrap from a default node. Other options allow you to control how times are displayed (local time or GMT) and whether you want a human readable output, or a JSON output.

To see how *dht-get* is used, type `./dht-get` and it will print a usage message.

### dht-spider

*dht-spider* is a command line tool that will *walk* the dht network when pointed to a specific module of a reflector. It will follow interlinked reflectors until all connected reflectors can be listed in a simple diagram. Here is a hypothetical result when probing Module A of a small interlinked system:

```bash
someone@somesystem:~$ ./dht-spider m17-mmm a
         =======
M17-AAA | =   + | AAA
M17-MMM |   = + | MMM
M17-ZZZ | + + = | ZZZ
         =======
          A M Z
          A M Z
          A M Z
```
Here, three reflectors (M17-AAA, M17-MMM and M17-ZZZ) are sharing module A, but there is a problem: ZZZ is interlinked to both AAA and MMM, but AAA is not interlinked with MMM. This means that users keying up on AAA won't be heard by users on MMM and *vis versa*.

### get-config-params

*get-config-params* is a simple bash script that uses both *dht-spider* and *dht-get* to print most any configuration parameter for all the reflectors found with *dht-spider*.

## Required packages

Several packages are needed;

```bash
sudo apt install git build-essential

### Distributed Hash Table (OpenDHT)

For these target systems using the DHT, connection information is published and updated directly by the target and is available to mvoice in near-realtime. All the mvoice user needs to know is the callsign of the target.

#### Installing the OpenDHT development library

Debian 12 and Ubuntu 24.04 can install the library directly:

```bash
sudo apt install libopendht-dev
```

Earlier versions of this library are also available and you can try them. If they give no linking or execution error, then you are good to go. Otherwise you need to compile and install the library.

#### Building and installing the OpenDHT development library

OpenDHT is available [here](https://github./com/savoirfairelinux/opendht.git). Building and installing instructions are in the [OpenDHT Wiki](https://github.com/savoirfairelinux/opendht/wiki/Build-the-library). Pascal support is not required for these tools and so can be considered optional. With this in mind, this should work on Debian/Ubuntu-based systems:

First, install OpenDHT dependencies

```bash
sudo apt install libncurses5-dev libreadline-dev nettle-dev libgnutls28-dev libargon2-0-dev libmsgpack-dev  libssl-dev libfmt-dev libjsoncpp-dev libhttp-parser-dev libasio-dev cmake pkg-config
```

Then clone the *ham-dht-tools* repo

```baseh
git clone https://github.com/savoirfairelinux/opendht.git
```

Finally, build and install the library

```bash
cd opendht
mkdir build && cd build
cmake -DOPENDHT_PYTHON=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

Please note that there is no easy way to uninstall OpenDHT once it's been installed. However, it is based on static header files and libraries and doesn't use any resources except for a small amount of disk space.

## Building the tools

After the OpenDHT library has been installed, clone and make the tools:

```bash
git clone https://github.com/n7tae/ham-dht-tools.git
cd ham-dht-tools
make
```

## Installing the tools

You can install the tools if you like, do `make install` and each tool will be copied to `$(BINDIR)` defined in the Makefile. To install to a different location, you can copy `Makefile` to `makefile`, modify the definition of `BINDIR` and then do `make install`, but it may be easier to just copy the executables to your desired folder manually.

## Running a tool

All command line tools will print a usage message if you don't supply any arguments. You cannot run these tools on a machine that has an application that is already using UDP port 17171, like *mrefd*, *urfd* or *mvoice*.
