# Copyright (c) 2022 by Thomas A. Early N7TAE
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# locations for the executibles and other files are set here
# NOTE: IF YOU CHANGE THESE, YOU WILL NEED TO UPDATE THE service.* FILES AND
# if you change these locations, make sure the sgs.service file is updated!
# you will also break hard coded paths in the dashboard file, index.php.

# if you make changed in these two variable, you'll need to change things
# in the main.h file as well as the systemd service file.

debug = false

BINDIR = /usr/local/bin
CFGDIR = /usr/local/etc

CFLAGS = -W -std=c++17
LDFLAGS = -pthread -lopendht

ifeq ($(debug), true)
CFLAGS += -ggdb3
endif

dht-get : dht-get.cpp
	$(CXX) $(CFLAGS) -o $@ $^ -pthread -lopendht

clean :
	$(RM) *.o *.d dht-get

-include $(DEPS)

install :
	cp -f dvin-get $(BINDIR)

uninstall :
	rm -f $(CFGDIR)/dvin-get
