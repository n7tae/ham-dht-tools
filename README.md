# dvin-tools

Tools for finding data on the Digital Voice Information Network (DVIN), using the OpenDHT, a Kademlia implementation of a distributed hash table network. These tools use the OpenDHT library to access information that exists on the DVIN. Systems on the DVIN publish important inforamation about their configuration and current environment.

## Tools

So far there are two tools, serveral more are planned.

### dht-get

*dht-get* is a command line tool that will print a section, or all sections, of a target's dht document. For a reflector there are four sections of its document:

1. The running configuration
2. The current list of peers
3. The current list of linked (*i.e.*, connected) clients
4. The *last heard* users list

Command lines options allow you specify a spection section, or, if you don't specify a specific section, all sections will be printed. Other options allow you to bootstrap into the DVIN at any node already connected to the DVIN. If you don't specify a bootstrap, *dht-get* will try to bootstrap from a default node. Other options allow you to control how times are displayed (local time or GMT) and whether you want a human readable output, or a JSON output.

To see how *dht-get* is used, type `./dht-get` and it will print a usage message.

### dht-spider

*dht-spider* is a command line tool that will *walk* the dht network when pointed to a specific module of a reflector. It will follow interlinked reflectors until all connected reflectors can be listed in a simple diagram. Here is a hypthetical result when probing Module A of a small interlinked system:

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

## Required packages

Several packages are needed;

```bash
sudo apt install git build-essential

### Distributed Hash Table (OpenDHT)

For these target systems using the DHT, connection information is published and updated directly by the target and is availabe to mvoice in near-realtime. All the mvoice user needs to know is the callsign of the target.

#### Building and installing the OpenDHT library

OpenDHT is available [here](https://github./com/savoirfairelinux/opendht.git). Building and installing instructions are in the [OpenDHT Wiki](https://github.com/savoirfairelinux/opendht/wiki/Build-the-library). Pascal support and proxy-server support (RESTinio) is not required for mvoice and so can be considered optional. With this in mind, this should work on Debian/Ubuntu-based systems:

```bash
# Install OpenDHT dependencies
sudo apt install libncurses5-dev libreadline-dev nettle-dev libgnutls28-dev libargon2-0-dev libmsgpack-dev  libssl-dev libfmt-dev libjsoncpp-dev libhttp-parser-dev libasio-dev cmake pkg-config

# clone the repo
git clone https://github.com/savoirfairelinux/opendht.git

# build and install
cd opendht
mkdir build && cd build
cmake -DOPENDHT_PYTHON=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
make
sudo make install
```

Please note that there is no easy way to uninstall OpenDHT once it's been installed. However, it is based on static header files and libraries and doesn't use any resouces except for a small amount of disk space.

## Building the DVIN tools

After the OpenDHT library has been installed, clone and make the tools:

```bash
git clone https://github.com/n7tae/dvin-tools.git
cd dvin-tools
make
```
