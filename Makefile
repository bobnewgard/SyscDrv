#
# Copyright 2013-2021 Robert Newgard
#
# This file is part of SyscDrv.
#
# SyscDrv is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# SyscDrv is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SyscDrv.  If not, see <http://www.gnu.org/licenses/>.
#

#
# define library name
# ------------------------------------------------------------------------------
LNAM := syscdrv
#
# clear rules
# ------------------------------------------------------------------------------
MAKEFLAGS += --no-builtin-rules
.SUFFIXES:
#
# includes
# ------------------------------------------------------------------------------
include lib-dep.mk
include test-dep.mk
#
# defines
# ------------------------------------------------------------------------------
define print-hint-header
	@col=$$(tput cols) ; while [ "$$col" -gt 0 ] ; do echo -n '-' ; col=$$(($$col - 1)) ; done
	@echo
endef
#
define print-hints
	@echo "lib        build .so and .a"
	@echo "test       build test"
	@echo "run        run test"
	@echo "clean      remove build outputs"
	@echo "spotless   remove build intermediates and library"
endef
#
define compile-for-obj
	g++ $(CXX_VER) -c $(DFLAGS) $(IPATH) -Wall $(CFLAGS) -o $@ $<
endef
#
define ar-for-archive
        ar cr $@ $%
endef
#
define link-for-shared-obj
	g++ $(CFLAGS) -shared -Wl,--whole-archive $^ -Wl,--no-whole-archive -o $@
endef
#
define link-for-executable
	g++ $(CFLAGS) -o $@ $(LPATH) $(LFLAGS) -l$(LNAM) $<
endef
#
define cpp-for-depends
	@echo "[INF] building $@"
	rm -f $@
        for file in $^ ; do cpp $(CXX_VER) -MM -MG $(IPATH) $$file ; done >> $@
        for file in $^ ; do cpp $(CXX_VER) -MM -MG -MT $@ $(IPATH) $$file >> $@ ; done
endef
#
define phonys
    test run clean hint hint-header
endef
#
define lib-sources
	SyscDrv.cxx
endef
#
define test-sources
	test1.cxx
endef
#
define dox
        rm -f Readme.md
        echo "\\mainpage %SyscDrv Project" >> Readme.md
        cat README.md >> Readme.md
        doxygen dox.conf
endef
#
#
# variables
# ------------------------------------------------------------------------------
SHELL     := /bin/bash
SYSC_DIR  := /opt/systemc/2.3.0
#
IPATH     := -I . -I ../SyscMsg -I ../SyscJson -isystem $(SYSC_DIR)/include
LPATH     := -L . -L ../SyscMsg -L ../SyscJson -L $(SYSC_DIR)/lib-linux64
#
CXX_VER   := -x c++ -std=c++11
#
DFLAGS    :=
CFLAGS    := -m64 -g -pthread -fPIC
LFLAGS    := -lsystemc -pthread -lsyscmsg -lsyscjson
#
# lib
LIB_CXX   := $(strip $(lib-sources))
LIB_OBJ   := $(patsubst %.cxx,%.o,$(LIB_CXX))
LIB_AR    := lib$(LNAM).a($(LIB_OBJ))
LIB_ARNAM := lib$(LNAM).a
LIB_SONAM := lib$(LNAM).so
#
# test
TEST_CXX  := $(strip $(test-sources))
TEST_OBJ  := $(patsubst %.cxx,%.o,$(TEST_CXX))
TEST_EXE  := $(patsubst %.cxx,%-exe,$(TEST_CXX))
TEST_PREQ := $(TEST_OBJ) $(LIB_SONAM)
#
# intermediate object files: position dependent
# ------------------------------------------------------------------------------
.INTERMEDIATE : $(LIB_OBJ)
#
# common implicit rules
# ------------------------------------------------------------------------------
(%) : % ; $(ar-for-archive)
#
# lib rules
# ------------------------------------------------------------------------------
%.o            : %.cxx         ; $(compile-for-obj)
$(LIB_ARNAM)   : $(LIB_AR)     ; 
$(LIB_SONAM)   : $(LIB_ARNAM)  ; $(link-for-shared-obj)
lib-dep.mk     : $(LIB_CXX)    ; $(cpp-for-depends)
#
# test rules
# ------------------------------------------------------------------------------
test-dep.mk    : $(TEST_CXX)   ; $(cpp-for-depends)
test1-exe      : $(TEST_PREQ)  ; $(link-for-executable)
#
# Other rules
# ------------------------------------------------------------------------------
hints          : hint-header   ; $(print-hints)
hint-header    :               ; $(print-hint-header)
lib            : $(LIB_SONAM)  ;
test           : $(TEST_EXE)   ; 
run            : $(TEST_EXE)   ; export LD_LIBRARY_PATH=.:../SyscMsg:../SyscJson:$(SYSC_DIR)/lib-linux64 ; ./test1-exe 
dox            :               ; $(dox)
clean          :               ; rm -f *-exe *.o *.pyc
spotless       : clean         ; rm -rf *-dep.mk *.so *.a doxygen Readme.md
srcs           :               ; @echo \"$(LIB_CXX)\"
objs           :               ; @echo \"$(LIB_OBJ)\"
test-objs      :               ; @echo \"$(TEST_OBJ)\"
test-exes      :               ; @echo \"$(TEST_EXE)\"
.PHONY         : $(phonys)     ; 

.DEFAULT_GOAL := hints
