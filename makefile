# makefile
# Copyright 2024 Roman Tsykaliak
#   This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#   This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#   You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <https://www.gnu.org/licenses/>. 
LIBS:= event #ldns
INCLUDE:= #lib
override CFLAGS+=-g -DDEBUG -m64 -std=gnu11 -time -pipe \
-Werror -Wall -Wextra
LDLIBS+=$(LIBS:%=-l%)
LDFLAGS!=echo -n ${INCLUDE} | \
sed 's@\([[:alnum:]./_]\+\)@-L\1 -Wl,-rpath=\1@g';
TARGETS:=$(patsubst %.c,%,$(wildcard *.c))
.PHONY:all
ifneq "$(strip ${TARGETS})" ""
all:${TARGETS}
else
all:;@echo "Nothing to make!"
endif
.PHONY:clean
clean:
	-rm -f ${TARGETS}
